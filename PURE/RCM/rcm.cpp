#include "rcm.hpp"
#include <vector>
#include <list>
#include <queue>
#include <algorithm>
#include <iostream>
#include <set>
#include <climits>
#include <chrono>

using namespace std;

RCM::RCM(int nodeCount, bool valuesExist, bool symmetric, bool oneBased)
	: valuesExist(valuesExist), symmetric(symmetric), oneBased(oneBased) {
	vertices.resize(nodeCount);
	for (int i = 0; i < nodeCount; i++) {
		unmarkedVertices.insert(i);
	}
}

RCM::RCM(string & iname, bool valuesExist, bool symmetric, bool oneBased) 
	: valuesExist(valuesExist), symmetric(symmetric), oneBased(oneBased) {
	// MatrixMarket input format expected [without comments]
	ifstream is(iname);
	if (!is.is_open()) throw InputFileErrorException();

	cout << "Started taking inputs" << endl;
	auto begin = chrono::high_resolution_clock::now();
	int vertexCount, edgeCount;
	is >> vertexCount >> vertexCount >> edgeCount;
	vertices.resize(vertexCount);
	for (int i = 0; i < edgeCount; i++) {
		int v1, v2, weight;
		if (valuesExist) {
			is >> v1 >> v2 >> weight;
		}
		else {
			is >> v1 >> v2;
		}
		
		// Input check
		int lowerBound = oneBased ? 1 : 0;
		int upperBound = oneBased ? vertexCount : vertexCount - 1;

		if (v1 < lowerBound || v1 > upperBound) 
			throw VertexNotFound(v1);
		if (v2 < lowerBound || v2 > upperBound)
			throw VertexNotFound(v2);

		insertEdge(v1, v2);
		if (symmetric) {
			insertEdge(v2, v1);
		}
	}
	
	auto end = chrono::high_resolution_clock::now();
	cout << "Input has been processed in " << chrono::duration_cast<chrono::milliseconds>(end - begin).count() << " ms" << endl;

	// Initialize the unvisited vertices set
	for (int i = 0; i < vertexCount; i++) {
		unmarkedVertices.insert(i);
	}
}

void RCM::insertEdge(int v1, int v2) {
	// Pre-condition: the new edge doesn't exist
	if (oneBased) {
		v1 -= 1;
		v2 -= 1;
	}
	vertices[v1].neighbors.push_back(v2);
}

void RCM::relabel() {
	// Pre-condition: At least 2 vertices exist in <vertices>

	cout << "Started relabeling vertices" << endl;
	auto begin = chrono::high_resolution_clock::now();

	while (!unmarkedVertices.empty()) {
		// 1 - Find the vertex having smallest degree
		pair<int, int> smallDegreeVertex = { 0, INT_MAX }; // < label, degree >
		for (vector<Vertex>::iterator it = vertices.begin(); it != vertices.end(); it++) {

			if (it->neighbors.size() < smallDegreeVertex.second && !it->visited) {
				smallDegreeVertex = { (it - vertices.begin()), it->neighbors.size() };
			}
		}

		// 2 - For each vertex V, add to the list the neighors of V WRT increasing order of degree
		vertices[smallDegreeVertex.first].visited = true;
		unmarkedVertices.erase(smallDegreeVertex.first);
		new_labels.push_back(smallDegreeVertex.first);
		bool componentMarked = true; // will be false when all vertices belonging to a connected subgraph are marked
		while (componentMarked) {
			componentMarked = false;
			Vertex & currentVertex = vertices[new_labels.front()];
			currentVertex.neighbors.sort();
			for (list<int>::iterator it = currentVertex.neighbors.begin(); it != currentVertex.neighbors.end(); it++) {
				if (!vertices[*it].visited) {
					vertices[*it].visited = true;
					componentMarked = true;
					unmarkedVertices.erase(*it);
					new_labels.push_back(*it);
				}
			}
		}
	}

	auto end = chrono::high_resolution_clock::now();
	cout << "Vertices has been relabeled in " << chrono::duration_cast<chrono::milliseconds>(end - begin).count() << " ms" << endl
		<< "Reversing labels" << endl;

	begin = chrono::high_resolution_clock::now();
	
	// 3 - Reverse the order of elements
	list<int>::iterator front = new_labels.begin();
	list<int>::reverse_iterator back = new_labels.rbegin();
	int frontPosition = 0;
	int backPosition = vertices.size() - 1;
	while (frontPosition < backPosition) {
		swap(*front, *back);
		frontPosition++;
		backPosition--;
		front++;
		back++;
	}

	end = chrono::high_resolution_clock::now();
	cout << "Labels have been reversed in " << chrono::duration_cast<chrono::milliseconds>(end - begin).count() << " ms" << endl;
}

void RCM::printNewLabels(string & oname) const {
	ofstream os(oname);
	cout << "Preparing the permutation file" << endl;
	auto begin = chrono::high_resolution_clock::now();

	for (list<int>::const_iterator it = new_labels.begin(); it != new_labels.end(); it++) {
		os << *it << endl;
	}

	auto end = chrono::high_resolution_clock::now();
	cout << "Permutation file has been prepared in " << chrono::duration_cast<chrono::milliseconds>(end - begin).count() << " ms" << endl;
}