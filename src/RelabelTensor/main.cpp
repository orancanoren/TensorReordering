#include <iostream>
#include "relabel.hpp"
#include <vector>
using namespace std;

namespace relabel
{
void usage() {
	cout << "PURE -t TENSOR -p PERMUTATION -[OPTIONS...]" << endl;
}

void help() {
	cout << "Tensor relabeling tool" << endl
		<< "----------------------" << endl
		<< "Relabels the COO formatted tensor file by the provided permutation file" << endl;
	usage();
	cout << "Available options" << endl
		<< "\t-o FILENAME\t\t sets the name of the output file" << endl
		<< "\t-v \t\t verbose mode" << endl;
}

int relabelMain(int argc, char * argv[]) {
	cout << "************************************" << endl;
	// 1 - parse the command line options
	// 1.1 - feed the arguments into a std::vector
	vector<string> arguments(argc);
	for (int i = 0; i < argc; i++) {
		arguments[i] = string(argv[i]);
	}

	// 1.2 - perform search for help option and check the validity of arguments
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

	bool verbose = false;
	string tensor_file;
	string permutation_file;
	string output_file = "relabeled_tensor.tns";

	for (int i = 1; i < argc; i++) {
		if (arguments[i] == "-t") { // tensor file input
			if (i + 1 >= argc || arguments[i + 1][0] == '-') {
				cerr << "expected tensor file, didn't find one!" << endl;
				exit(1);
			}
			i++;
			tensor_file = arguments[i];
		}
		else if (arguments[i] == "-p") { // permutation file input
			if (i + 1 >= argc || arguments[i + 1][0] == '-') {
				cerr << "expected permutation file, didn't find one!" << endl;
				exit(1);
			}
			i++;
			permutation_file = arguments[i];
		}
		else if (arguments[i] == "-v") { // verbose mode activated
			verbose = true;
		}
		else if (arguments[i] == "-o") { // output file name
			if (i + 1 >= argc || arguments[i + 1][0] == '-') {
				cerr << "expected filename for output, didn't find one!" << endl;
				exit(1);
			}
			i++;
			output_file = arguments[i];
		}
		else { // unknown argument!
			cerr << "Unknown argument encountered: " << arguments[i] << endl;
			exit(1);
		}
	}

	if (tensor_file == "") {
		cerr << "A tensor file must be provided!" << endl;
		exit(1);
	}
	if (permutation_file == "") {
		cerr << "A permutation file must be provided!" << endl;
		exit(1);
	}

	Relabel relabel_obj(permutation_file, verbose);
	relabel_obj.relabel_tensor(tensor_file, output_file);

	return 0;
}
}
