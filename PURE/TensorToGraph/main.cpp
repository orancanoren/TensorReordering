#include <iostream>
#include "convert.hpp"
#include <string>
#include <algorithm>

using namespace std;

void usage() {
	cout << "Usage: PURE TENSOR -[OPTIONS...]" << endl;
}

void help() {
	cout << "" << endl
		<< "-----------------------" << endl;
	usage();
	cout << "Avaiable options:" << endl
		<< "\t-o=FILE\t\t sets the name of the output file" << endl
		<< "\t-v \t\t verbosee" << endl;
}

int main(int argc, char * argv[]) {
	cout << "************************************" << endl;

	// 0 - Parse CLI arguments
	vector<string> arguments(argc);
	int nnz = -1, dimension = -1;
	for (int i = 0; i < argc; i++) {
		arguments[i] = string(argv[i]);
	}

	bool verbose = false;;

	if (find(begin(arguments), end(arguments), "--help") != end(arguments)) {
		help();
		exit(0);
		cout << "****************************************" << endl;
	}
	else if (argc < 2) {
		usage();
		exit(0);
		cout << "****************************************" << endl;
	}

	if (find(begin(arguments), end(arguments), "-v") != end(arguments)) {
		cout << "Verbose mode" << endl;
		verbose = true;
	}

	string infile, outfile;
	for (vector<string>::const_iterator it = arguments.cbegin() + 1; it != arguments.cend() && infile == ""; it++) {
		if (it->at(0) != '-') {
			infile = *it;
		}
		else if (it->substr(0, 3) == "-o=") {
			outfile = it->substr(3);
		}
	}
	if (infile == "") {
		cout << "A tensor file must be provided" << endl
			<< "PURE --help for more info" << endl;
		exit(1);
		cout << "****************************************" << endl;
	}

	Convert conv_obj(infile, verbose, outfile);
	conv_obj.write_graph();

	cout << "************************************" << endl;
	return 0;
}