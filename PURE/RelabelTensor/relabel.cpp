#include "relabel.hpp"
#include <fstream>
#include <string>
#include <chrono>
#include <iostream>

using namespace std;

Relabel::Relabel(string tensor_file, string perm_file, bool verbose, bool no_values) {
	chrono::high_resolution_clock::time_point begin, end;
	if (verbose) {
		cout << "Start: reading permutation file" << endl;
	}

	// 1 - Create the read stream
	ifstream tns_is(tensor_file), perm_is(perm_file);

	// 1.1 - Obtain vertex and edge counts
	uint vertex_count, edge_count;
	tns_is >> vertex_count >> vertex_count >> edge_count;

	// 1.2 - Read the file and create the permutat
	
}