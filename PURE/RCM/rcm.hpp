#ifndef _RCM_H
#define _RCM_H

#include <vector>
#include <queue>
#include <list>
#include <set>
#include <fstream>
#include <exception>
#include <utility>

class RCM {
public:
	// A graph is symmetric when it contains only the v1-v2 edge in undirected structure
	RCM(int nodeCount, bool valuesExist = false, bool symmetric = true, bool oneBased = true);
	RCM(std::string & iname, bool valuesExist = false, bool symmetric = true, bool oneBased = true);

	void insertEdge(int v1, int v2, int weight);
	void relabel(bool degree_based = false);
	void printNewLabels(std::string & oname) const;

private:
	struct Vertex {
		Vertex() {
			visited = false;
		}

		bool visited;
		std::list< std::pair<int, int>> neighbors; // < neighor, weight >

	};
	
	bool _vertexCompare_degree(const Vertex & lhs, const Vertex & rhs) const;
	bool _vertexCompare_weight(const Vertex & lhs, const Vertex & rhs) const;

	std::vector<Vertex> vertices;
	std::set<int> unmarkedVertices;
	std::list<int> new_labels;

	bool valuesExist;
	bool symmetric;
	bool oneBased;
};

// =======================
// EXCEPTION CLASSES BELOW
// =======================

class RCMexception : public std::exception {
public:
	RCMexception(const char * msg) : msg(msg) { }

	const char * what() const noexcept {
		return msg;
	}
private:
	const char * msg;
};

class InputFileErrorException : public RCMexception {
public:
	InputFileErrorException(const char * msg = "Cannot read the input file") 
		: RCMexception(msg) { }
};

class VertexNotFound : public InputFileErrorException {
public:
	VertexNotFound(int id) : InputFileErrorException(std::string("Invalid vertex encountered: " + id).c_str()) {}
};


#endif