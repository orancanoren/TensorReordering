#ifndef _ORDERING_H

#include <list>
#include <vector>
#include <exception>
#include <unordered_map>
#include <set>
#include "dendrogram.hpp"

typedef unsigned int uint;

struct Edge {
	uint toVertex;
	uint weight;

	bool operator=(const uint rhs) {
		return this->toVertex == rhs;
	}
};

class Ordering {
public:
	Ordering(std::string filename, bool symmetric = true,
		bool zeroBased = true, bool writeGraph = false); // Reads adjacency list graph with header info

	void insertEdge(uint from, uint to, uint value);
	void rabbitOrder(const std::string output_filename);
private:
	struct EdgeComparator {
		bool operator()(const Edge & lhs, const Edge & rhs) const {
			return lhs.toVertex == rhs.toVertex;
		}
		bool operator()(const Edge & lhs, const uint & rhs) const {
			return lhs.toVertex == rhs;
		}
	};

	struct Vertex {
		Vertex() : label(labelCounter++), merged(false) { }

		std::set < Edge, EdgeComparator > edges; // < neighbor label, edge weight >
		static uint labelCounter; // will be used to give out labels
		bool merged;
		uint label;

		bool operator < (const Vertex & rhs) const {
			return this->edges.size() < rhs.edges.size();
		}
	};

	// Member variables
	uint new_id;
	uint edgeCounter;
	bool symmetric;
	bool valuesExist;
	bool writeGraph;
	std::vector<Vertex> vertices;
	std::vector<uint> new_labels;
	Dendrogram dendrogram;

	// Sub-Algorithms
	void mergeVertices(uint u, uint v);
	const std::vector<uint> * ordering_generation();

	// Utilities
	double modularity(uint u, uint v);
	void community_detection();
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