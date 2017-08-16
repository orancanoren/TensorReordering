#ifndef _CONVERT_HPP
#define _CONVERT_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <exception>
#include <list>
#include <iostream>

typedef unsigned int uint;

struct Edge {
	Edge(uint v1, uint v2, uint weight) : vertex1(v1), vertex2(v2), weight(weight) { }

	uint vertex1;
	uint vertex2;
	uint weight;
};

class Convert {
public:
	Convert(const std::string filename, bool verbose = false);

	void write_graph() const;
private:

	// Member variables
	std::vector< std::list< Edge > > modes;
	std::vector< uint > super_diagonal;
	std::list< std::vector< uint > > coordinates;
	bool verbose;

	// Mutators
	void processCoordinates();

	// Utilities
	std::list< Edge >::iterator find(const std::pair< uint, uint > & edge_to_find, const uint mode);
};

// EXCEPTION CLASSES BELOW

class ConvertException : public std::exception {
public:
	ConvertException(const char * msg) : msg(msg) { }
	void what() {
		std::cout << msg;
	}
private:
	const char * msg;
};

class FileNotFoundException : public ConvertException {
public:
	FileNotFoundException() : ConvertException("Tensor file not found!") {}
};

#endif