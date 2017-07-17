#include "ordering.hpp"
#include <iostream>
#include <string>
#include <fstream>

using namespace std;

int main() {
	/*
	Ordering graph(8);
	graph.insertEdge(0, 2, 1.4);
	graph.insertEdge(0, 4, 5.1);
	graph.insertEdge(0, 7, 2.6);
	graph.insertEdge(5, 7, 0.7);
	graph.insertEdge(4, 6, 1.3);
	graph.insertEdge(4, 3, 0.5);
	graph.insertEdge(3, 1, 8.4);
	graph.insertEdge(6, 1, 4.2);
	graph.insertEdge(2, 4, 8.0);
	graph.insertEdge(2, 7, 9.2);
	graph.insertEdge(4, 7, 7.9);
	graph.insertEdge(6, 3, 3.1);
	*/
	const string filename = "sample_graph.txt";
	ifstream is(filename);
	if (!is.is_open()) {
		cout << "Unable to open file " << filename << endl
			<< "Terminating" << endl;
		exit(1);
	}
	Ordering graph(is);

	ofstream os("output.txt");
	graph.rabbitOrder(os);

	return 0;
}
