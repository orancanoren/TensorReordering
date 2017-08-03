#include <iostream>
#include "rcm.hpp"
#include <string>
#include <algorithm>
#include <iterator>
#include <vector>
using namespace std;

void banner() {
	cout
		<< "      ___           ___           ___     " << endl
		<< "     /\\  \\         /\\  \\         /\\__\\    " << endl
		<< "    /::\\  \\       /::\\  \\       /::|  |   " << endl
		<< "   /:/\\:\\  \\     /:/\\:\\  \\     /:|:|  |   " << endl
		<< "  /::\\~\\:\\  \\   /:/  \\:\\  \\   /:/|:|__|__ " << endl
		<< " /:/\\:\\ \\:\\__\\ /:/__/ \\:\\__\\ /:/ |::::\\__\\ " << endl
		<< " \\/_|::\\/:/  / \\:\\  \\  \\/__/ \\/__/~~/:/  /" << endl
		<< "    |:|::/  /   \\:\\  \\             /:/  / " << endl
		<< "    |:|\\/__/     \\:\\  \\           /:/  /  " << endl
		<< "    |:|  |        \\:\\__\\         /:/  /   " << endl
		<< "     \\|__|         \\/__/         \\/__/    " << endl
		<< "------Reverse Cuthill Mckee Permutation------" << endl;
}

void usage() {
	cout << "Usage: PURE GRAPH [OPTION...]" << endl
		<< "PURE --help for more info" << endl;
}

void help() {
	banner();
	usage();
	cout << "----------------------------------------------------" << endl
		<< "Available options:" << endl << endl
		<< "\t-zero_based \t\t vertices are labeled zero based" << endl
		<< "\t-weighted \t\t creates a weighted graph" << endl
		<< "\t-symmetric \t\t for the edge (u, v) the file doesn't contain (v, u)" << endl
		<< "\t-o=FILE_NAME \t\t name of the output file" << endl
		<< "\t-no_write \t\t does NOT write the new permutation" << endl
		<< "\t-degree_based \t\t computes degree based RCM" << endl;
}

int main(int argc, char * argv[]) {
	// 0 - Parse CLI arguments

	vector<string> arguments(argc);
	for (int i = 0; i < argc; i++) {
		arguments[i] = string(argv[i]);
	}

	bool values_exist = false, symmetric = false, zero_based = false, write = true, degree_based = false;;
	string input_filename, output_filename = "RCM_permutation.txt";

	if (find(begin(arguments), end(arguments), "--help") != end(arguments)) {
		help();
		exit(0);
	}
	else if (argc < 3) {
		usage();
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
	if (find(begin(arguments), end(arguments), "-no_write") != end(arguments)) {
		cout << "permutation will not be written" << endl;
		write = false;
	}
	if (find(begin(arguments), end(arguments), "-degree_based") != end(arguments)) {
		cout << "Computing degree based RCM" << endl;
		degree_based = true;
	}
	for (vector<string>::const_iterator it = cbegin(arguments); it != cend(arguments); it++) {
		if (it->length() >= 3 && it->substr(0, 3) == "-o=") {
			if (it->length() == 3) {
				cout << "Invalid file name" << endl;
				exit(1);
			}
			output_filename = it->substr(3);
		}
		else if (it->at(0) != '-') {
			input_filename = *it;
		}
	}

	if (input_filename == "") {
		cout << "A graph file must be provided" << endl;
		exit(1);
	}

	// 1 - RCM
	try {	
		banner();
		RCM graph(input_filename, values_exist, symmetric, !zero_based);
		graph.relabel(degree_based);
		if (write) {
			graph.printNewLabels(output_filename);
		}
	}
	catch (RCMexception & exc) {
		cout << "Error occured:" << endl
			<< exc.what() << endl;
	}
	return 0;
}