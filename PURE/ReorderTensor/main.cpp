#include <iostream>
#include "convert.hpp"
#include <string>

using namespace std;

int main(int argc, char * argv[]) {
	cout << "************************************" << endl;
	string filename = "random_tensor.tns";
	Convert cobject(filename, true);
	cobject.write_graph();
	cout << "Graph has been written" << endl;
	cout << "************************************" << endl;
	return 0;
}