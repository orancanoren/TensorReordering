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

#define __STDC_LIMIT_MACOS

using namespace std;

// Class Ordering

int Ordering::Vertex::labelCounter = 0;

Ordering::Ordering(int vertexCount, bool symmetric) : dendrogram(vertexCount), symmetric(symmetric) {
#ifdef _DEBUG_HIGH
	cout << "Ordering constructor invoked" << endl;
#endif
	vertices.resize(vertexCount);
	new_id = vertexCount;
	edgeCounter = 0;
	edgeInserted = false;
}

Ordering::Ordering(ifstream & is, bool symmetric, bool valuesExist, bool zeroBased) : symmetric(symmetric), valuesExist(valuesExist)  {
	// Input Format:
	// * First line: <vertex count> <vertex count> <edge count>
	// <vertex id> <vertex id> <weight>
#ifdef _DEBUG_HIGH
	cout << "Ordering constructor invoked" << endl;
#endif
	if (!is.is_open()) throw InputFileErrorException();

	cout << "Started taking inputs from stream" << endl;
	auto begin = chrono::high_resolution_clock::now();
	unsigned int vertexCount, edgeCount;
	is >> vertexCount >> vertexCount >> edgeCount;
	vertices.resize(vertexCount);

	dendrogram = Dendrogram(vertexCount);

	for (int i = 0; i < edgeCount; i++) {
		if (valuesExist) {
			int v1, v2, weight;
			is >> v1 >> v2 >> weight;
			if (is.fail()) {
				throw InvalidInputException();
			}
			
			if (!zeroBased) {
				v1 -= 1;
				v2 -= 1;
			}
			insertEdge(v1, v2);
			if (symmetric) {
				insertEdge(v2, v1);
			}
		}
		else {
			int v1, v2;
			is >> v1 >> v2;
			if (is.fail()) {
				throw InvalidInputException();
			}

			if (!zeroBased) {
				v1 -= 1;
				v2 -= 1;
			}
			insertEdge(v1, v2);
			if (symmetric) {
				insertEdge(v2, v1);
			}
		}
	}
	new_id = vertexCount;
	edgeCounter = edgeCount;
	if (!symmetric) {
		edgeCounter /= 2;
	}
	auto end = chrono::high_resolution_clock::now();
	cout << "Input has been processed in " << 
		chrono::duration_cast<chrono::milliseconds>(end - begin).count() << " ms" << endl;
#ifdef _DEBUG_HIGH
	cout << "Graph has been read successfully" << endl;
#endif
}

Ordering::~Ordering() {
#ifdef _DEBUG_HIGH
	cout << "Ordering destructor invoked" << endl;
#endif
}

// Class Ordering | Public Member Function Definitions

void Ordering::insertEdge(int from, int to, int value) {
#ifdef _DEBUG_HIGH
	cout << "Ordering::insertEdge() invoked" << endl;
#endif
	if (from >= vertices.size() || from < 0) throw NotFoundException(VERTEX_NOT_FOUND);

	vertices[from].edges.insert({ to, value });
	if (symmetric) {
		vertices[to].edges.insert({ from, value });
	}
	if (!symmetric) {
		if (edgeInserted) {
			edgeInserted = false;
		}
		else {
			edgeCounter++;
		}
	}
	else {
		edgeCounter++;
	}
}

void Ordering::rabbitOrder(ofstream & os) {
	// 1 - Community Detection
	chrono::high_resolution_clock::time_point begin = chrono::high_resolution_clock::now();
	community_detection();
	chrono::high_resolution_clock::time_point end = chrono::high_resolution_clock::now();

	cout << "Community Detection has been completed in "
		<< chrono::duration_cast<chrono::milliseconds>(end - begin).count() << " ms" << endl;

	// 2- Ordering Generation
	begin = chrono::high_resolution_clock::now();
	new_labels = ordering_generation();
	end = chrono::high_resolution_clock::now();
	cout << "Ordering Generation has been completed in "
		<< chrono::duration_cast<chrono::milliseconds>(end - begin).count() << " ms" << endl;

	for (vector<int>::const_iterator it = new_labels.begin(); it != new_labels.end(); it++) {
	  os << *it << " ";
	}
	cout << "New permutations has been saved to file" << endl;
}

// Class Ordering | Private Member Function Definitions

void Ordering::mergeVertices(int u, int v) {
	// Pre-condition: u and v are neighbors OR u and v are the identical
	// Post-condition: vertex <v> is merged into <u>
#ifdef _DEBUG_HIGH
	cout << "Ordering::mergeVertices() invoked" << endl;
#endif
	// 0 - If <u> and <v> are identical, no merge operation will be performed
	if (u == v) return;

	// 1 - Relable the vertex that's being merged on to ( <v> --> <v'> )
	vertices[v].label = new_id++;

	unordered_map<int, int> & u_edges = vertices[u].edges, &v_edges = vertices[v].edges;
	// 2 - Reconnect edges connected to <u>, to <v'>
	for (unordered_map<int, int>::iterator it = u_edges.begin(); it != u_edges.end(); it++) {
		int neighbor_id = it->first;
		if (neighbor_id == v || neighbor_id == u) {
			continue;
		}

		unordered_map<int, int> & neighbor_edges = vertices[neighbor_id].edges;
		unordered_map<int, int>::iterator findResult = neighbor_edges.find(u);
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
	unordered_map<int, int>::iterator v_u_edge = v_edges.find(u), v_v_edge = v_edges.find(v), u_u_edge = u_edges.find(u);
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
	for (int i = 0; i < vertices.size(); i++) {
		unordered_map<int, int> & edges = vertices[i].edges;
		for (unordered_map<int, int>::iterator it = edges.begin(); it != edges.end(); it++) {
			it->second = 1;
		}
	}
	
	// 2 - Iterate vertices in increasing order of degree
	for (vector<Vertex>::const_iterator iter = sortedVertices.begin(); iter != sortedVertices.end(); iter++) {
		Vertex & currentVertex = vertices[iter->label];
		if (currentVertex.edges.empty() || currentVertex.merged) { // no merging operations will be performed if degree is 0
			continue;
		}

		std::pair<int, double> maxModularityNeighbor = { INT_MIN, INT_MIN }; // < vertex_label, modularity >
		unordered_map<int, int> & edges = currentVertex.edges;
		for (unordered_map<int, int>::const_iterator edge = edges.begin(); edge != edges.end(); edge++) {
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

const vector<int> Ordering::ordering_generation() {
	return dendrogram.DFS();
}

double Ordering::modularity(int u, int v) {
	unordered_map<int, int>::iterator edge = vertices[u].edges.find(v);
	assert(edge != vertices[u].edges.end());

	double m = edgeCounter;
	double weighted_degree_u = 0.0;
	double weighted_degree_v = 0.0;

	for (unordered_map<int, int>::iterator u_edge = vertices[u].edges.begin(); u_edge != vertices[u].edges.end(); u_edge++) {
		weighted_degree_u += u_edge->second;
	}
	for (unordered_map<int, int>::iterator v_edge = vertices[v].edges.begin(); v_edge != vertices[v].edges.end(); v_edge++) {
		weighted_degree_v += v_edge->second;
	}

	double modularity = (((double)edge->second / (2.0 * m)) - (weighted_degree_u * weighted_degree_v / ((2.0 * m) * (2.0 * m))));
	return modularity;
}
