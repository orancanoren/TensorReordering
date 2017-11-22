#include "convert.hpp"
#include <string>
#include <chrono>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <sstream>

using namespace std;

Convert::Convert(const string filename, bool verbose) : verbose(verbose) {
	/* The file format is assumed to be:
	<dim 1 coordinate> <dim 2 coordinate> ... <dim n coordinate> <value>
	<dim 1 coordinate> <dim 2 coordinate> ... <dim n coordinate> <value>
	...
	*/

	chrono::high_resolution_clock::time_point begin, end;
	if (verbose) {
		cout << "Begin: read the tensor file" << endl;
		begin = chrono::high_resolution_clock::now();
	}

	// 1 - Obtain the number of vertices and dimension; create pairCoordinates
	// 1.1 - Create the input stream
	ifstream is(filename);
	if (!is.is_open())
		throw FileNotFoundException();

	// 1.2 - Obtain the dimension of the tensor and initialize pairCoordinates
	string first_line;
	getline(is, first_line);
	dimension = count(first_line.cbegin(), first_line.cend(), ' ');
	cout << "dimension: " << dimension << endl;
	const uint modePairs = (dimension)*(dimension - 1) / 2;
	pairCoordinates.resize(modePairs);
	for (uint i = 0; i < modePairs; i++) {
		pairCoordinates[i].resize(1000000); // Number of nonzeros [POSSIBLE BUG]
	}
	is.seekg(0); // reset the read pointer

	num_vertices = 0;

	// 1.3 Fill up the pairCoordinates arrays
	while (!is.eof()) {
		string line;
		getline(is, line);
		istringstream iss(line);

		uint * currentCoordinates = new uint[dimension];
		for (uint i = 0; i < dimension; i++) {
			iss >> currentCoordinates[i];
		}
		
		if (line == "") {
		  break;
		}
		
		string value;
		iss >> value;

		int modePair = 0;
		for (uint mode1 = 0; mode1 < dimension - 1; mode1++) {
			for (uint mode2 = mode1 + 1; mode2 < dimension; mode2++, modePair++) {
				pairCoordinates[modePair][num_vertices].vertex1 = currentCoordinates[mode1];
				pairCoordinates[modePair][num_vertices].vertex2 = currentCoordinates[mode2];
				pairCoordinates[modePair][num_vertices].weight = 1;
			}
		}

		num_vertices++;
	}

	for (int i = 0; i < modePairs; i++) {
		pairCoordinates[i].resize(num_vertices);
	}

	if (verbose) {
		end = chrono::high_resolution_clock::now();
		cout << "End: read the tensor file [" << chrono::duration_cast<chrono::milliseconds>(end - begin).count() << " ms]" << endl;
	}

	processCoordinates();
}

bool Convert::compareEdge(const Edge & lhs, const Edge & rhs) {
	// Compare by vertex1 first and vertex2 afterwards
	if (lhs.vertex1 < rhs.vertex1) {
		return true;
	}
	else if (lhs.vertex1 == rhs.vertex1 && lhs.vertex2 < rhs.vertex2) {
		return true;
	}
	return false;
}

void Convert::processCoordinates() {
	// Post-condition: vertexPairs have been generated successfully
	chrono::high_resolution_clock::time_point begin, end, tempBegin;
	if (verbose) {
		cout << "Begin: processing coordinates for all modes" << endl;
		begin = chrono::high_resolution_clock::now();
	}
	
	// 1 - Sort each pairCoordinate array
	tempBegin = chrono::high_resolution_clock::now();
	cout << "Sorting the arrays" << endl;
	const uint pairCount = dimension*(dimension - 1) / 2;
	for (uint i = 0; i < pairCount; i++) {
		sort(pairCoordinates[i].begin(), pairCoordinates[i].end(), compareEdge);
	}
	end = chrono::high_resolution_clock::now();
	cout << "Sorting done ["
		<< chrono::duration_cast<chrono::milliseconds>(end - tempBegin).count() << " ms]" << endl;

	// 2 - In a single pass over the pairCoordinate arrays, do the following:
	// * By comparing adjacent elements, detect duplicates
	// * equal adjacent pairs have found, mark the one with smaller array index by
	// setting the edge weight 0
	// * Increase the weight of the edge between the vertices by the value of the edge weight
	// of the vertex pair got deleted in step 2
	// * continue until the end of the array
	for (uint currentArray = 0; currentArray < pairCount; currentArray++) {
		for (uint index = 1; index < pairCoordinates[currentArray].size(); index++) {
			Edge & currentCoordinates = pairCoordinates[currentArray][index];
			Edge & previousCoordinates = pairCoordinates[currentArray][index - 1];
			if (previousCoordinates == currentCoordinates) {
				// increase the weight of j'th pair's edge by (j-1)'th pair's edge
				currentCoordinates.weight += previousCoordinates.weight;
				previousCoordinates.weight = 0;
			}
		}
	}

	if (verbose) {
		end = chrono::high_resolution_clock::now();
		cout << "End: processing coordinates for all modes [" << chrono::duration_cast<chrono::milliseconds>(end - begin).count() << " ms]" << endl;
	}
}

void Convert::write_graph(const string & output_file) const {
	chrono::high_resolution_clock::time_point begin, end;
	if (verbose) {
		begin = chrono::high_resolution_clock::now();
		cout << "Starting writing the graph" << endl;
	}

	//ofstream os(output_file);

	// Iterate all arrays and output in the format:
	// <vertex1> <vertex2> <weight>
	int mode1 = 0, mode2 = 1;
	const uint pairCount = dimension*(dimension - 1) / 2;
	for (uint currentArray = 0; currentArray < pairCount; currentArray++) {
		ofstream os("mode_" + to_string(mode1) + "_" + to_string(mode2) + ".graph");
		if (!os.is_open()) {
			cout << "Cannot create output stream for modes " << mode1 << " and " << mode2 << endl;
		}
		for (const Edge & currentCoordinates: pairCoordinates[currentArray]) {
			if (currentCoordinates.weight != 0) {
				os << currentCoordinates.vertex1 << ' ' << currentCoordinates.vertex2
					<< ' ' << currentCoordinates.weight << endl;
				if (currentCoordinates.weight > 1) {
					cout << currentCoordinates.vertex1 << ' ' << currentCoordinates.vertex2
						<< ' ' << currentCoordinates.weight << endl;
				}
			}
		}
		if (mode2 + 1 == dimension) {
			mode1++;
			mode2 = mode1 + 1;
		}
		else {
			mode2++;
		}
	}

	if (verbose) {
		end = chrono::high_resolution_clock::now();
		cout << "Graph has been written [" << chrono::duration_cast<chrono::milliseconds>(end - begin).count() << " ms]" << endl;
	}
}
