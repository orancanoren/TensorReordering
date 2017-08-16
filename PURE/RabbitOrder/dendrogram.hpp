#ifndef _DENDROGRAM_H
#define _DENDROGRAM_H

#include <vector>
#include <list>
#include <set>

typedef unsigned int uint;

class Dendrogram {
public:
	Dendrogram();
	Dendrogram(uint nodeCount);

	void connect(uint u, uint v);
	std::vector<uint> * DFS();
	// Returns DFS order for each community in a vector,
	// arr[i] contains the new label for i'th vertex
private:
	struct Vertex {
		Vertex(uint label)
			: label(label), hasParent(false), visited(false), hasChildren(false) { }
		int label;
		// In a dendrogram, a vertex is allowed to
		// be connected to at most 2 vertices
		std::list<Vertex>::iterator edge1;
		std::list<Vertex>::iterator edge2;
		bool hasParent;
		bool hasChildren; // iterators aren't nullable, this is needed
		bool visited;

		bool operator == (const Vertex & rhs) {
			return this->label == rhs.label;
		}
	};

	std::list<Vertex> vertices;
	uint nodeCount;
	uint new_id;
};

#endif