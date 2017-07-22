#include <iostream>
#include "rcm.hpp"
#include <fstream>
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
		<< "     \\|__|         \\/__/         \\/__/    " << endl;

}

int main(int argc, char * argv[]) {
	// Parse command line arguments
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

	if (find(begin(arguments), end(arguments), "-help") != end(arguments)) {
		cout << "PURE <file name> -[options]" << endl
			<< "----------------------" << endl
			<< "1. values_exist -> indicates that the input file conatins edge weights" << endl
			<< "2. not_symmetric -> indicates that for <u>-[e]-><v>, <v>-[e]-><u> resides in the file" << endl
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

			ofstream os("permutation.txt");
			ifstream is(filename);
			
			banner();
			RCM graph(is, valuesExist, symmetric, oneBased);
			graph.relabel();
			graph.printNewLabels(os);
		}
		catch (RCMexception & exc) {
			cout << "Error occured:" << endl
				<< exc.what() << endl;
		}
	}
	return 0;
}