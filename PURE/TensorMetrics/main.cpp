#include <iostream>
#include "tmetrics.hpp"
#include <chrono>
#include <string>

using namespace std;

void usage() {
	cout << "Usage: PURE TENSOR -[OPTIONS...]" << endl
		<< "PURE --help for more info" << endl;
}

void help() {
	cout << "Tensor Ordering Metrics" << endl
		<< "-----------------------" << endl;
	usage();
	cout << "Avaiable options:" << endl
		<< "\t-no_values \t\t tensor file does NOT contain values" << endl
		<< "\t-distance \t\t computes only the distance metric" << endl;
}

int main(int argc, char * argv[]) {
	// 0 - Parse CLI arguments
	vector<string> arguments(argc);
	int nnz = -1, dimension = -1;
	for (int i = 0; i < argc; i++) {
		arguments[i] = string(argv[i]);
	}

	bool values_exist = true, zero_based = false, distance_only = false;

	if (find(begin(arguments), end(arguments), "--help") != end(arguments)) {
		help();
		exit(0);
	}
	else if (argc < 2) {
		usage();
		exit(0);
	}

	if (find(begin(arguments), end(arguments), "-no_values") != end(arguments)) {
		cout << "Coordinate only tensor generation" << endl;
		values_exist = false;
	}
	if (find(begin(arguments), end(arguments), "-distance") != end(arguments)) {
		cout << "Computing only the distance metric" << endl;
		distance_only = true;
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
	if (distance_only) {
		cout << metric_calculator.max_distance_to_diagonal();
		end = chrono::high_resolution_clock::now();
	}
	else {
		double * metrics = metric_calculator.all_metrics();
		cout << "Max distance to diagonal: " << metrics[0] << endl
			<< "Max Normalized Pairwise Difference: " << metrics[1] << endl
			<< "Max Pairwise Difference: " << metrics[2] << endl;
		end = chrono::high_resolution_clock::now();
	}
	cout << "**********************" << endl
		<< "Computation time: " << chrono::duration_cast<chrono::milliseconds>(end - begin).count() << " ms" << endl;

	return 0;
}