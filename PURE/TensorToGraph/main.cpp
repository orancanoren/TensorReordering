#include <iostream>
#include "convert.hpp"
#include <string>
#include <algorithm>
#include <vector>
#include <sstream>

using namespace std;

void usage() {
	cout << "Usage: PURE TENSOR -nnz NNZ -n DIMENSION WIDTH1 WIDTH2... -[OPTIONS...]" << endl;
}

void help() {
	cout << "" << endl
		<< "-----------------------" << endl;
	usage();
	cout << "Avaiable options:" << endl
		<< "\t-o FILE\t\t sets the name of the output file" << endl
		<< "\t-v \t\t verbose mode" << endl;
}

int main(int argc, char * argv[]) {
	cout << "************************************" << endl;

	// 0 - Parse CLI arguments
	vector<string> arguments(argc);
	for (int i = 0; i < argc; i++) {
		arguments[i] = string(argv[i]);
	}

	bool verbose = false;

	if (find(begin(arguments), end(arguments), "--help") != end(arguments)) {
		help();
		exit(0);
		cout << "****************************************" << endl;
	}
	else if (argc < 4) {
		usage();
		exit(0);
		cout << "****************************************" << endl;
	}

	string infile = argv[1];
	string outfile = "converted_graph.txt";
	uint dimension = 0;
	uint * mode_widths;
	uint nnz = 0;
	uint num_widths_read = 0;
	bool dimensions_provided = false;
	for (int i = 2; i < argc; i++) {
		const string arg_i = argv[i];
		if (arg_i == "-v") {
			verbose = true;
		}
		else if (arg_i == "-o") {
			if (i + 1 < argc && argv[i + 1][0] != '-') {
				outfile = argv[i + 1];
				i++;
			}
			else {
				cerr << "An output file must be provided with -o option!" << endl;
				exit(1);
			}
		}
		else if (arg_i == "-n") {
			if (i + 1 < argc && argv[i + 1][0] != '-') {
				i++;
				istringstream iss(argv[i]);
				iss >> dimension;
				mode_widths = new uint[dimension];
				uint mode = 0;
				i++;
				for (; i < argc; i++) {
					num_widths_read++;
					iss = istringstream(argv[i]);
					iss >> mode_widths[mode];					
					mode++;
				}
				dimensions_provided = true;
			}
			else {
				cerr << "Dimension and widths must be provided" << endl;
				exit(1);
			}
		}
		else if (arg_i == "-nnz") {
			if (i + 1 < argc) {
			  istringstream iss(argv[i + 1]);
			  iss >> nnz;
			}
			else {
				cerr << "Number of nonzeros must be proivded" << endl;
				exit(1);
			}
			++i;
		}
		else {
			cerr << "Unknown command line option " << arg_i << endl;
			exit(1);
		}
	}

	if (num_widths_read != dimension) {
		cerr << "widths for all dimensions must be provided" << endl;
		exit(1);
	}
	if (infile[0] == '-' || infile == "") {
		cerr << "Tensor file name must be provided as the first argument" << endl;
		exit(1);
	}

	try {
		Convert conv_obj(infile, dimension, nnz, mode_widths, verbose);
		conv_obj.write_graph(outfile);
	}
	catch (ConvertException & exc) {
		exc.what();
	}

	cout << "************************************" << endl;
	return 0;
}
