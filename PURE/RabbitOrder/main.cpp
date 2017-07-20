#include "ordering.hpp"
#include <iostream>
#include <string>
#include <fstream>

using namespace std;

int main(int argc, char * argv[]) {
	string filename;
	bool is_symmetric = true, values_exist = true, zero_based = true;
	if (argc > 1) {
		filename = argv[1];
	}
	else {
		cout << "Please eneter the filename as a CL argument" << endl;
		exit(1);
	}
	if (argc > 2 && string(argv[2]) == "not_symmetric") {
		is_symmetric = false;
		cout << "non-symmetric graph input" << endl;
	}
	if (argc > 3 && string(argv[3]) == "no_values") {
		values_exist = false;
		cout << "unweighted graph input" << endl;
	}
	if (argc > 4 && string(argv[4]) == "one_based") {
		zero_based = false;
		cout << "one-based graph input" << endl;
	}

	ifstream is(filename);
	if (!is.is_open()) {
		cout << "Unable to open file " << filename << endl
			<< "Terminating" << endl;
		exit(1);
	}
	try {
		Ordering graph(is, is_symmetric, values_exist, zero_based);
		ofstream os("permutation.txt");
		graph.rabbitOrder(os);
	}
	catch (GraphException & exc) {
		cout << "Error occured:" << endl;
		cout << exc.what() << endl;
		cout << "Terminating" << endl;
	}
	return 0;
}
