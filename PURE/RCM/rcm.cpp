#include "rcm.hpp"
#include <vector>
#include <list>
#include <queue>
#include <algorithm>
#include <iostream>
#include <set>
#include <climits>

using namespace std;

RCM::RCM(int nodeCount, bool valuesExist, bool symmetric, bool oneBased)
	: valuesExist(valuesExist), symmetric(symmetric), oneBased(oneBased) {
	vertices.resize(nodeCount);
	for (int i = 0; i < nodeCount; i++) {
		unmarkedVertices.insert(i);
	}
}

RCM::RCM(ifstream & is, bool valuesExist, bool symmetric, bool oneBased) 
	: valuesExist(valuesExist), symmetric(symmetric), oneBased(oneBased) {
	// MatrixMarket input format expected [without comments]
	if (!is.is_open()) throw InputFileErrorException();

	int vertexCount, edgeCount;
	is >> vertexCount >> edgeCount;
	for (int i = 0; i < vertexCount; i++) {
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

		if (v1 < lowerBound || v1 > upperBound || v2 < lowerBound || v2 > upperBound) 
			throw InvalidInputException();

		insertEdge(v1, v2);
		if (symmetric) {
			insertEdge(v2, v1);
		}
	}
}

void RCM::insertEdge(int v1, int v2) {
	// Pre-condition: the new edge doesn't exist

	vertices[v1].neighbors.push_back(v2);
	if (symmetric) {
		vertices[v2].neighbors.push_back(v1);
	}
}

void RCM::relabel() {
	// Pre-condition: At least 2 vertices exist in <vertices>

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
}

void RCM::printNewLabels(ofstream & os) const {
	int iterationCounter = 0;
	for (list<int>::const_iterator it = new_labels.begin(); it != new_labels.end(); it++, iterationCounter++) {
		os << iterationCounter << " -> " << *it << endl;
	}
}