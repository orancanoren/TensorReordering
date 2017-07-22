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
	Ordering(std::ifstream & is, bool symmetric = true, bool valuesExist = true, bool zeroBased = true); // Reads MatrixMarket graph
	~Ordering();

	void insertEdge(int from, int to, int value = 1);
	void rabbitOrder(std::ofstream & os);
private:
	struct Vertex {
		Vertex() : label(labelCounter++), merged(false) { }
		// keep a hashtable of edges
		std::unordered_map<int, int> edges;
		int label;
		static int labelCounter; // will be used to give out labels
		bool merged;

		bool operator < (const Vertex & rhs) const {
			return this->edges.size() < rhs.edges.size();
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

class InputFileErrorException : public GraphException {
public:
	InputFileErrorException(char * m = "Cannot read the input file")
		: GraphException() {
		msg = m;
	}
};

class InvalidInputException : public InputFileErrorException {
public:
	InvalidInputException() : InputFileErrorException("Error during input parse") {}
};


#endif