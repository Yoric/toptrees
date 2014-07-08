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
//*
	Labels<string> labels;

	string filename = argc > 1 ? string(argv[1]) : "data/1998statistics.xml";

	XmlParser<OrderedTree<TreeNode,TreeEdge>> xml(filename, t, labels);
	xml.parse();

	cout << t.summary() << "; Height: " << t.height() << " Avg depth: " << t.avgDepth() << endl;

	TopTree topTree(t._numNodes, labels);
	vector<int> nodeIds(t._numNodes);
	for (int i = 0; i < t._numNodes; ++i) {
		nodeIds[i] = i;
	}

	Timer timer;
	t.doMerges([&] (const int u, const int v, const int n, const MergeType type) {
		nodeIds[n] = topTree.addCluster(nodeIds[u], nodeIds[v], type);
	});

	cout << "Top tree construction took " << timer.getAndReset() << "ms; Top tree has " << topTree.clusters.size() << " clusters (" << topTree.clusters.size() - t._numNodes << " non-leaves)" << endl;

	BinaryDag<string> dag;
	DagBuilder<string> builder(topTree, dag);
	builder.createDag();

	const int edges = dag.countEdges();
	const double percentage = (edges * 100.0) / topTree.numLeaves;
	const double ratio = ((int)(1000/percentage))/10.0;
	cout << "Top dag has " << dag.nodes.size() - 1<< " nodes, " << edges << " edges (" << percentage << "% of original tree, " << ratio << ":1)" << endl;
	cout << "Top dag construction took in " << timer.elapsedMillis() << "ms" << endl;

/*/

	int foo = argc;
	foo++;
	char** bar = argv;
	bar++;

	t.addNodes(11);
	t.addEdge(0, 1);
	t.addEdge(0, 2);
	t.addEdge(0, 3);
	t.addEdge(1, 4);
	t.addEdge(1, 5);
	t.addEdge(3, 6);
	t.addEdge(6, 7);
	t.addEdge(7, 8);
	t.addEdge(4, 9);
	t.addEdge(4, 10);
	cout << t << endl << endl;

	vector<string> labels({"root", "chain", "chain", "chain", "chain", "chain", "chain", "chain", "chain", "chain", "chain"});

	t.toString();

	TopTree topTree(t._numNodes, labels);
	vector<int> nodeIds(t._numNodes);
	for (int i = 0; i < t._numNodes; ++i) {
		nodeIds[i] = i;
	}
	t.doMerges([&] (const int u, const int v, const int n, const MergeType type) {
		cout << "merged " << u  << " and " << v << " into " << n << " type " << type << endl;
		nodeIds[n] = topTree.addCluster(nodeIds[u], nodeIds[v], type);
	});
	cout << endl << t << endl;
	cout << topTree << endl;
	BinaryDag<string> dag;
	DagBuilder<string> builder(topTree, dag);
	builder.createDag();
	cout << dag << endl;

	XmlWriter<TopTree> writer(topTree);
	writer.write("/tmp/toptree.xml");

	TopTree newTopTree(t._numNodes);
	BinaryDagUnpacker<string> dagUnpacker(dag, newTopTree);
	dagUnpacker.unpack();
	cout << newTopTree << endl;

	OrderedTree<TreeNode, TreeEdge> newTree;
	TopTreeUnpacker<OrderedTree<TreeNode, TreeEdge>> unpacker(topTree, newTree);
	unpacker.unpack();

	//XmlWriter<OrderedTree<TreeNode, TreeEdge>> unpackedWriter(newTree, labels);
	//unpackedWriter.write("/tmp/unpacked.xml");

//*/
	return 0;
}
