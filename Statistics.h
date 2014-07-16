#pragma once

#include <algorithm>
#include <cassert>
#include <iomanip>
#include <mutex>
#include <ostream>
#include <fstream>
#include <vector>


/// A statistics writer (thread-safe)
/// Uses locking for all operations, feel free to do crazy things with threads
struct StatWriter {
	void open(const std::string &filename) {
		mutex.lock();
		out.open(filename.c_str());
		mutex.unlock();
	}

	void close() {
		mutex.lock();
		if (out.is_open())
			out.close();
		mutex.unlock();
	}

	template <typename T>
	void write(const T &data, const bool newLine = true) {
		mutex.lock();
		if (out.is_open()) {
			out << data;
			if (newLine)
				out << std::endl;
		}
		mutex.unlock();
	}

	std::mutex mutex;
	std::ofstream out;
};

static StatWriter edgeRatioWriter;
static StatWriter debugInfoWriter;

/// Holds debug information about a tree compression run
struct DebugInfo {
	/// the time it took to generate the tree (in milliseconds)
	double generationDuration;
	/// the time it took to transform the tree into its top tree (in milliseconds)
	double mergeDuration;
	/// the time it took to compute the top tree's minimal DAG (in milliseconds)
	double dagDuration;
	/// the minimum ratio of edges before and after an iteration (there are theoretical bounds on this)
	double minEdgeRatio;
	/// the maximum ratio of edges before and after an iteration
	double maxEdgeRatio;
	/// the sum of the edge ratios
	double edgeRatios;
	/// the number of edge ratios summed up in ::edgeRatios
	int numEdgeRatios;
	/// number of edges in the minimal DAG
	int numDagEdges;
	/// number of nodes in the minimal DAG
	int numDagNodes;
	/// height of the tree
	int height;
	/// average depth of the tree's nodes
	double avgDepth;

	DebugInfo()
		: generationDuration(0.0),
		  mergeDuration(0.0),
		  dagDuration(0.0),
		  minEdgeRatio(9.99),
		  maxEdgeRatio(0.0),
		  edgeRatios(0.0),
		  numEdgeRatios(0),
		  numDagEdges(0),
		  numDagNodes(0),
		  height(0),
		  avgDepth(0.0) {}

	/// the total time it took to perform the relevant (i.e., non-statistical) operations
	double totalDuration() const {
		return generationDuration + mergeDuration + dagDuration;
	}

	/// add a ratio of edges before and after an iteration
	void addEdgeRatio(double ratio) {
		++numEdgeRatios;
		edgeRatios += ratio;
		if (ratio < minEdgeRatio) {
			minEdgeRatio = ratio;
		}
		if (ratio > maxEdgeRatio) {
			maxEdgeRatio = ratio;
		}
		edgeRatioWriter.write(ratio);
	}

	/// the average ratio of edges compressed in all iterations
	double avgEdgeRatio() const {
		return edgeRatios / numEdgeRatios;
	}

	/// add another DebugInfo object to this
	void add(const DebugInfo &other) {
		generationDuration += other.generationDuration;
		mergeDuration += other.mergeDuration;
		dagDuration += other.dagDuration;
		edgeRatios += other.edgeRatios;
		numEdgeRatios += other.numEdgeRatios;
		numDagEdges += other.numDagEdges;
		numDagNodes += other.numDagNodes;
		height += other.height;
		avgDepth += other.avgDepth;
	}

	/// calculate element-wise minimum with another DebugInfo object in-place
	void min(const DebugInfo &other) {
		generationDuration = std::min(generationDuration, other.generationDuration);
		mergeDuration = std::min(mergeDuration, other.mergeDuration);
		dagDuration = std::min(dagDuration, other.dagDuration);
		minEdgeRatio = std::min(minEdgeRatio, other.minEdgeRatio);
		numDagEdges = std::min(numDagEdges, other.numDagEdges);
		numDagNodes = std::min(numDagNodes, other.numDagNodes);
		height = std::min(height, other.height);
		avgDepth = std::min(avgDepth, other.avgDepth);
	}

	/// calculate element-wise maximum with another DebugInfo object in-place
	void max(const DebugInfo &other) {
		generationDuration = std::max(generationDuration, other.generationDuration);
		mergeDuration = std::max(mergeDuration, other.mergeDuration);
		dagDuration = std::max(dagDuration, other.dagDuration);
		minEdgeRatio = std::max(maxEdgeRatio, other.maxEdgeRatio);
		numDagEdges = std::max(numDagEdges, other.numDagEdges);
		numDagNodes = std::max(numDagNodes, other.numDagNodes);
		height = std::max(height, other.height);
		avgDepth = std::max(avgDepth, other.avgDepth);
	}

	/// divide all (reasonable) elements for statistics aggregation
	void divide(const int factor) {
		generationDuration /= factor;
		mergeDuration /= factor;
		dagDuration /= factor;
		numDagEdges /= factor;
		numDagNodes /= factor;
		height /= factor;
		avgDepth /= factor;
	}

	/// Dump this debugInfo object to an output stream (tab-separated values)
	void dump(std::ostream &os) const {
		os << totalDuration() << "\t"
		   << generationDuration << "\t"
		   << mergeDuration << "\t"
		   << dagDuration << "\t"
		   << minEdgeRatio << "\t"
		   << maxEdgeRatio << "\t"
		   << avgEdgeRatio() << "\t"
		   << numDagEdges << "\t"
		   << numDagNodes << "\t"
		   << height << "\t"
		   << avgDepth
		   << std::endl;
	}

	/// Write an explanative header (tab-separated)
	static void dumpHeader(std::ostream &os) {
		os << "totalDuration" << "\t"
		   << "generationDuration" << "\t"
		   << "mergeDuration" << "\t"
		   << "dagDuration" << "\t"
		   << "minEdgeRatio" << "\t"
		   << "maxEdgeRatio" << "\t"
		   << "avgEdgeRatio" << "\t"
		   << "numDagEdges" << "\t"
		   << "numDagNodes" << "\t"
		   << "height" << "\t"
		   << "avgDepth" << std::endl;
	}

	friend std::ostream& operator<<(std::ostream &os, const DebugInfo &info) {
		info.dump(os);
		return os;
	}
};

/// A statistics aggregator
struct Statistics {
	Statistics(const std::string &edgeRatioFilename = "", const std::string &debugInfoFilename = "") : numDebugInfos(0) {
		if (edgeRatioFilename != "") {
			edgeRatioWriter.open(edgeRatioFilename);
		}
		if (debugInfoFilename != "") {
			debugInfoWriter.open(debugInfoFilename);
			DebugInfo::dumpHeader(debugInfoWriter.out);
		}
	}

	~Statistics() {
		edgeRatioWriter.close();
	}

	/// add a debug info object
	void addDebugInfo(const DebugInfo &info) {
		if (numDebugInfos == 0) {
			min = info;
			max = info;
			avg = info;
		} else {
			min.min(info);
			max.max(info);
			avg.add(info);
		}
		debugInfoWriter.write(info, false);
		++numDebugInfos;
	}

	void compute() {
		avg.divide(numDebugInfos);
	}

	void dump(std::ostream &os) const {
		os << std::fixed << std::setprecision(2);
		os << std::endl << "Statistics:" << std::endl << std::endl
		   << "Total duration p. tree: " << avg.totalDuration() << "ms (avg), " << min.totalDuration() << "ms (min), " << max.totalDuration() << "ms (max)" << std::endl
		   << "Random tree generation: " << avg.generationDuration << "ms (avg), " << min.generationDuration << "ms (min), " << max.generationDuration << "ms (max)" << std::endl
		   << "Top Tree construction:  " << avg.mergeDuration << "ms (avg), " << min.mergeDuration << "ms (min), " << max.mergeDuration << "ms (max)" << std::endl
		   << "Top DAG compression:    " << avg.dagDuration << "ms (avg), " << min.dagDuration << "ms (min), " << max.dagDuration << "ms (max)" << std::endl
		   << std::setprecision(6)
		   << "Edge comp. ratio: " << avg.avgEdgeRatio() << " (avg), " << min.minEdgeRatio << " (min), " << max.maxEdgeRatio << " (max)" << std::endl
		   << std::setprecision(2)
		   << "DAG Edges: " << avg.numDagEdges << " (avg), " << min.numDagEdges << " (min), " << max.numDagEdges << " (max)" << std::endl
		   << "DAG Nodes: " << avg.numDagNodes << " (avg), " << min.numDagNodes << " (min), " << max.numDagNodes << " (max)" << std::endl
		   << "Tree height:    " << avg.height << " (avg), " << min.height << " (min), " << max.height << " (max)" << std::endl
		   << "Avg node depth: " << avg.avgDepth << " (avg), " << min.avgDepth << " (min), " << max.avgDepth << " (max)" << std::endl;
	}

	DebugInfo min, max, avg;
	std::ofstream out;
	int numDebugInfos;
};
