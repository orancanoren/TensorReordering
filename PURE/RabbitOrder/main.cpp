#include "ordering.hpp"
#include <iostream>
#include <string>
#include <fstream>

using namespace std;

int main(int argc, char * argv[]) {
	// Parse the CLI arguments
	vector<string> arguments(argc);
	for (int i = 0; i < argc; i++) {
		arguments[i] = string(argv[i]);
	}

	bool valuesExist = false, symmetric = true, oneBased = true;

	if (find(begin(arguments), end(arguments), "-values_exist") != end(arguments)) {
		valuesExist = true;
		cout << "Values exist in the data" << endl;
	}

	if (find(begin(arguments), end(arguments), "-not_symmetric") != end(arguments)) {
		symmetric = false;
		cout << "Asymmetrical input processing" << endl;
	}

	if (find(begin(arguments), end(arguments), "-zero_based") != end(arguments)) {
		oneBased = false;
		cout << "Zero based input processing" << endl;
	}

	if (find(begin(arguments), end(arguments), "-help") != end(arguments) || argc == 1) {
		cout << "PURE <file name> -[options]" << endl
			<< "----------------------" << endl
			<< "1. values_exist -> indicates that the input file conatins edge weights" << endl
			<< "2. not_symmetric -> indicates that for <u>-[e]-><v>, <v>-[e]-><u> doesn't reside in the file" << endl
			<< "3. zero_based -> indicates that the coordinates in the file are zero based" << endl;
	}
	else {
		try {
			string filename;
			for (int i = 0; i < argc; i++) {
				if (arguments[i][0] != '-') {
					filename = arguments[i];
				}
			}
			if (filename == "") {
				cout << "Name of the input file should be passed as [-<file name>]" << endl
					<< "Type -help for more" << endl;
				exit(1);
			}

			ofstream os("rabbit_permutation.txt");
			ifstream is(filename);

			Ordering graph(is, symmetric, valuesExist, !oneBased);
			graph.rabbitOrder(os);
		}
		catch (GraphException & exc) {
			cout << "Error occured:" << endl
				<< exc.what() << endl;
		}
	}

	return 0;
}
