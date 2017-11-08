#ifndef _CONVERT_HPP
#define _CONVERT_HPP

#include <string>
#include <exception>
#include <iostream>
#include <vector>

typedef unsigned int uint;

struct Edge {
	Edge() { weight = 1; }
	Edge(uint v1, uint v2, uint weight) : vertex1(v1), vertex2(v2), weight(weight) { }
	uint vertex1;
	uint vertex2;
	uint weight;

	bool operator==(const Edge & rhs) const {
		if (this->vertex1 == rhs.vertex1 && this->vertex2 == rhs.vertex2) {
			return true;
		}
		return false;
	}
};

class Convert {
public:
	Convert(const std::string filename, bool verbose = false);

	void write_graph(const std::string & output_file) const;
private:
	// Member variables
	std::vector<std::vector<Edge>> pairCoordinates;
	bool verbose;
	uint dimension;
	uint num_vertices;

	// Mutators
	void processCoordinates();
	static bool compareEdge(const Edge & lhs, const Edge & rhs);
};

// =====================
// EXCEPTION CLASS BELOW
// =====================

class ConvertException : public std::exception {
public:
	ConvertException(const char * msg) : msg(msg) { }
	void what() {
		std::cout << msg << std::endl;
	}
private:
	const char * msg;
};

class FileNotFoundException : public ConvertException {
public:
	FileNotFoundException() : ConvertException("Tensor file not found!") {}
};

#endif