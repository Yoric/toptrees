#pragma once

#include "Labels.h"

template <typename TreeType, typename DataType>
struct NodeHasher {
	NodeHasher(TreeType &tree, const LabelsT<DataType> &labels) : tree(tree), labels(labels) {}

	void hash() const {
		hashNode(0, true);
	}

	void hashNode(int nodeId, const bool force=false) const {
		uint seed(0);
		boost_hash_combine(seed, labels[nodeId]);

		for (auto *edge = tree.firstEdge(nodeId); edge <= tree.lastEdge(nodeId); ++edge) {
			if (edge->valid) {
				if (force || tree.nodes[edge->headNode].hash == 0) {
					hashNode(edge->headNode, force);
				}
				boost_hash_combine(seed, tree.nodes[edge->headNode].hash);
			}
		}

		tree.nodes[nodeId].hash = seed;
	}

	TreeType &tree;
	const LabelsT<DataType> &labels;
};
