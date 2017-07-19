#include "ordering.hpp"
#include <iostream>
#include <string>
#include <fstream>

using namespace std;

int main(int argc, char * argv[]) {
	string filename;
	bool is_symmetric = true, values_exist = true;
	if (argc > 1) {
		filename = argv[1];
	}
	else {
		cout << "Please eneter the filename as a CL argument" << endl;
		exit(1);
	}
	if (argc > 2 && argv[2] == "not_symmetric") {
		is_symmetric = false;
	}
	if (argc > 3 && argv[3] == "no_values") {
		values_exist = false;
	}

	ifstream is(filename);
	if (!is.is_open()) {
		cout << "Unable to open file " << filename << endl
			<< "Terminating" << endl;
		exit(1);
	}
	try {
		Ordering graph(is, is_symmetric, values_exist);
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
