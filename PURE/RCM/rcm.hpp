#ifndef _RCM_H
#define _RCM_H

#include <vector>
#include <queue>
#include <list>
#include <set>
#include <fstream>

class RCM {
public:
	// A graph is symmetric when it contains only the v1-v2 edge in undirected structure
	RCM(int nodeCount, bool valuesExist = false, bool symmetric = true);

	void insertEdge(int v1, int v2);
	void relabel();
	void printNewLabels(std::ofstream & os) const;

private:
	struct Vertex {
		Vertex() {
			visited = false;
		}

		bool visited;
		std::list<int> neighbors;

		bool operator < (const Vertex & rhs) const {
			return this->neighbors.size() < rhs.neighbors.size();
		}
	};
	
	std::vector<Vertex> vertices;
	std::set<int> unmarkedVertices;
	std::list<int> new_labels;

	bool valuesExist;
	bool symmetric;
};

#endif