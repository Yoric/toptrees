#include <iostream>
#include <vector>

#include "Edges.h"
#include "Nodes.h"
#include "OrderedTree.h"

#include "TopTree.h"
#include "XML.h"
#include "Timer.h"

#include "DagBuilder.h"

using std::cout;
using std::endl;
using std::string;
using std::vector;

int main(int argc, char** argv) {
	OrderedTree<TreeNode,TreeEdge> t;

	vector<string> labels;

	string filename = argc > 1 ? string(argv[1]) : "data/1998statistics.xml";
	string outputfolder = argc > 2 ? string(argv[2]) : "/tmp";

	// Read input file
	XmlParser<OrderedTree<TreeNode,TreeEdge>> xml(filename, t, labels);
	xml.parse();

	// Dump input file for comparison of output
	Timer timer;
	XmlWriter<OrderedTree<TreeNode,TreeEdge>> origWriter(t, labels);
	origWriter.write(outputfolder + "/orig.xml");

	cout << "Wrote orginial trimmed XML file in " << timer.getAndReset() << "ms: " << t.summary() << endl;

	// Prepare for construction of top tree
	TopTree topTree(t._numNodes, labels);
	vector<int> nodeIds(t._numNodes);
	for (int i = 0; i < t._numNodes; ++i) {
		nodeIds[i] = i;
	}

	// construct top tree
	timer.reset();
	t.doMerges([&] (const int u, const int v, const int n, const MergeType type) {
		nodeIds[n] = topTree.addCluster(nodeIds[u], nodeIds[v], type);
	});

	cout << "Top tree construction took " << timer.getAndReset() << "ms; Top tree has " << topTree.clusters.size() << " clusters (" << topTree.clusters.size() - t._numNodes << " non-leaves)" << endl;

	// Construct top DAG
	BinaryDag<string> dag;
	DagBuilder<string> builder(topTree, dag);
	builder.createDag();

	cout << "Top DAG has " << dag.nodes.size() - 1<< " nodes, " << dag.countEdges() << " edges" << endl;
	cout << "Top DAG construction took in " << timer.getAndReset() << "ms" << endl;

	// Unpack top DAG to recoveredTopTree
	TopTree recoveredTopTree(t._numNodes);
	BinaryDagUnpacker<string> dagUnpacker(dag, recoveredTopTree);
	dagUnpacker.unpack();

	cout << "Unpacked Top DAG in " << timer.getAndReset() << "ms, has " << recoveredTopTree.clusters.size() << " clusters" << endl;
	cout << "Equality check... " << topTree.isEqual(recoveredTopTree) << endl;

	// unpack recovered top tree
	OrderedTree<TreeNode,TreeEdge> recoveredTree;
	vector<string> newLabels;
	TopTreeUnpacker<OrderedTree<TreeNode,TreeEdge>> unpacker(recoveredTopTree, recoveredTree, newLabels);
	unpacker.unpack();
	cout << "Unpacked recovered top tree in " << timer.getAndReset() << "ms: " << recoveredTree.summary() << endl;

	XmlWriter<OrderedTree<TreeNode,TreeEdge>> unpackedWriter(recoveredTree, newLabels);
	unpackedWriter.write(outputfolder + "/unpacked.xml");
	cout << "Wrote recovered tree in " << timer.getAndReset() << "ms" << endl;

	return 0;
}
