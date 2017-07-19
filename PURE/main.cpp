#include "ordering.hpp"
#include <iostream>
#include <string>
#include <fstream>

using namespace std;

int main() {
	const string filename = "sample_graph.txt";
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
