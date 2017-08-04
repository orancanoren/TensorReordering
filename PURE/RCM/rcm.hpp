#ifndef _RCM_H
#define _RCM_H

#include <vector>
#include <queue>
#include <list>
#include <set>
#include <fstream>
#include <exception>
#include <utility>

typedef std::pair<int, float> EDGE; // < neighor, weight >
typedef std::list< EDGE > EDGE_LIST;

class RCM {
public:
	// A graph is symmetric when it contains only the v1-v2 edge in undirected structure
	explicit RCM(std::string & iname, bool valuesExist = false, bool symmetric = true, bool oneBased = true, bool degree_based = true);

	void insertEdge(int v1, int v2, float weight);
	void relabel();
	void printNewLabels(std::string & oname) const;

private:
	struct Vertex {
		Vertex() {
			visited = false;
		}

		bool visited;
		EDGE_LIST neighbors;
	};

	struct Comparator {
		Comparator(const RCM & rcm_obj) : rcm_obj(rcm_obj) { }
		bool operator() (const EDGE & lhs, const EDGE & rhs); // comparator as a functor

		const RCM & rcm_obj;
	};

	std::vector<Vertex> vertices;
	std::set<int> unmarkedVertices;
	std::list<int> new_labels;

	bool valuesExist;
	bool symmetric;
	bool oneBased;
	bool degree_based;
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