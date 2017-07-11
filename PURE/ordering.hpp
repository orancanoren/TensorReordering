#ifndef _ORDERING_H

#include <list>
#include <vector>
#include <exception>
#include <unordered_map>
#include "dendrogram.hpp"

class Ordering {
public:
	Ordering(int vertexCount);
	~Ordering();

	void insertEdge(int from, int to);
	void rabbitOrder();
protected:
	struct Edge {
		Edge(int dest) : weight(1), dest(dest) { }
		double weight;
		int dest; // destination
	};
	struct Vertex {
		Vertex() : label(labelCounter++), merged(false) { }
		// keep a hashtable of edges
		std::unordered_map<int, int> edges; // undirected edges
		// degree of a vertex is kept in the unordered_map above by its size

		int label;

		bool operator < (const Vertex & rhs) {
			return this->edges.size() < rhs.edges.size();
		}

		static int labelCounter; // will be used to give out labels
		bool merged;
	};

	void mergeVertices(int u, int v);
	void community_detection();

	const std::vector<int> ordering_generation();
	double modularity(int u, int v);

	std::vector<Vertex> vertices;
	Dendrogram dendrogram;
};

// =======================
// EXCEPTION CLASSES BELOW
// =======================

enum ExceptionType {
	UNKNOWN_EXCEPTION = 0, // shouldn't be thrown
	NOT_FOUND
};

class GraphException : public std::exception {
public:
	GraphException() : excType(UNKNOWN_EXCEPTION), msg("UNKNOWN EXCEPTION") { }

	ExceptionType getExceptionType() const noexcept {
		return excType;
	}

	const char * what() {
		return msg;
	}
protected:
	ExceptionType excType;
	const char * msg;
};

enum Reason {
	VERTEX_NOT_FOUND = 0,
	EDGE_NOT_FOUND
};

class NotFound : public GraphException {
public:
	NotFound(Reason reason) {
		excType = NOT_FOUND;
		if (reason == VERTEX_NOT_FOUND) msg = "VERTEX NOT FOUND";
		else msg = "EDGE NOT FOUND";
	}
};


#endif