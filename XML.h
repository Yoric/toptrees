#pragma once

#include <fstream>
#include <iostream>
#include <vector>

#include "Timer.h"
#include "TopTree.h"

#include "3rdparty/rapidxml.hpp"
#include "3rdparty/rapidxml_utils.hpp"

using std::cout;
using std::endl;
using std::flush;
using std::string;
using std::vector;

template<typename TreeType>
class XmlParser {
public:
	// TODO figure out if we can keep the char pointers instead of converting them to string
	// this currently uses more than half of the parsing time
	XmlParser(const string& fn, TreeType &t, vector<string> &l): filename(fn), tree(t), labels(l) {}

	void parse(const bool verbose = true) {
		if (verbose) cout << "Reading " << filename << "… " << flush;
		Timer timer;

		rapidxml::file<> file(filename.c_str());
		rapidxml::xml_document<> xml;
		if (verbose) cout << " " << timer.getAndReset() << "ms; parsing… " << flush;

		xml.parse<rapidxml::parse_no_data_nodes>(file.data());
		rapidxml::xml_node<>* root = xml.first_node();
		if (verbose) cout << timer.getAndReset() << "ms; building tree… " << flush;

		int rootId = tree.addNode();
		assert((int)labels.size() == rootId);
		labels.push_back(root->name());
		parseStructure(root, rootId);

		if (verbose) cout << timer.elapsedMillis() << "ms." << endl;
	}

private:
	void parseStructure(rapidxml::xml_node<>* node, int id) {
		rapidxml::xml_node<>* child = node->first_node();
		int numChildren(0), childId;
		while (child != NULL) {
			numChildren++;
			childId = tree.addNode();
			tree.addEdge(id, childId);
			assert((int)labels.size() == childId);
			labels.push_back(child->name());
			child = child->next_sibling();
		}
		if (numChildren == 0) return;
		childId -= numChildren - 1;
		child = node->first_node();
		while (child != NULL) {
			parseStructure(child, childId);
			childId++;
			child = child->next_sibling();
		}
	}

	string filename;
	TreeType &tree;
	vector<string> &labels;
};

class XmlWriter {
public:
	XmlWriter(TopTree &tree): tree(tree) {}

	void write(const string &filename) {
		std::ofstream out(filename.c_str());
		assert(out.is_open());

		int rootId = tree.clusters.size() - 1;
		writeNode(out, rootId, 0);

		out.close();
	}

private:
	void writeNode(std::ofstream &out, const int nodeId, const int depth) {
		auto &node = tree.clusters[nodeId];
		const string& label(node.label != NULL ? *node.label : "DUMMY");
		for (int i = 0; i < depth; ++i) out << "\t";
		out << "<" << label << ">" << endl;

		if (node.left >= 0) {
			writeNode(out, node.left, depth + 1);
		}
		if (node.right >= 0) {
			writeNode(out, node.right, depth + 1);
		}

		for (int i = 0; i < depth; ++i) out << "\t";
		out << "</" << label << ">" << endl;
	}

	TopTree &tree;
};
