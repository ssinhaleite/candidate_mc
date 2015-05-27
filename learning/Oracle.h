#ifndef CANDIDATE_MC_LEARNING_ORACLE_H__
#define CANDIDATE_MC_LEARNING_ORACLE_H__

#include <crag/Crag.h>
#include <inference/MultiCut.h>
#include <features/NodeFeatures.h>
#include <features/EdgeFeatures.h>
#include "Loss.h"
#include "BestEffort.h"

/**
 * Provides solution for loss-augmented inference problem, given a set of 
 * weights. To be used in a learning optimizer.
 */
class Oracle {

public:

	Oracle(
			const Crag&                  crag,
			const NodeFeatures&          nodeFeatures,
			const EdgeFeatures&          edgeFeatures,
			const Loss&                  loss,
			const BestEffort&            bestEffort,
			MultiCut::Parameters         parameters = MultiCut::Parameters()) :
		_crag(crag),
		_nodeFeatures(nodeFeatures),
		_edgeFeatures(edgeFeatures),
		_loss(loss),
		_bestEffort(bestEffort),
		_nodeCosts(_crag),
		_edgeCosts(_crag),
		_multicut(crag, parameters) {}

	void operator()(
			const std::vector<double>& weights,
			double&                    value,
			std::vector<double>&       gradient);

private:

	void updateCosts(const std::vector<double>& weights);

	void accumulateGradient(std::vector<double>& gradient);

	inline double nodeCost(const std::vector<double>& weights, const std::vector<double>& nodeFeatures) const {

		return dot(weights.begin(), weights.begin() + _nodeFeatures.dims(), nodeFeatures.begin());
	}

	inline double edgeCost(const std::vector<double>& weights, const std::vector<double>& edgeFeatures) const {

		return dot(weights.begin() + _nodeFeatures.dims(), weights.end(), edgeFeatures.begin());
	}

	template <typename IT>
	inline double dot(IT ba, IT ea, IT bb) const {

		double sum = 0;
		while (ba != ea) {

			sum += (*ba)*(*bb);
			ba++;
			bb++;
		}

		return sum;
	}

	const Crag&         _crag;
	const NodeFeatures& _nodeFeatures;
	const EdgeFeatures& _edgeFeatures;
	const Loss&         _loss;
	const BestEffort&   _bestEffort;

	Crag::NodeMap<double> _nodeCosts;
	Crag::EdgeMap<double> _edgeCosts;

	// constant to be added to the optimal value of the multi-cut solution
	double _constant;

	MultiCut _multicut;
};

#endif // CANDIDATE_MC_LEARNING_ORACLE_H__
