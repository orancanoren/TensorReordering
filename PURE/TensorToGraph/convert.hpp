#ifndef _REORDER_HPP
#define _REORDER_HPP

#include <vector>
#include <string>
#include <exception>
#include <list>
#include <unordered_map>

typedef unsigned int uint;

class Convert {
public:
	Convert(const std::string & tensor_file, bool verbose = false, std::string output_file = "");
	
	void write_graph();
private:
	struct Vertex {
		std::list<uint> ancestors; // neighbors from lower modes
		std::unordered_map< uint, uint > neighbors; // < neighbor_id, weight >
	};

	// Member variables
	std::vector<Vertex> vertices;
	std::vector<uint> super_diagonal;
	bool verbose;
	std::string output_file;
	uint num_edges;

	// Graph Mutators
	void createGraph(std::ifstream & is);
	void insertEdge(uint u1, uint u2); // inserts a directed edge from <u1> to <u2>

	// Utilities
	uint getMode(const uint vertex_id) const;
	void fixWeights(const uint sourceVertex);
};

#endif