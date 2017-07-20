#include <iostream>
#include "rcm.hpp"
#include <fstream>

using namespace std;

int main() {
	RCM graph(8, false, true);
	graph.insertEdge(0, 2);
	graph.insertEdge(0, 7);
	graph.insertEdge(1, 3);
	graph.insertEdge(1, 6);
	graph.insertEdge(3, 1);
	graph.insertEdge(3, 6);
	graph.insertEdge(7, 0);
	graph.insertEdge(7, 5);
	graph.insertEdge(6, 7);

	graph.relabel();

	ofstream os("permutation.txt");

	graph.printNewLabels(os);
	return 0;
}