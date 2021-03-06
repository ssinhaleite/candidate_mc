#ifndef CANDIDATE_MC_INFERENCE_RANDOM_FOREST_H__
#define CANDIDATE_MC_INFERENCE_RANDOM_FOREST_H__

#include <vector>

#include <vigra/multi_array.hxx>
#include <vigra/random_forest.hxx>

class RandomForest {

public:

	typedef unsigned int LabelType;
	typedef double       FeatureType;

	typedef vigra::MultiArray<2, FeatureType> SamplesType;
	typedef vigra::MultiArray<2, LabelType>   LabelsType;
	typedef vigra::MultiArray<2, double>      ProbsType;

	typedef SamplesType::difference_type SamplesSize;
	typedef LabelsType::difference_type  LabelsSize;
	typedef ProbsType::difference_type   ProbsSize;

	typedef vigra::RandomForest<LabelType> RandomForestType;

	RandomForest();

	/**
	 * Allocate memory to hold the given number of samples.
	 */
	void prepareTraining(int numSamples, int numFeatures);

	/**
	 * Add a training sample.
	 */
	void addSample(const std::vector<FeatureType>& sample, LabelType label);

	/**
	 * Train the classifier with the given number of trees under consideration
	 * of numFeatures features.
	 */
	void train(int numTrees = 0, int numFeatures = 0, bool computeVariableImportance = false);

	/**
	 * Returns the out-of-bag error after training.
	 */
	double getOutOfBagError();

	/**
	 * Get the variable importance for each feature after training.
	 */
	std::vector<double> getVariableImportance();

	/**
	 * Get the predicted label for a single sample.
	 */
	int getLabel(const std::vector<FeatureType>& sample);

	/**
	 * Get the class probability distribution for a single sample. The number of
	 * classes depends on the labels of the training data.
	 */
	std::vector<double> getProbabilities(const std::vector<FeatureType>& sample);

	/**
	 * Write the classifier to a file.
	 */
	void write(std::string filename, std::string group);

	/**
	 * Read the classifier from a file.
	 */
	void read(std::string filename, std::string group);

private:

	// random forest implementation

	RandomForestType _rf;

	// training data

	SamplesType _samples;
	LabelsType  _labels;

	unsigned int _numSamples;
	unsigned int _numFeatures;
	unsigned int _numClasses;

	unsigned int _nextSample;

	// training statistics

	double              _outOfBagError;
	std::vector<double> _variableImportance;

};

#endif // CANDIDATE_MC_INFERENCE_RANDOM_FOREST_H__

