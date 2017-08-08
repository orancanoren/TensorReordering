#include <iostream>
#include "tmetrics.hpp"
#include <chrono>
#include <string>
#include <algorithm>

using namespace std;

void usage() {
	cout << "Usage: PURE TENSOR -[OPTIONS...]" << endl;
}

void help() {
	cout << "Tensor Ordering Metrics" << endl
		<< "-----------------------" << endl;
	usage();
	cout << "Avaiable options:" << endl
		<< "\t-no_values \t\t tensor file does NOT contain values" << endl
		<< "\t-v \t\t verbose, i.e. prints each metric to STDOUT" << endl;
}

int main(int argc, char * argv[]) {
	// 0 - Parse CLI arguments
	vector<string> arguments(argc);
	int nnz = -1, dimension = -1;
	for (int i = 0; i < argc; i++) {
		arguments[i] = string(argv[i]);
	}

	bool values_exist = true, zero_based = false, verbose = false;;

	if (find(begin(arguments), end(arguments), "--help") != end(arguments)) {
		help();
		exit(0);
	}
	else if (argc < 2) {
		usage();
		exit(0);
	}

	if (find(begin(arguments), end(arguments), "-no_values") != end(arguments)) {
		cout << "COO contains coordinates only" << endl;
		values_exist = false;
	}
	if (find(begin(arguments), end(arguments), "-v") != end(arguments)) {
		cout << "Verbose mode" << endl;
		values_exist = false;
	}

	string file;
	for (vector<string>::const_iterator it = arguments.cbegin() + 1; it != arguments.cend() && file == ""; it++) {
		if (it->at(0) != '-') {
			file = *it;
		}
	}
	if (file == "") {
		cout << "A tensor file must be provided" << endl
			<< "PURE --help for more info" << endl;
		exit(1);
	}

	// 1 - Compute the metrics
	
	Tmetrics metric_calculator(file, !values_exist);
	
	
	cout << "**********************" << endl;
	chrono::high_resolution_clock::time_point begin = chrono::high_resolution_clock::now(), end;
	metric_calculator.all_metrics_all_modes();
	end = chrono::high_resolution_clock::now();
	cout << "**********************" << endl
		<< "Computation time: " << chrono::duration_cast<chrono::milliseconds>(end - begin).count() << " ms" << endl;

	return 0;
}