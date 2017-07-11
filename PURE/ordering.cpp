#include "ordering.hpp"
#include <iostream>
#include <unordered_map>
#include <cassert>
#include <queue>
#include <vector>
#include <algorithm>

#define _DEBUG_HIGH

using namespace std;

// Utilities

class VertexComparator final : private Ordering {
public:
	bool operator() (const Vertex & v1, const Vertex & v2) {
		return v1.edges.size() < v2.edges.size();
	}
};

// Class Ordering

int Ordering::Vertex::labelCounter = 0;

Ordering::Ordering(int vertexCount) : dendrogram(vertexCount){
#ifdef _DEBUG_HIGH
	cout << "Ordering constructor invoked" << endl;
#endif
	vertices.resize(vertexCount); // theta(V)	
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
	if (u == v) return;

	unordered_map<int, int> & u_edges = vertices[u].edges, &v_edges = vertices[v].edges;
	// 1 - Reconnect edges connected to u, to v
	for (unordered_map<int, int>::iterator it = u_edges.begin(); it != u_edges.end(); it++) {
		if (it->first == v) {
			continue;
		}

		int id = it->first;
		unordered_map<int, int> & it_edges = vertices[it->first].edges;

		// Reconnect the edge incident on <*it> and <u>, such that it's now connecting <*it> and <v>
		// If there already is such an edge, increase its weight
		unordered_map<int, int>::iterator findResult = it_edges.find(u);
		assert(findResult != it_edges.end());
		int edgeWeight = findResult->second;
		it_edges.erase(findResult);


		findResult = v_edges.find(id);
		if (findResult != v_edges.end()) {
			findResult->second += edgeWeight;
			vertices[id].edges[v] += edgeWeight;
		}
		else {
			vertices[it->first].edges.insert({ v, edgeWeight });
			vertices[v].edges.insert({ it->first, edgeWeight });
		}
	}


	// 2 - Build the self-loop on <v'>
	unordered_map<int, int>::iterator v_u_edge = v_edges.find(u), v_v_edge = v_edges.find(v), u_u_edge = u_edges.find(u);
	assert(v_u_edge != v_edges.end());
	int loopWeight = 2 * (v_u_edge->second);
	loopWeight += (v_v_edge == v_edges.end() ? 0 : v_v_edge->second);
	loopWeight += (u_u_edge == u_edges.end() ? 0 : u_u_edge->second);
	v_edges.insert({ v, loopWeight });

	// 3 - vertex u no more exists, clear it
	u_edges.clear();
	v_edges.erase(v_u_edge);
	vertices[u].merged = true;
	// NOTE: The code assumes the degree of <v> doesn't change (it counts the loop)
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

			double currentModularity = modularity(neighborLabel, currentVertex.label);
			if (currentModularity > maxModularityNeighbor.second) {
				maxModularityNeighbor = { neighborLabel, currentModularity };
			}
		}
		mergeVertices(maxModularityNeighbor.first, currentVertex.label);
		dendrogram.connect(currentVertex.label, maxModularityNeighbor.first);
	}
}

const vector<int> Ordering::ordering_generation() {
	return dendrogram.DFS();
}

double Ordering::modularity(int u, int v) {
	unordered_map<int, int>::iterator edge = vertices[u].edges.find(v);
	assert(edge != vertices[u].edges.end());

	unsigned int m = vertices.size();
	double modularity = (((double)edge->second / (2.0 * m))
		- ((double)vertices[u].edges.size() * (double)vertices[v].edges.size() / (2.0 * m * m)));

	return modularity;
}