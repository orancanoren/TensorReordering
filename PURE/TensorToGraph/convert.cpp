#include "convert.hpp"
#include <vector>
#include <list>
#include <string>
#include <chrono>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <unordered_map>
#include <sstream>

using namespace std;

Convert::Convert(const string filename, bool verbose) : verbose(verbose) {
	chrono::high_resolution_clock::time_point begin, end;
	if (verbose) {
		cout << "Begin: read the tensor file" << endl;
		begin = chrono::high_resolution_clock::now();
	}

	// 1 - Read the file, get the widths of the dimensions as well
	// 1.1 - Create the input stream
	ifstream is(filename);
	if (!is.is_open())
		throw FileNotFoundException();

	// 1.2 - Obtain the dimension of the tensor
	string first_line;
	getline(is, first_line);
	uint dimension = count(first_line.cbegin(), first_line.cend(), ' ');

	// 1.3 Fill up the coordinate list and obtain the width of each dimension as well
	super_diagonal.resize(dimension, 0);
	is.seekg(0);
	while (!is.eof()) {
		vector< uint > current_coordinates(dimension);
		for (uint i = 0; i < dimension; i++) {
			uint component;
			is >> component;

			if (is.eof())
				break;

			current_coordinates[i] = component;
			super_diagonal[i] = max(super_diagonal[i], component);
			coordinates.push_back(current_coordinates);
		}

		double value;
		is >> value;
	}

	if (verbose) {
		end = chrono::high_resolution_clock::now();
		cout << "End: read the tensor file [" << chrono::duration_cast<chrono::milliseconds>(end - begin).count() << " ms]" << endl;
	}

	processCoordinates();
}

void Convert::processCoordinates() {
	chrono::high_resolution_clock::time_point begin, end;
	if (verbose) {
		cout << "Begin: processing coordinates for all modes" << endl;
		begin = chrono::high_resolution_clock::now();
	}
	// For each mode, insert the pairs into the appropriate hashtable
	for (list< vector< uint > >::const_iterator it = coordinates.cbegin(); it != coordinates.cend(); it++) {
		const vector< uint > & current_coordinates = *it;

		for (uint mode = 0; mode < super_diagonal.size(); mode++) {
			// For one mode, enumerate all permutations of size 2 and insert them to the htable of appropriate mode
			for (uint i = 0; i < current_coordinates.size() - 1; i++) {
				if (i == mode) continue;

				uint label_1 = current_coordinates[mode] + (mode == 0 ? 0 : super_diagonal[mode - 1]);
				uint label_2 = current_coordinates[i] + (i == 0 ? 0 : super_diagonal[i - 1]);

				list< Edge >::iterator find_result = find({ label_1, label_2 }, mode);
				if (find_result == modes[mode].end()) {
					modes[mode].push_back(Edge(label_1, label_2, 1));
				}
				else {
					find_result->weight += 1;
				}
			}
		}
	}

	if (verbose) {
		end = chrono::high_resolution_clock::now();
		cout << "End: processing coordinates for all modes [" << chrono::duration_cast<chrono::milliseconds>(end - begin).count() << " ms]" << endl;
	}
}

void Convert::write_graph() const {
	uint vertex_count = 0;
	for (vector< uint >::const_iterator it = super_diagonal.cbegin(); it != super_diagonal.cend(); it++) {
		vertex_count++;
	}

	for (uint i = 0; i < modes.size(); i++) {
		ofstream os("mode_" + to_string(i) + ".graph");

		// print the header info <vertex count> <vertex count> <edge count>
		os << vertex_count << " " << vertex_count << " " << modes[i].size() << endl;

		for (auto it = modes[i].cbegin(); it != modes[i].cend(); it++) {
			os << it->first.first << " " << it->first.second << " " << it->second << endl;
		}
	}
}

list< Edge >::iterator Convert::find(const pair< uint, uint > & vertex_pair, const uint mode) {
	for (list< Edge >::iterator it = modes[mode].begin(); it != modes[mode].end(); it++) {
		if (it->vertex1 == vertex_pair) {
			return it;
		}
	}
	return modes[mode].end();
}