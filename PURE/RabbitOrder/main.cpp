#include "ordering.hpp"
#include <iostream>
#include <string>
#include <algorithm>

using namespace std;

void help() {
	cout << "Usage: PURE GRAPH [OPTION...]" << endl
		<< "----------------------------------------------------" << endl
		<< "Available options:" << endl << endl
		<< "\t-zero_based \t\t vertices are labeled zero based" << endl
		<< "\t-symmetric \t\t for the edge (u, v) the file doesn't contain (v, u)" << endl
		<< "\t-o=FILE_NAME \t\t name of the output file" << endl
		<< "\t-write_graph \t\t writes the re-ordered graph in MatrixMarket format" << endl;
}

void ordering(int argc, char * argv[]) {
	// Parse the CLI arguments
	vector<string> arguments(argc);
	for (int i = 0; i < argc; i++) {
		arguments[i] = string(argv[i]);
	}

	bool valuesExist = false, symmetric = false, oneBased = true, writeGraph = false;
	string input_filename, output_filename = "rabbit_permutation.txt";

	if (find(begin(arguments), end(arguments), "-symmetric") != end(arguments)) {
		symmetric = true;
		cout << "Symmetrical input processing" << endl;
	}

	if (find(begin(arguments), end(arguments), "-zero_based") != end(arguments)) {
		oneBased = false;
		cout << "Zero based input processing" << endl;
	}

	if (find(begin(arguments), end(arguments), "-write_graph") != end(arguments)) {
		writeGraph = true;
		cout << "Writing graph in the end" << endl;
	}

	if (find(begin(arguments), end(arguments), "--help") != end(arguments) || argc == 1) {
		help();
		exit(0);
	}

	for (vector<string>::iterator it = begin(arguments); it != end(arguments); it++) {
		if (it->length() >= 3 && it->substr(0, 3) == "-o=") {
			if (it->length() == 3) {
				cout << "Invalid file name" << endl;
				exit(1);
			}
			output_filename = it->substr(3);
		}
		if (it->at(0) != '-') {
			input_filename = *it;
		}
	}
	try {

		Ordering graph(input_filename, symmetric, !oneBased, writeGraph);
		graph.rabbitOrder(output_filename);
	}
	catch (GraphException & exc) {
		cout << "Error occured:" << endl
			<< exc.what() << endl;
	}
}
