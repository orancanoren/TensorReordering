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

	cout << "Important: current version assumes the COO coordinates are 1-based!" << endl;

	// 1 - Obtain the number of vertices and dimension; create pairCoordinates
	// 1.1 - Create the input stream
	ifstream is(filename);
	if (!is.is_open())
		throw FileNotFoundException();

	// 1.2 - Obtain the dimension of the tensor and initialize pairCoordinates
	string first_line;
	getline(is, first_line);
	dimension = count(first_line.cbegin(), first_line.cend(), ' ');
	pairCoordinates = new Edge* [(dimension)*(dimension - 1) / 2];
	is.seekg(0); // reset the read pointer

	// 1.3 Fill up the pairCoordinates arrays
	while (!is.eof()) {
		string line;
		getline(cin, line);
		istringstream iss(line);

		uint * currentCoordinates = new uint[dimension];
		for (uint i = 0; i < dimension; i++) {
			iss >> currentCoordinates[i];
		}

		for (uint i = 0; i < dimension; i++) {
			const uint vertex1 = currentCoordinates[i];
			for (uint j = i + 1; j < dimension; j++) {
				pairCoordinates[i][j].vertex1 = vertex1;
				pairCoordinates[i][j].vertex2 = currentCoordinates[j];
				pairCoordinates[i][j].weight = 1;
			}
		}
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
	chrono::high_resolution_clock::time_point begin, end;
	if (verbose) {
		cout << "Begin: processing coordinates for all modes" << endl;
		begin = chrono::high_resolution_clock::now();
	}
	
	// 1 - Sort each pairCoordinate array
	const uint pairCount = dimension*(dimension - 1) / 2;
	for (uint i = 0; i < pairCount; i++) {
		sort(pairCoordinates[i], pairCoordinates[i] + num_vertices, compareEdge);
	}

	// 2 - In a single pass over the pairCoordinate arrays, do the following:
	// * By comparing adjacent elements, detect duplicates
	// * equal adjacent pairs have found, mark the one with smaller array index by
	// setting the edge weight 0
	// * Increase the weight of the edge between the vertices by the value of the edge weight
	// of the vertex pair got deleted in step 2
	// * continue until the end of the array
	for (uint currentArray = 0; currentArray < pairCount; currentArray++) {
		for (unsigned int j = 1; j < num_vertices; j++) {
			Edge & currentCoordinates = pairCoordinates[currentArray][j];
			Edge & previousCoordinates = pairCoordinates[currentArray][j - 1];
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

	ofstream os(output_file);
	if (!os.is_open()) {
		throw FileNotFoundException();
	}

	// Iterate all arrays and output in the format:
	// <vertex1> <vertex2> <weight>
	const uint pairCount = dimension*(dimension - 1) / 2;
	for (uint currentArray = 0; currentArray < pairCount; currentArray++) {
		for (int j = 0; j < num_vertices; j++) {
			const Edge & currentCoordinates = pairCoordinates[currentArray][j];
			cout << currentCoordinates.vertex1 << ' ' << currentCoordinates.vertex2 
				<< ' ' << currentCoordinates.weight << endl;
		}
	}

	if (verbose) {
		end = chrono::high_resolution_clock::now();
		cout << "Graph has been written [" << chrono::duration_cast<chrono::milliseconds>(end - begin).count() << " ms]" << endl;
	}
}
