#include "ordering.hpp"
#include <iostream>
#include <set>
#include <cassert>
#include <list>
#include <vector>
#include <algorithm>
#include <fstream>
#include <queue>
#include <limits.h>
#include <chrono>
#include <string>
#include <sstream>

using namespace std;
namespace rabbit
{

// Class Ordering

uint Ordering::Vertex::labelCounter = 0;

Ordering::Ordering(string filename, bool symmetric, bool zero_based, bool write_graph) 
	: symmetric(symmetric), valuesExist(valuesExist), writeGraph(write_graph)  {
	/* Input Format: first two lines contain header info [dimension widhts & # of edges]
	 * First line: % width1 width2 ... widthN
	 * Second line: #_of_edges
	 * Next <#_of_edges> lines: vertex1 vertex2 weight
	 */

	chrono::high_resolution_clock::time_point begin, end;

	// 1 - Create the input stream
	ifstream is(filename);
	if (!is.is_open()) {
		throw InputFileErrorException();
	}
	cout << "Start: read the graph file" << endl;

	begin = chrono::high_resolution_clock::now();

	// 2 - read the first line [header info] & set values of member variables
	string line_buffer;
	getline(is, line_buffer);
	num_vertices = 0;
	if (line_buffer[0] != '%') {
		cerr << "Graph file is incompatible - header info not found" << endl;
	}
	istringstream iss(line_buffer);
	iss >> line_buffer;
	while (!iss.eof()) {
		string current_width;
		iss >> current_width;
		try {
			num_vertices += stoi(current_width);
			dimension_widths.push_back(stoi(current_width));
		}
		catch (...) {
			break;
		}
	}

	getline(is, line_buffer);
	if (line_buffer[0] != '%') {
		cerr << "Graph file is incompatible - header info not found" << endl;
	}
	iss = istringstream(line_buffer);
	iss >> line_buffer; // read the "%" out
	iss >> num_edges;
	vertices.resize(num_vertices);
	new_id = num_vertices;
	dendrogram = Dendrogram(num_vertices);
	cout << num_vertices << " vertices " << num_edges << " edges" << endl;
	
	// 3 - read the edges of the graph
	uint num_edges_read = 0;
	for (uint current_edge = 0; !is.eof(); current_edge++, num_edges_read++) {
		string vertex1, vertex2, weight; // weight is also provided as unsigned integers, always
		/*
		if (is.eof())
		  break; // possible bug [not reading the last line] - but needed
		 */

		if (is.fail()) {
		  cerr << "input stream failed" << endl;
		  throw InvalidInputException();
		}

		try {
			is >> vertex1 >> vertex2 >> weight;
			insertEdge(stoi(vertex1), stoi(vertex2), stoi(weight));
			if (symmetric) {
				insertEdge(stoi(vertex2), stoi(vertex1), stoi(weight));
			}
		}
		catch (...) {
			break;
		}
	}

	if (num_edges_read != num_edges) {
		cerr << "Graph file has fewer edges than expected" << endl;
	}

	end = chrono::high_resolution_clock::now();
	cout << "End: read the graph file [" << 
		chrono::duration_cast<chrono::milliseconds>(end - begin).count() << " ms]" << endl;
}

// Class Ordering | Public Member Function Definitions

void Ordering::insertEdge(uint from, uint to, uint value) {
	// 0 - Ensure that <from> exists among the vertices
  if (from >= vertices.size() || from < 0) {
		throw NotFoundException(VERTEX_NOT_FOUND);
  }

	vertices[from].edges.insert({ to, value });
}

void Ordering::rabbitOrder(const string output_filename) {
	// 0 - Copy the graph [if necessary]
	chrono::high_resolution_clock::time_point begin, end;
	vector<Vertex> original_graph;
	if (writeGraph) {
		cout << "Start: prepare a copy of the original graph" << endl;
		begin = chrono::high_resolution_clock::now();
		original_graph = vertices;
		end = chrono::high_resolution_clock::now();
		cout << "End: prepare a copy of the original graph [" << chrono::duration_cast<chrono::milliseconds>(end - begin).count() << " ms]" << endl;
	}

	// 1 - Community Detection
	cout << "Start: community detection" << endl;
	begin = chrono::high_resolution_clock::now();
	community_detection();
	end = chrono::high_resolution_clock::now();
	cout << "End: community detection [" << chrono::duration_cast<chrono::milliseconds>(end - begin).count() << " ms]" << endl;

	// 2- Ordering Generation
	cout << "Start: ordering generation" << endl;
	begin = chrono::high_resolution_clock::now();
	new_labels = *ordering_generation(); // memory leak
	end = chrono::high_resolution_clock::now();

	cout << "End: ordering generation ["
		<< chrono::duration_cast<chrono::milliseconds>(end - begin).count() << " ms]" << endl
		<< "Start: write the permutation file" << endl;

	// 3 - Write output
	ofstream os(output_filename);
	// 3.1 - write out header info
	os << "% ";
	for (int i = 0; i < dimension_widths.size(); i++) {
		os << dimension_widths[i] << " ";
	}
	os << endl << "% " << num_vertices << endl;
	// 3.2 - write new labels seperated by spaces
	begin = chrono::high_resolution_clock::now();
	for (vector<uint>::const_iterator it = new_labels.begin(); it != new_labels.end(); it++) {
	  os << *it << " ";
	}
	end = chrono::high_resolution_clock::now();
	os.close();

	cout << "End: write the permutation file [" << chrono::duration_cast<chrono::milliseconds>(end - begin).count() << " ms]" << endl;

	if (!writeGraph)
		return;

	// Outputs asymmetrical graph (i.e. outputs all edges of the graph)

	cout << "Start: write the reordered graph" << endl;
	begin = chrono::high_resolution_clock::now();
	uint old_label = 0;
	for (vector<uint>::const_iterator it = new_labels.begin(); it != new_labels.end(); it++, old_label++) {
		Vertex & currentVertex = vertices[old_label];
		currentVertex.label = *it;
	}
	cout << "End: write the reordered graph" << endl;

	ofstream orderedStream("ordered_graph.txt");
	for (vector<Vertex>::const_iterator it = vertices.begin(); it != vertices.end(); it++) {
		for (unordered_map<uint, uint>::const_iterator edge = it->edges.begin(); edge != it->edges.end(); edge++) {
			orderedStream << it->label << " " << vertices[edge->first].label << " " << edge->second << endl;
		}
	}
	
	end = chrono::high_resolution_clock::now();
	cout << "Ordered graph file has been saved in " << chrono::duration_cast<chrono::milliseconds>(end - begin).count() << " ms" << endl;
}

// Class Ordering | Private Member Function Definitions

void Ordering::mergeVertices(uint u, uint v) {
	// Pre-condition: u and v are neighbors OR u and v are the identical
	// Post-condition: vertex <v> is merged into <u>

	// 0 - If <u> and <v> are identical, no merge operation will be performed
	if (u == v) return;

	// 1 - Relable the vertex that's being merged on to ( <v> --> <v'> )
	vertices[v].label = new_id++;

	auto & u_edges = vertices[u].edges, &v_edges = vertices[v].edges;
	// 2 - Reconnect edges connected to <u>, to <v'>
	for (auto it = u_edges.begin(); it != u_edges.end(); it++) {
		int neighbor_id = it->first;
		if (neighbor_id == v || neighbor_id == u) {
			continue;
		}

		auto & neighbor_edges = vertices[neighbor_id].edges;
		auto findResult = neighbor_edges.find(u);
		assert(findResult != neighbor_edges.end());
		int edgeWeight = findResult->second;
		neighbor_edges.erase(findResult); // 2.1 - erase the edge connecting the current neighbor to <u>

		// 2.2 - Create the edge incident on <u> and <v'>
		findResult = v_edges.find(neighbor_id); 
		if (findResult != v_edges.end()) {
			findResult->second += edgeWeight;
			vertices[neighbor_id].edges[v] += edgeWeight;
		}
		else {
			vertices[it->first].edges.insert({ v, edgeWeight });
			vertices[v].edges.insert({ it->first, edgeWeight });
		}
	}

	// 3 - Build the self-loop on <v'>
	unordered_map<uint, uint>::iterator v_u_edge = v_edges.find(u), v_v_edge = v_edges.find(v), u_u_edge = u_edges.find(u);
	assert(v_u_edge != v_edges.end());
	int loopWeight = 2 * (v_u_edge->second);
	loopWeight += (v_v_edge == v_edges.end() ? 0 : v_v_edge->second);
	loopWeight += (u_u_edge == u_edges.end() ? 0 : u_u_edge->second);
	v_edges.insert({ v, loopWeight });

	// 4 - vertex <u> no more exists, clear it
	u_edges.clear();
	v_edges.erase(v_u_edge);
	vertices[u].merged = true;
}

void Ordering::community_detection() {
	// Sort the array of vertices with respect to increasing order of degree
	vector<Vertex> sortedVertices = vertices;
	sort(sortedVertices.begin(), sortedVertices.end()); // keeps the vertices sorted in increasing order of degree

	vector<Vertex> originalArray = vertices; // we'll modify the <vertices> array by performing merge operations,
	// we'll use <originalArray> to revert back to the original array we had
	
	// 2 - Iterate vertices in increasing order of degree
	for (vector<Vertex>::const_iterator iter = sortedVertices.begin(); iter != sortedVertices.end(); iter++) {
		Vertex & currentVertex = vertices[iter->label];
		if (currentVertex.edges.empty() || currentVertex.merged) { // no merging operations will be performed if degree is 0
			continue;
		}

		std::pair<uint, double> maxModularityNeighbor = { INT_MIN, INT_MIN }; // < vertex_label, modularity >
		auto & edges = currentVertex.edges;
		for (auto edge = edges.begin(); edge != edges.end(); edge++) {
			int neighborLabel = edge->first;
			if (vertices[neighborLabel].label == currentVertex.label) {
				continue;
			}

			double currentModularity = modularity(neighborLabel, iter->label);
			if (currentModularity > maxModularityNeighbor.second) {
				maxModularityNeighbor = { neighborLabel, currentModularity };
			}
		}
		if (maxModularityNeighbor.second > 0) {
			int previousLabel = vertices[maxModularityNeighbor.first].label;
			mergeVertices(iter->label, maxModularityNeighbor.first);
			dendrogram.connect(currentVertex.label, previousLabel);
		}
	}

	// We've modified the <vertices> array, revert it back to it's original state
	vertices = originalArray;
}

const vector<uint> * Ordering::ordering_generation() {
	return dendrogram.DFS();
}

double Ordering::modularity(uint u, uint v) {
	auto edge = vertices[u].edges.find(v);
	assert(edge != vertices[u].edges.end());

	double m = num_edges;
	double weighted_degree_u = 0.0;
	double weighted_degree_v = 0.0;

	for (unordered_map<uint, uint>::iterator u_edge = vertices[u].edges.begin(); u_edge != vertices[u].edges.end(); u_edge++) {
		weighted_degree_u += u_edge->second;
	}
	for (unordered_map<uint, uint>::iterator v_edge = vertices[v].edges.begin(); v_edge != vertices[v].edges.end(); v_edge++) {
		weighted_degree_v += v_edge->second;
	}

	double modularity = ((static_cast<double>(edge->second) / (2.0 * m)) - (weighted_degree_u * weighted_degree_v / ((2.0 * m) * (2.0 * m))));
	return modularity;
}
}
