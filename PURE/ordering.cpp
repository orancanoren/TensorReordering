#include "ordering.hpp"
#include <iostream>
#include <unordered_map>
#include <cassert>
#include <list>
#include <vector>
#include <algorithm>

#define _DEBUG_HIGH

using namespace std;

// Class Ordering

int Ordering::Vertex::labelCounter = 0;

Ordering::Ordering(int vertexCount) : dendrogram(vertexCount){
#ifdef _DEBUG_HIGH
	cout << "Ordering constructor invoked" << endl;
#endif
	vertices.resize(vertexCount); // theta(V)
	new_id = vertexCount;
	edgeCounter = 0;
}

Ordering::~Ordering() {
#ifdef _DEBUG_HIGH
	cout << "Ordering destructor invoked" << endl;
#endif
}

// Class Ordering | Public Member Function Definitions

void Ordering::insertEdge(int from, int to) {
#ifdef _DEBUG_HIGH
	cout << "Ordering::insertEdge() invoked" << endl;
#endif
	if (from >= vertices.size() || from < 0) throw NotFound(VERTEX_NOT_FOUND);

	vertices[from].edges.insert({ to, 1 });
	vertices[to].edges.insert({ from, 1 });
	edgeCounter++;
}

void Ordering::rabbitOrder() {
	community_detection();
	const vector<int> new_labels = ordering_generation();
	int index = 0;
	for (vector<int>::const_iterator it = new_labels.begin(); it != new_labels.end(); it++, index++) {
		cout << index << " -> " << *it << endl;
	}
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
	for (unordered_map<int, int>::iterator it = u_edges.begin(); it != u_edges.end(); it++) { // iterate over edges of <u>
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
	// IMPORTANT NOTE: The code assumes the degree of <v> doesn't change (it counts the loop)
}

void Ordering::community_detection() {
	// Sort the array of vertices with respect to increasing order of degree
	vector<Vertex> sortedVertices = vertices;
	sort(sortedVertices.begin(), sortedVertices.end());

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