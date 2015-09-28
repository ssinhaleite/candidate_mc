#ifndef CANDIDATE_MC_FEATURES_NODE_FEATURES_H__
#define CANDIDATE_MC_FEATURES_NODE_FEATURES_H__

#include "Features.h"
#include "FeatureWeights.h"

class NodeFeatures {

	typedef Features<Crag::CragNode> FeaturesType;

public:

	NodeFeatures(const Crag& crag) :
			_crag(crag),
			_vnFeatures(crag),
			_slFeatures(crag),
			_asFeatures(crag) {}

	inline void append(Crag::CragNode n, double feature) {

		features(_crag.type(n)).append(n, feature);
	}

	const std::vector<double>& operator[](Crag::CragNode n) const {

		return features(_crag.type(n))[n];
	}

	void set(Crag::CragNode n, const std::vector<double>& v) {

		features(_crag.type(n)).set(n, v);
	}

	inline unsigned int dims(Crag::NodeType type) const {

		return features(type).dims();
	}

	void normalize() {

		features(Crag::VolumeNode).normalize();
		features(Crag::SliceNode).normalize();
		features(Crag::AssignmentNode).normalize();
	}

	void normalize(
			const FeatureWeights& min,
			const FeatureWeights& max) {

		features(Crag::VolumeNode).normalize(min[Crag::VolumeNode], max[Crag::VolumeNode]);
		features(Crag::SliceNode).normalize(min[Crag::SliceNode], max[Crag::SliceNode]);
		features(Crag::AssignmentNode).normalize(min[Crag::AssignmentNode], max[Crag::AssignmentNode]);
	}

	void getMin(FeatureWeights& min) {

		min[Crag::VolumeNode] = features(Crag::VolumeNode).getMin();
		min[Crag::SliceNode] = features(Crag::SliceNode).getMin();
		min[Crag::AssignmentNode] = features(Crag::AssignmentNode).getMin();
	}

	void getMax(FeatureWeights& max) {

		max[Crag::VolumeNode] = features(Crag::VolumeNode).getMax();
		max[Crag::SliceNode] = features(Crag::SliceNode).getMax();
		max[Crag::AssignmentNode] = features(Crag::AssignmentNode).getMax();
	}

private:

	FeaturesType& features(Crag::NodeType type) {

		switch (type) {

			case Crag::VolumeNode:
				return _vnFeatures;
			case Crag::SliceNode:
				return _slFeatures;
			case Crag::AssignmentNode:
				return _asFeatures;
			case Crag::NoAssignmentNode:
				UTIL_THROW_EXCEPTION(
						UsageError,
						"nodes of type NoAssignmentNode don't have features");
		}

		UTIL_THROW_EXCEPTION(
				UsageError,
				"unknown node type " << type);
	}

	const FeaturesType& features(Crag::NodeType type) const {

		switch (type) {

			case Crag::VolumeNode:
				return _vnFeatures;
			case Crag::SliceNode:
				return _slFeatures;
			case Crag::AssignmentNode:
				return _asFeatures;
			case Crag::NoAssignmentNode:
				UTIL_THROW_EXCEPTION(
						UsageError,
						"nodes of type NoAssignmentNode don't have features");
		}

		UTIL_THROW_EXCEPTION(
				UsageError,
				"unknown node type " << type);
	}

	const Crag& _crag;

	// one set of features for each node type
	FeaturesType _vnFeatures;
	FeaturesType _slFeatures;
	FeaturesType _asFeatures;
};

#endif // CANDIDATE_MC_FEATURES_NODE_FEATURES_H__

