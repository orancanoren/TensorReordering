#include "rcm.hpp"
#include <vector>
#include <list>
#include <queue>
#include <algorithm>
#include <iostream>
#include <set>
#include <climits>
#include <chrono>
#include <utility>

using namespace std;
namespace rcm
{

RCM::RCM(string & iname, bool valuesExist, bool symmetric, bool oneBased, bool degree_based) 
	: valuesExist(valuesExist), symmetric(symmetric), oneBased(oneBased), degree_based(degree_based) {
	// MatrixMarket input format expected [without comments]
	ifstream is(iname);
	if (!is.is_open()) throw InputFileErrorException();

	cout << "Started taking inputs" << endl;
	auto begin = chrono::high_resolution_clock::now();
	int vertexCount, edgeCount;
	is >> vertexCount >> vertexCount >> edgeCount;
	vertices.resize(vertexCount);
	for (int i = 0; i < edgeCount; i++) {
		int v1, v2;
		float weight = 1;
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

		insertEdge(v1, v2, weight);
		if (symmetric) {
			insertEdge(v2, v1, weight);
		}
	}
	
	auto end = chrono::high_resolution_clock::now();
	cout << "Input has been processed in " << chrono::duration_cast<chrono::milliseconds>(end - begin).count() << " ms" << endl;

	// Initialize the unvisited vertices set
	for (int i = 0; i < vertexCount; i++) {
		unmarkedVertices.insert(i);
	}
}

void RCM::insertEdge(int v1, int v2, float weight) {
	// Pre-condition: the new edge doesn't exist
	if (oneBased) {
		v1 -= 1;
		v2 -= 1;
	}
	pair<int, int> edge = { v1, weight };
	vertices[v1].neighbors.push_back(edge);
}

void RCM::relabel() {
	// Pre-condition: At least 2 vertices exist in <vertices>

	cout << "Started relabeling vertices" << endl;
	auto begin = chrono::high_resolution_clock::now();

	while (!unmarkedVertices.empty()) {
		// 1 - Find the vertex having smallest degree / total degree weight
		pair<int, float> smallDegreeVertex = { 0, INT_MAX }; // < label, degree >
		for (vector<Vertex>::iterator it = vertices.begin(); it != vertices.end(); it++) {

			if (degree_based && it->neighbors.size() < smallDegreeVertex.second && !it->visited) {
				smallDegreeVertex = { (it - vertices.begin()), it->neighbors.size() };
			}
			else if (!degree_based) {
				float weight_sum = 0;
				for (list< pair<int, float> >::const_iterator neighbor = it->neighbors.cbegin(); neighbor != it->neighbors.cend(); neighbor++) {
					weight_sum += neighbor->second;
				}
				if (weight_sum < smallDegreeVertex.second && !it->visited) {
					smallDegreeVertex = { (it - vertices.begin()), weight_sum };
				}
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

			// ===== Comparison & sorting =======
			Comparator comp(*this);
			currentVertex.neighbors.sort(comp); 
			/* comparator function is a functor provided by the class [access to class members 
			is required for the comparator to work - see RCM::operator()()]*/

			for (list< pair<int, float> >::iterator it = currentVertex.neighbors.begin(); it != currentVertex.neighbors.end(); it++) {
				if (!vertices[it->first].visited) {
					vertices[it->first].visited = true;
					componentMarked = true;
					unmarkedVertices.erase(it->first);
					new_labels.push_back(it->first);
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

bool RCM::Comparator::operator()(const EDGE & lhs, const EDGE & rhs) {
	if (!rcm_obj.degree_based) {
		float sum_lhs = 0, sum_rhs = 0;
		for (EDGE_LIST::const_iterator it = rcm_obj.vertices[lhs.first].neighbors.cbegin(); it != rcm_obj.vertices[lhs.first].neighbors.cend(); it++) {
			sum_lhs += it->second;
		}
		for (EDGE_LIST::const_iterator it = rcm_obj.vertices[rhs.first].neighbors.cbegin(); it != rcm_obj.vertices[rhs.first].neighbors.cend(); it++) {
			sum_rhs += it->second;
		}
		return sum_lhs < sum_rhs;
	}
	else {
		return rcm_obj.vertices[lhs.first].neighbors.size() < rcm_obj.vertices[rhs.first].neighbors.size();
	}
}
}
