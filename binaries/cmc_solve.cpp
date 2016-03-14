/**
 * Reads a treemc project file containing features and solves the segmentation 
 * problem for a given set of feature weights.
 */

#include <iostream>
#include <fstream>
#include <boost/filesystem.hpp>

#include <util/Logger.h>
#include <util/ProgramOptions.h>
#include <util/exceptions.h>
#include <util/helpers.hpp>
#include <util/timing.h>
#include <util/assert.h>
#include <io/Hdf5CragStore.h>
#include <io/Hdf5VolumeStore.h>
#include <io/SolutionImageWriter.h>
#include <features/FeatureExtractor.h>
#include <inference/CragSolverFactory.h>

util::ProgramOption optionForegroundBias(
		util::_long_name        = "foregroundBias",
		util::_short_name       = "f",
		util::_description_text = "A bias to be added to each node weight.",
		util::_default_value    = 0);

util::ProgramOption optionMergeBias(
		util::_long_name        = "mergeBias",
		util::_short_name       = "b",
		util::_description_text = "A bias to be added to each edge weight.",
		util::_default_value    = 0);

util::ProgramOption optionDismissPositiveCosts(
		util::_long_name        = "dismissPositiveCosts",
		util::_description_text = "If set, the costs of non-leaf nodes and non-leaf edges that are positive "
								  "will be set to inf.");

util::ProgramOption optionProjectFile(
		util::_long_name        = "projectFile",
		util::_short_name       = "p",
		util::_description_text = "The candidate mc project file.");

util::ProgramOption optionExportSolution(
		util::_long_name        = "exportSolution",
		util::_description_text = "Create a volume export for the solution.");

util::ProgramOption optionNumIterations(
		util::_long_name        = "numIterations",
		util::_description_text = "The number of iterations to spend on finding a solution. Depends on used solver.");

util::ProgramOption optionExportSolutionWithBoundary(
		util::_long_name        = "exportSolutionWithBoundary",
		util::_description_text = "Create a volume export for the solution, showing the boundaries as well..");

util::ProgramOption optionReadOnly(
		util::_long_name        = "readOnly",
		util::_description_text = "Don't write the solution or costs to the project file (only export the solution).");

inline double dot(const std::vector<double>& a, const std::vector<double>& b) {

	UTIL_ASSERT_REL(a.size(), ==, b.size());

	double sum = 0;
	auto ba = a.begin();
	auto ea = a.end();
	auto bb = b.begin();

	while (ba != ea) {

		sum += (*ba)*(*bb);
		ba++;
		bb++;
	}

	return sum;
}

int main(int argc, char** argv) {

	UTIL_TIME_SCOPE("main");

	try {

		util::ProgramOptions::init(argc, argv);
		logger::LogManager::init();

		Crag        crag;
		CragVolumes volumes(crag);

		NodeFeatures nodeFeatures(crag);
		EdgeFeatures edgeFeatures(crag);

		LOG_USER(logger::out) << "reading CRAG and volumes" << std::endl;

		Hdf5CragStore cragStore(optionProjectFile.as<std::string>());
		cragStore.retrieveCrag(crag);
		cragStore.retrieveVolumes(volumes);

		LOG_USER(logger::out) << "reading features" << std::endl;

		cragStore.retrieveNodeFeatures(crag, nodeFeatures);
		cragStore.retrieveEdgeFeatures(crag, edgeFeatures);

		LOG_USER(logger::out) << "computing costs" << std::endl;

		FeatureWeights weights;
		cragStore.retrieveFeatureWeights(weights);

		Costs costs(crag);

		float edgeBias = optionMergeBias;
		float nodeBias = optionForegroundBias;

		bool dismissPositiveCosts = optionDismissPositiveCosts;

		for (Crag::CragNode n : crag.nodes()) {

			costs.node[n] = nodeBias;
			costs.node[n] += dot(weights[crag.type(n)], nodeFeatures[n]);

			if (dismissPositiveCosts && !crag.isLeafNode(n) && costs.node[n] > 0)
				costs.node[n] = std::numeric_limits<double>::infinity();
		}

		for (Crag::CragEdge e : crag.edges()) {

			costs.edge[e] = edgeBias;
			costs.edge[e] += dot(weights[crag.type(e)], edgeFeatures[e]);

			if (dismissPositiveCosts && !crag.isLeafEdge(e) && costs.edge[e] > 0)
				costs.edge[e] = std::numeric_limits<double>::infinity();
		}

		if (!optionReadOnly)
			cragStore.saveCosts(crag, costs, "costs");

		LOG_USER(logger::out) << "solving" << std::endl;

		CragSolution solution(crag);
		CragSolver::Parameters parameters;
		if (optionNumIterations)
			parameters.numIterations = optionNumIterations;
		std::unique_ptr<CragSolver> solver(CragSolverFactory::createSolver(crag, volumes, parameters));

		solver->setCosts(costs);
		{
			UTIL_TIME_SCOPE("solve candidate multi-cut");
			solver->solve(solution);
		}

		LOG_USER(logger::out) << "problem solved" << std::endl;

		LOG_USER(logger::out) << "storing solution" << std::endl;

		if (!optionReadOnly)
			cragStore.saveSolution(crag, solution, "solution");

		if (optionExportSolution) {

			LOG_USER(logger::out) << "exporting solution to " << optionExportSolution.as<std::string>() << std::endl;

			Hdf5VolumeStore volumeStore(optionProjectFile.as<std::string>());
			ExplicitVolume<float> intensities;
			volumeStore.retrieveIntensities(intensities);

			SolutionImageWriter imageWriter;
			imageWriter.setExportArea(intensities.getBoundingBox());
			imageWriter.write(crag, volumes, solution, optionExportSolution.as<std::string>());
		}

		if (optionExportSolutionWithBoundary) {

			LOG_USER(logger::out) << "exporting solution with boundaries to " << optionExportSolutionWithBoundary.as<std::string>() << std::endl;

			Hdf5VolumeStore volumeStore(optionProjectFile.as<std::string>());
			ExplicitVolume<float> intensities;
			volumeStore.retrieveIntensities(intensities);

			SolutionImageWriter imageWriter;
			imageWriter.setExportArea(intensities.getBoundingBox());
			imageWriter.write(crag, volumes, solution, optionExportSolution.as<std::string>() + "_boundary", true);
		}

	} catch (Exception& e) {

		handleException(e, std::cerr);
	}
}

