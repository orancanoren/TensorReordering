#include "ordering.hpp"
#include <iostream>
#include <string>
#include <fstream>

using namespace std;

int main(int argc, char * argv[]) {
	string filename;
	if (argc > 1) {
		filename = argv[1];
	}
	else {
		cout << "Please enter the filename as a CL argument" << endl;
		exit(1);
	}

	ifstream is(filename);
	if (!is.is_open()) {
		cout << "Unable to open file " << filename << endl
			<< "Terminating" << endl;
		exit(1);
	}
	try {
		Ordering graph(is);
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
