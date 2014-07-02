#pragma once

#include <cassert>
#include <vector>

#include "Nodes.h"

using std::vector;

template<typename DataType>
class BinaryDag {
public:
	BinaryDag(const int n = 0): nodes() {
		nodes.reserve(n);
		// add a dummy element that is guaranteed to not appear
		// (we assume that -1 is used for leaves, never -2, except for this dummy)
		nodes.emplace_back(-2, -2, (std::string*)NULL);
	}

	int addNode(int left, int right, DataType *label) {
		nodes.emplace_back(left, right, label);
		// TODO figure out if this is correct
		if (left != -1) {
			nodes[left].inDegree++;
		}
		if (right != -1) {
			nodes[right].inDegree++;
		}
		return (nodes.size() - 1);
	}

	void popNode() {
		nodes.pop_back();
	}

	int addNodes(const int n) {
		nodes.resize(nodes.size() + n);
		return nodes.size() - n;
	}

	friend std::ostream& operator<<(std::ostream& os, const BinaryDag<DataType> &dag) {
		os << "Binary Dag with " << dag.nodes.size() << " nodes";
		for (uint i = 0; i < dag.nodes.size(); ++i) {
			os << "; " << i << "=" << dag.nodes[i];
		}
		return os;
	}

	vector<DagNode<DataType>> nodes;
};