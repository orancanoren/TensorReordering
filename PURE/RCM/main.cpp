#include <iostream>
#include "rcm.hpp"
#include <fstream>

using namespace std;

int main() {
	ofstream os("permutation.txt");
	ifstream is("sample_graph.txt");

	RCM graph(is);

	graph.relabel();



	graph.printNewLabels(os);
	return 0;
}