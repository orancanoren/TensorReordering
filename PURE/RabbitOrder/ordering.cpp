#include "ordering.hpp"
#include <iostream>
#include <unordered_map>
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

#define __STDC_LIMIT_MACOS

using namespace std;

// Class Ordering

uint Ordering::Vertex::labelCounter = 0;

Ordering::Ordering(string filename, bool symmetric, bool zero_based, bool write_graph) 
	: symmetric(symmetric), valuesExist(valuesExist), writeGraph(writeGraph)  {
	/* Input Format:
	 * First line: <vertex count> <edge count>
	 * For any line <i> of remaining lines:
	 * <neighbor 1 of vertex <i>> <edge weight> <neighbor 2 of vertex<i>> <edge weight>
	 */

	chrono::high_resolution_clock::time_point begin, end;

	// 1 - Create and read the input stream
	ifstream is(filename);
	if (!is.is_open()) throw InputFileErrorException();
	cout << "Start: read the graph file" << endl;

	uint vertexCount, edgeCount;
	begin = chrono::high_resolution_clock::now();
	int header_end_indicator;

	is >> vertexCount >> edgeCount >> header_end_indicator;
	vertices.resize(vertexCount);
	dendrogram = Dendrogram(vertexCount);

	for (uint current_vertex = 0; !is.eof(); current_vertex++) {
		string line;
		getline(is, line);

		if (is.fail())
			throw InvalidInputException();

		// Parse the current line
		istringstream iss(line);
		while (!iss.eof()) {
			uint neighbor, weight;
			iss >> neighbor >> weight;

			if (iss.fail())
				throw InvalidInputException();

			if (zero_based) {
				neighbor -= 1;
			}

			insertEdge(current_vertex, neighbor, weight);
			if (symmetric) {
				insertEdge(neighbor, current_vertex, weight);
			}
		}
	}

	// 2 - Set the values of member variables according to the read data
	new_id = vertexCount;
	edgeCounter = edgeCount;

	end = chrono::high_resolution_clock::now();
	cout << "End: read the graph file [" << 
		chrono::duration_cast<chrono::milliseconds>(end - begin).count() << " ms]" << endl;
}

// Class Ordering | Public Member Function Definitions

void Ordering::insertEdge(uint from, uint to, uint value) {
	// 0 - Ensure that <from> exists among the vertices
	if (from >= vertices.size() || from < 0) 
		throw NotFoundException(VERTEX_NOT_FOUND);

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

	unordered_map<uint, uint> & u_edges = vertices[u].edges, &v_edges = vertices[v].edges;
	// 2 - Reconnect edges connected to <u>, to <v'>
	for (unordered_map<uint, uint>::iterator it = u_edges.begin(); it != u_edges.end(); it++) {
		int neighbor_id = it->first;
		if (neighbor_id == v || neighbor_id == u) {
			continue;
		}

		unordered_map<uint, uint> & neighbor_edges = vertices[neighbor_id].edges;
		unordered_map<uint, uint>::iterator findResult = neighbor_edges.find(u);
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

	// 1 - Set all the edge weights to 1
	for (uint i = 0; i < vertices.size(); i++) {
		unordered_map<uint, uint> & edges = vertices[i].edges;
		for (unordered_map<uint, uint>::iterator it = edges.begin(); it != edges.end(); it++) {
			it->second = 1;
		}
	}
	
	// 2 - Iterate vertices in increasing order of degree
	for (vector<Vertex>::const_iterator iter = sortedVertices.begin(); iter != sortedVertices.end(); iter++) {
		Vertex & currentVertex = vertices[iter->label];
		if (currentVertex.edges.empty() || currentVertex.merged) { // no merging operations will be performed if degree is 0
			continue;
		}

		std::pair<uint, double> maxModularityNeighbor = { INT_MIN, INT_MIN }; // < vertex_label, modularity >
		unordered_map<uint, uint> & edges = currentVertex.edges;
		for (unordered_map<uint, uint>::const_iterator edge = edges.begin(); edge != edges.end(); edge++) {
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
	unordered_map<uint, uint>::iterator edge = vertices[u].edges.find(v);
	assert(edge != vertices[u].edges.end());

	double m = edgeCounter;
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
