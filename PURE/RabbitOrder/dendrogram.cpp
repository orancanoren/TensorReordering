#include "dendrogram.hpp"
#include <iostream>
#include <vector>
#include <list>
#include <cassert>
#include <stack>
using namespace std;

Dendrogram::Dendrogram(int nodeCount) : nodeCount(nodeCount) {
#ifdef _DEBUG_HIGH
	cout << "Dendrogram constructor invoked" << endl;
#endif
	for (int i = 0; i < nodeCount; i++) {
		vertices.push_back(Vertex(i));
	}
	new_id = nodeCount;
}

Dendrogram::Dendrogram() {
	// Creates an empty dendrogram with no vertices [should be used for initializing only!]
#ifdef _DEBUG_HIGH
	cout << "Dendrogram constructor invoked" << endl;
#endif
	new_id = nodeCount;
}

Dendrogram::~Dendrogram() {
#ifdef _DEBUG_HIGH
	cout << "Dendrogram destructor invoked" << endl;
#endif
}

// Class Dendrogram | Public Member Function Definitions

vector<int> * Dendrogram::DFS() {
#ifdef _DEBUG_HIGH
	cout << "Dendrogram::DFS() invoked" << endl;
#endif
	// The returned vector contains the old label for the new label i at position i

	// 1 - keep a list of vertices with no parent (community roots)
	stack<list<Vertex>::iterator> communities;
	for (list<Vertex>::iterator iter = vertices.begin(); iter != vertices.end(); iter++) {
		if (!iter->hasParent) {
			communities.push(iter);
		}
	}

	// 2 - while the list is not empty, get one community root and perform DFS starting from it
	vector<int> * DFSorder = new vector<int>(nodeCount);
	int labelIncrement = 0;
	while (!communities.empty()) {
		list<Vertex>::iterator current_community = communities.top();
		communities.pop();

		// Iterative DFS implementation below
		stack<list<Vertex>::iterator> DFSstack;
		DFSstack.push(current_community);
		while (!DFSstack.empty()) {
			list<Vertex>::iterator current_top = DFSstack.top();
			current_top->visited = true;

			// If current vertex is a leaf, relabel the vertex
			if (!current_top->hasChildren) { // if edge1 is empty, edge2 must be empty as well
				(*DFSorder)[current_top->label] = labelIncrement++; // assign new label
				DFSstack.pop();
			}
			else if (!current_top->edge1->visited) {
				DFSstack.push(current_top->edge1);
			}
			else if (!current_top->edge2->visited) {
				DFSstack.push(current_top->edge2);
			}
			else { // vertex has two visited children
				DFSstack.pop();
			}
		}
	}
	return DFSorder;
}

void Dendrogram::connect(int u, int v) {
	// Precondition: <u> and <v> exist in the dendrogram && <u> and <v> are distinct vertices
	Vertex newVertex = Vertex(new_id++);

	list<Vertex>::iterator u_ptr, v_ptr;
	for (list<Vertex>::iterator it = vertices.begin(); it != vertices.end(); it++) {
		if (it->label == u) u_ptr = it;
		else if (it->label == v) v_ptr = it;
	}

	newVertex.edge1 = u_ptr;
	newVertex.edge2 = v_ptr;
	newVertex.hasChildren = true;
	u_ptr->hasParent = true;
	v_ptr->hasParent = true;
	vertices.push_back(newVertex);
}