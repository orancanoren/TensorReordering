#include <iostream>
#include <fstream>
#include <chrono>
#include <random>
#include <string>
#include <vector>
#include <algorithm>
#include <iterator>
#include <functional>
#include <climits>

using namespace std;

typedef unsigned int uint;

void help() {
	cout << "Usage: PURE NUM_EDGES MAX_VERTEX_LABEL  [OPTION...]" << endl
		<< "----------------------------------------------------" << endl
		<< "Available options:" << endl << endl
		<< "\t-zero_based \t\t vertices are labeled zero based" << endl
		<< "\t-weighted \t\t creates a weighted graph" << endl
		<< "\t-symmetric \t\t for the edge (u, v) the file doesn't contain (v, u)" << endl
		<< "\t-o=FILE_NAME \t\t name of the output file" << endl;
}

void generateGraph(ofstream & os, uint max_label, uint num_edges, bool zero_based, bool symmetric, bool values_exist) {
	default_random_engine engine;
	int lower_bound = zero_based ? 0 : 1;
	uniform_int_distribution<uint> dist(lower_bound, UINT_MAX);
	auto RNG = bind(dist, engine);
	
	chrono::high_resolution_clock::time_point begin = chrono::high_resolution_clock::now();

	for (int i = 0; i < num_edges; i++) {
		uint vertex1 = RNG() % max_label;
		uint vertex2 = RNG() % max_label;
		if (!zero_based) {
			vertex1++;
			vertex2++;
			if (vertex1 > max_label)
				vertex1 = 1;
			if (vertex2 > max_label)
				vertex2 = 1;
		}

		os << vertex1 << " " << vertex2;
		uint weight = RNG();
		if (values_exist) {
			os << " " << weight;

			if (!symmetric) {
				os << endl << vertex2 << " " << vertex1 << " " << weight;
			}
		}
		else if (!symmetric) {
			os << endl << vertex2 << " " << vertex1;
		}
		os << endl;
	}

	chrono::high_resolution_clock::time_point end = chrono::high_resolution_clock::now();

	cout << "Graph has been generated in " << chrono::duration_cast<chrono::milliseconds>(end - begin).count() << " ms" << endl;
}

int main(int argc, char * argv[]) {
	// 0 - Parse CLI arguments
	vector<string> arguments(argc);
	for (int i = 0; i < argc; i++) {
		arguments[i] = string(argv[i]);
	}
	
	bool values_exist = false, symmetric = false, zero_based = false;
	string filename = "random_graph.txt";

	if (find(begin(arguments), end(arguments), "--help") != end(arguments)) {
		help();
		exit(0);
	}
	else if (argc < 3) {
		cout << "Usage: PURE NUM_EDGES MAX_VERTEX_LABEL [OPTION...]" << endl
			<< "PURE --help for more info" << endl;
		exit(0);
	}

	if (find(begin(arguments), end(arguments), "-zero_based") != end(arguments)) {
		cout << "zero based graph generation" << endl;
		zero_based = true;
	}
	if (find(begin(arguments), end(arguments), "-symmetric") != end(arguments)) {
		cout << "symmetric graph generation" << endl;
		symmetric = true;
	}
	if (find(begin(arguments), end(arguments), "-weighted") != end(arguments)) {
		cout << "weighted graph generation" << endl;
		values_exist = true;
	}
	for (vector<string>::iterator it = begin(arguments); it != end(arguments); it++) {
		if (it->length() >= 3 && it->substr(0, 3) == "-o=") {
			if (it->length() == 3) {
				cout << "Invalid file name" << endl;
				exit(1);
			}
			filename = it->substr(3);
		}
	}
	
	const int num_edges = atoi(arguments[1].c_str());
	const int max_label = atoi(arguments[2].c_str());

	if (max_label > UINT_MAX) {
		cout << "Max label is too large!" << endl;
		exit(1);
	}

	// 1 - Generate the graph and output it
	ofstream os(filename);
	generateGraph(os, max_label, num_edges, zero_based, symmetric, values_exist);

	return 0;
}