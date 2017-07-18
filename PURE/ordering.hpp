#ifndef _ORDERING_H

#include <list>
#include <vector>
#include <exception>
#include <unordered_map>
#include <fstream>
#include "dendrogram.hpp"

class Ordering {
public:
	Ordering(int vertexCount, bool symmetric = true);
	Ordering(std::ifstream & is, bool symmetric = true, bool valuesExist = true); // Reads Matlab graph format
	~Ordering();

	void insertEdge(int from, int to, int value = 0);
	void rabbitOrder(std::ofstream & os, std::ofstream & matlab_stream, std::ofstream & label_stream); // returns the relabled graph in CRS format
private:
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

		bool operator < (const Vertex & rhs) const {
			return this->edges.size() < rhs.edges.size();
		}

		static int labelCounter; // will be used to give out labels
		bool merged;
	};
	struct VertexPtrComparator { // Utility struct for vertex comparison used in Ordering::rabbitOrder()
		bool operator() (const std::vector<Vertex>::const_iterator lhs, const std::vector<Vertex>::const_iterator rhs) const {
			return lhs->label > rhs->label;
		}
	};

	void mergeVertices(int u, int v);
	void community_detection();

	const std::vector<int> ordering_generation();
	double modularity(int u, int v);
	int new_id;
	unsigned int edgeCounter;
	bool symmetric;
	bool edgeInserted;
	bool valuesExist;

	void processOutput(const std::vector<int> & data, std::ofstream & os);

	std::vector<Vertex> vertices;
	std::vector<int> new_labels;
	Dendrogram dendrogram;
};

// =======================
// EXCEPTION CLASSES BELOW
// =======================

enum ExceptionType {
	UNKNOWN_EXCEPTION = 0, // shouldn't be thrown
	NOT_FOUND,
	INPUT_ERROR
};

class GraphException : public std::exception {
public:
	GraphException() : excType(UNKNOWN_EXCEPTION), msg("UNKNOWN EXCEPTION") { }

	ExceptionType getExceptionType() const noexcept {
		return excType;
	}

	const char * what() const noexcept {
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

class NotFoundException : public GraphException {
public:
	NotFoundException(Reason reason) {
		excType = NOT_FOUND;
		if (reason == VERTEX_NOT_FOUND) msg = "VERTEX NOT FOUND";
		else msg = "EDGE NOT FOUND";
	}
};

class InputErrorException : public GraphException {
public:
	InputErrorException() {
		excType = INPUT_ERROR;
		msg = "Failure during input processing";
	}
};


#endif