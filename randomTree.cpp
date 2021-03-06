/*
 * Generate a random tree and dump it as an XML
 * file or DOT graph. Can also dump top tree and
 * Top DAG as DOT graphs.
 */

#include <iostream>

// Data Structures
#include "Edges.h"
#include "Labels.h"
#include "Nodes.h"
#include "OrderedTree.h"
#include "TopDag.h"
#include "TopTree.h"

// Algorithms
#include "TopDagUnpacker.h"
#include "DotGraphExporter.h"
#include "RandomTree.h"
#include "TopDagConstructor.h"

// Utils
#include "ArgParser.h"
#include "Timer.h"
#include "XML.h"

using std::cout;
using std::endl;

void usage(char* name) {
    cout << "Usage: " << name << " <options>" << endl
         << "  -n <int>   tree size (edges) (default: 10)" << endl
         << "  -l <int>   number of distinct labels" << endl
         << "  -o <str>   output XML filename (default: do not write)" << endl
         << "  -s <int>   seed (default: 12345678)" << endl
         << "  -d         dump DOT graph if tree is small enough" << endl
         << "  -c         construct Top DAG" << endl
         << "  -v         verbose output" << endl;
}

int main(int argc, char **argv) {
    ArgParser argParser(argc, argv);

    if (argParser.isSet("h") || argParser.isSet("-help")) {
        usage(argv[0]);
        return 0;
    }

    const int size = argParser.get<int>("n", 10);
    const int seed = argParser.get<int>("s", 12345678);
    const int numLabels = argParser.get<int>("l", 2);
    const std::string outfn = argParser.get<std::string>("o", "");
    const bool dump = argParser.isSet("d");
    const bool construct = argParser.isSet("c");
    const bool verbose = argParser.isSet("v");

    // Initiliase
    getRandomGenerator().seed(seed);
    RandomTreeGenerator<RandomGeneratorType> rand(getRandomGenerator());
    OrderedTree<TreeNode, TreeEdge> tree;

    // Generate the tree and the labels
    Timer timer;
    rand.generateTree(tree, size, (verbose && size < 1000));
    RandomLabels<RandomGeneratorType> labels(size+1, numLabels, getRandomGenerator());

    if (verbose) cout << "Generated " << tree.summary() << " in " << timer.get() << "ms" << endl;
    timer.reset();

    if (outfn != "") {
        // Write output XML
        XmlWriter<OrderedTree<TreeNode, TreeEdge>>::template write<int>(tree, labels, outfn);
    }

    if (dump) {
        // Dump some various stuff if the tree is small
        if (size <= 30) cout << tree << endl;

        if (size <= 10000) {
            OrderedTreeDotGraphExporter<TreeNode, TreeEdge, int>().write(tree, labels, "/tmp/tree.dot");
            cout << "Wrote DOT file in " << timer.getAndReset() << "ms" << endl;
        }

        if (size <= 1000) {
            DotGraphExporter<OrderedTree<TreeNode, TreeEdge>>::drawSvg("/tmp/tree.dot", "/tmp/tree.svg");
            cout << "Graphed DOT file in " << timer.getAndReset() << "ms" << endl;
        }
    }

    // Abort here if we don't want the top tree / DAG
    if (!construct) {
        return 0;
    }

    const int treeEdges = tree._numEdges;
    TopDag<int> dag(tree._numNodes, labels);
    TopDagConstructor<OrderedTree<TreeNode, TreeEdge>, int> topDagConstructor(tree, dag);

    timer.reset();
    topDagConstructor.construct();
    if (verbose) cout << "Top DAG construction took " << timer.get() << "ms" << endl;
    timer.reset();

    const int edges = dag.countEdges();
    const double percentage = (edges * 100.0) / treeEdges;
    const double ratio = ((int)(1000 / percentage)) / 10.0;
    if (verbose)
        cout << "Top dag has " << dag.nodes.size() - 1 << " nodes, " << edges << " edges (" << percentage
             << "% of original tree, " << ratio << ":1)" << endl;

    if (dump) {
        if (size <= 10000) {
            TopDagDotGraphExporter<int>().write(dag, "/tmp/topdag.dot");
            if (verbose) cout << "Wrote DOT file in " << timer.get() << "ms" << endl;
            timer.reset();
        }

        if (size <= 1000) {
            TopDagDotGraphExporter<int>::drawSvg("/tmp/topdag.dot", "/tmp/topdag.svg");
            if (verbose) cout << "Graphed DOT file in " << timer.get() << "ms" << endl;
            timer.reset();
        }
    }
}
