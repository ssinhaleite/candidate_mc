/**
 * This programs visualizes a CRAG stored in an hdf5 file.
 */

#include <io/Hdf5CragStore.h>
#include <io/Hdf5VolumeStore.h>
#include <util/ProgramOptions.h>
#include <imageprocessing/ExplicitVolume.h>
#include <gui/CragView.h>
#include <gui/MeshViewController.h>
#include <gui/CostsView.h>
#include <gui/FeaturesView.h>
#include <sg_gui/RotateView.h>
#include <sg_gui/ZoomView.h>
#include <sg_gui/Window.h>

using namespace sg_gui;

util::ProgramOption optionProjectFile(
		util::_long_name        = "projectFile",
		util::_short_name       = "p",
		util::_description_text = "The project file to read the CRAG from.",
		util::_default_value    = "project.hdf");

util::ProgramOption optionShowLabels(
		util::_long_name        = "showLabels",
		util::_description_text = "Instead of showing the CRAG leaf nodes, show the labels (i.e., a segmentation) stored in the project file.");

util::ProgramOption optionShowGroundTruth(
		util::_long_name        = "showGroundTruth",
		util::_description_text = "Instead of showing the CRAG leaf nodes, show the ground truth labels.");

int main(int argc, char** argv) {

	try {

		util::ProgramOptions::init(argc, argv);
		logger::LogManager::init();

		// create hdf5 stores

		Hdf5CragStore   cragStore(optionProjectFile.as<std::string>());
		Hdf5VolumeStore volumeStore(optionProjectFile.as<std::string>());

		// get crag and volumes

		Crag        crag;
		CragVolumes volumes(crag);

		try {

			cragStore.retrieveCrag(crag);
			cragStore.retrieveVolumes(volumes);

		} catch (std::exception& e) {

			LOG_USER(logger::out) << "could not find a CRAG" << std::endl;
		}

		// get intensities and supervoxel labels

		auto intensities = std::make_shared<ExplicitVolume<float>>();
		volumeStore.retrieveIntensities(*intensities);
		intensities->normalize();
		float min, max;
		intensities->data().minmax(&min, &max);

		auto supervoxels =
				std::make_shared<ExplicitVolume<float>>(
						intensities->width(),
						intensities->height(),
						intensities->depth());
		supervoxels->setResolution(intensities->getResolution());
		supervoxels->setOffset(intensities->getOffset());
		supervoxels->data() = 0;

		for (Crag::NodeIt n(crag); n != lemon::INVALID; ++n) {

			if (!crag.isLeafNode(n))
				continue;

			const CragVolume& volume = *volumes[n];
			util::point<int, 3> offset = volume.getOffset()/volume.getResolution();

			for (unsigned int z = 0; z < volume.getDiscreteBoundingBox().depth();  z++)
			for (unsigned int y = 0; y < volume.getDiscreteBoundingBox().height(); y++)
			for (unsigned int x = 0; x < volume.getDiscreteBoundingBox().width();  x++) {

				if (volume.data()(x, y, z))
					(*supervoxels)(
							offset.x() + x,
							offset.y() + y,
							offset.z() + z)
									= crag.id(n);
			}
		}

		auto labels = supervoxels;

		if (optionShowLabels) {

			labels = std::make_shared<ExplicitVolume<float>>();

			LOG_USER(logger::out) << "showing solution labels" << std::endl;

			ExplicitVolume<int> tmp;
			volumeStore.retrieveLabels(tmp);
			*labels = tmp;

		} else if (optionShowGroundTruth) {

			labels = std::make_shared<ExplicitVolume<float>>();

			LOG_USER(logger::out) << "showing ground truth labels" << std::endl;

			ExplicitVolume<int> tmp;
			volumeStore.retrieveGroundTruth(tmp);
			*labels = tmp;

		} else {

			LOG_USER(logger::out) << "showing leaf node labels" << std::endl;
		}

		// get features

		NodeFeatures nodeFeatures(crag);
		EdgeFeatures edgeFeatures(crag);

		try {

			cragStore.retrieveNodeFeatures(crag, nodeFeatures);
			cragStore.retrieveEdgeFeatures(crag, edgeFeatures);

		} catch (std::exception& e) {

			LOG_USER(logger::out) << "could not find features" << std::endl;
		}

		// get costs

		Costs bestEffortLoss(crag);

		try {

			cragStore.retrieveCosts(crag, bestEffortLoss, "best-effort_loss");

		} catch (std::exception& e) {

			LOG_USER(logger::out) << "could not find costs" << std::endl;
		}

		// visualize

		auto cragView       = std::make_shared<CragView>();
		auto meshController = std::make_shared<MeshViewController>(crag, volumes, supervoxels);
		auto costsView      = std::make_shared<CostsView>(crag, bestEffortLoss, "best-effort loss");
		auto featuresView   = std::make_shared<FeaturesView>(crag, nodeFeatures, edgeFeatures);
		auto rotateView     = std::make_shared<RotateView>();
		auto zoomView       = std::make_shared<ZoomView>(true);
		auto window         = std::make_shared<sg_gui::Window>("CRAG viewer");

		window->add(zoomView);
		zoomView->add(rotateView);
		rotateView->add(cragView);
		rotateView->add(meshController);
		rotateView->add(costsView);
		rotateView->add(featuresView);

		cragView->setRawVolume(intensities);
		cragView->setLabelsVolume(labels);

		window->processEvents();

	} catch (boost::exception& e) {

		handleException(e, std::cerr);
	}
}

