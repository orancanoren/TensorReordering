#include <iostream>
#include <vector>
#include <fstream>
#include <chrono>
#include <random>
#include <functional> // std::bind()
#include <algorithm>
#include <climits>

using namespace std;
namespace randomtensor
{
  
typedef unsigned int uint;

const int MAX_VALUE = 200000;

void usage() {
	cout << "Usage: PURE -dim=NUM_DIMENSIONS DIM_1_WIDTH DIM_2_WIDTH .. DIM_N_WIDTH -nnz=NNZ_COUNT [OPTION...]" << endl;
}

void help() {
	cout << "Random N-Dimensional Tensor Generator" << endl;
	usage();
	cout << "----------------------------------------------------" << endl
		<< "Available options:" << endl << endl
		<< "\t-one_based \t\t coordinates are labeled one based" << endl
		<< "\t-no_values \t\t creates a tensor without values" << endl
		<< "\t-o=FILE_NAME \t\t name of the output file" << endl;
}

void generateTensor(const string & filename, bool zero_based, bool values_exist, int nnz, const vector<int> & dimensions) {
	ofstream os(filename);
	// output header info - dimension widths
	os << "% ";
	for (vector<int>::const_iterator it = dimensions.cbegin(); it != dimensions.cend(); it++) {
		os << *it << " ";
	}
	os << endl;

	default_random_engine engine;
	int lower_bound = zero_based ? 0 : 1;
	uniform_int_distribution<uint> dist(lower_bound, UINT_MAX);
	auto RNG = bind(dist, engine);

	for (int i = 0; i < nnz; i++) {
		for (int j = 0; j < dimensions.size(); j++) {
			int RN = 0;
			if (!zero_based) {
				while (RN == 0) {
					RN = RNG() % (dimensions[j] + 1);
				}
			}
			else {
				RN = dimensions[j];
				while (RN == dimensions[j]) {
					RN = RNG() % dimensions[j];
				}
			}
			os << RN;
			if (j != dimensions.size() - 1) {
				os << " ";
			}
		}
		if (values_exist) {
			os << " " << RNG() % MAX_VALUE;
		}
		os << endl;
	}
}

int randTensorMain(int argc, char * argv[]) {
	// 0 - Parse CLI arguments
	vector<string> arguments(argc);
	int nnz = -1, dimension = -1;
	for (int i = 0; i < argc; i++) {
		arguments[i] = string(argv[i]);
	}

	bool values_exist = true, zero_based = true;
	string output_filename = "random_tensor.tns";

	if (find(begin(arguments), end(arguments), "--help") != end(arguments)) {
		help();
		exit(0);
	}
	else if (argc < 3) {
		usage();
		cout << "PURE --help for more info" << endl;
		exit(0);
	}

	if (find(begin(arguments), end(arguments), "-one_based") != end(arguments)) {
		cout << "zero based tensor generation" << endl;
		zero_based = false;
	}
	if (find(begin(arguments), end(arguments), "-no_values") != end(arguments)) {
		cout << "Coordinate only tensor generation" << endl;
		values_exist = false;
	}
	for (vector<string>::iterator it = begin(arguments); it != end(arguments); it++) {
		if (it->length() >= 3 && it->substr(0, 3) == "-o=") {
			if (it->length() == 3) {
				cout << "Invalid file name" << endl;
				exit(1);
			}
			output_filename = it->substr(3);
		}
		else if (it->length() >= 5 && it->substr(0, 5) == "-nnz=") {
			if (it->length() == 5) {
				cout << "Invalid non zero count" << endl;
				exit(1);
			}
			nnz = atoi(it->substr(5).c_str());
		}
		else if (it->length() >= 5 && it->substr(0, 5) == "-dim=") {
			if (it->length() == 5) {
				cout << "Invalid dimension" << endl;
				exit(1);
			}
			dimension = atoi(it->substr(5).c_str());
		}
	}

	if (dimension == -1) {
		cout << "Dimension must be provided" << endl
			<< "PURE --help for more info" << endl;
		exit(1);
	}
	if (nnz == -1) {
		cout << "non zero count must be provided" << endl
			<< "PURE --help for more info" << endl;
		exit(1);
	}

	vector<int> dimension_widths(dimension);
	int dim = 0;
	for (vector<string>::const_iterator it = arguments.begin() + 1; it != arguments.end(); it++) {
		if (it->at(0) == '-')
			continue;

		dimension_widths[dim++] = atoi(it->c_str());
	}

	// 1 - Create randomized tensor with specified inputs
	generateTensor(output_filename, zero_based, values_exist, nnz, dimension_widths);

	return 0;
}
}
