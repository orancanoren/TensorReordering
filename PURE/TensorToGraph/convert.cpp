#include "convert.hpp"
#include <string>
#include <chrono>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <sstream>

using namespace std;

Convert::Convert(const string filename, uint dimension, uint nnz, uint * mode_widths, bool verbose)
	: verbose(verbose), mode_widths(mode_widths), nnz(nnz), dimension(dimension) {
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
	if (!is.is_open()) {
		throw FileNotFoundException();
	}

	// skip comments in file [comments are marked with "%" in beginning of line]
	string line;
	fpos_t prev_position = is.tellg();
	getline(is, line);
	while (line.at(0) == '%' && !is.eof()) {
		prev_position = is.tellg();
		getline(is, line);
	}
	is.seekg(prev_position);

	// 1.3 Fill up the pairCoordinates arrays
	const uint modePairs = (dimension)*(dimension - 1) / 2;
	pairCoordinates = new Edge*[modePairs];
	for (int i = 0; i < modePairs; i++) {
		pairCoordinates[i] = new Edge[nnz];
	}

	uint * currentCoordinates = new uint[dimension];
	uint coordinate_counter = 0;
	while (!is.eof()) {
		getline(is, line);
		istringstream iss(line);

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
				pairCoordinates[modePair][coordinate_counter].vertex1 = currentCoordinates[mode1];
				pairCoordinates[modePair][coordinate_counter].vertex2 = currentCoordinates[mode2];
				pairCoordinates[modePair][coordinate_counter].weight = 1;
			}
		}
		coordinate_counter++;
	}
	delete[] currentCoordinates;

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
		sort(pairCoordinates[i], pairCoordinates[i] + nnz, compareEdge);
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
	num_output_edges = nnz*pairCount;
	for (uint currentArray = 0; currentArray < pairCount; currentArray++) {
		for (uint index = 1; index < nnz; index++) {
			Edge & currentCoordinates = pairCoordinates[currentArray][index];
			Edge & previousCoordinates = pairCoordinates[currentArray][index - 1];
			if (previousCoordinates == currentCoordinates) {
				// increase the weight of j'th pair's edge by (j-1)'th pair's edge
				currentCoordinates.weight += previousCoordinates.weight;
				previousCoordinates.weight = 0;
				num_output_edges--;
			}
		}
	}
	cout << "The graph has " << num_output_edges << " edges" << endl;

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

	// Iterate all arrays and output in the format:
	// <vertex1> <vertex2> <weight>
	const uint pairCount = dimension*(dimension - 1) / 2;
	ofstream os(output_file);
	if (!os.is_open()) {
		cerr << "Cannot create output stream for graph" << endl;
		exit(1);
	}
	// output the header info - widths of dimensions
	os << "% ";
	for (int i = 0; i < dimension; i++) {
		os << mode_widths[i] << " ";
	}
	os << endl << "% " << num_output_edges << endl;

	// output each coordinate with the defined offset
	int mode1 = 0, mode2 = 1;
	uint offset1 = 0, offset2 = 0;
	for (uint currentArray = 0; currentArray < pairCount; currentArray++) {
		// 1 - set offsets for modes
		for (int i = 0; i < mode1; i++) {
			offset1 += mode_widths[i];
		}
		for (int i = 0; i < mode2; i++) {
			offset2 += mode_widths[i];
		}

		// 2 - output vertex labels taking into account the offset
		for (int i = 0; i < nnz; i++) {
			const Edge & currentCoordinates = pairCoordinates[currentArray][i];
			if (currentCoordinates.weight != 0) {
				os << currentCoordinates.vertex1 + offset1 << ' ' << currentCoordinates.vertex2 + offset2
					<< ' ' << currentCoordinates.weight << endl;
			}
		}
		if (mode2 + 1 == dimension) {
			mode1++;
			mode2 = mode1 + 1;
		}
		else {
			mode2++;
		}
		offset1 = 0;
		offset2 = 0;
	}
	os.close();
	if (verbose) {
		end = chrono::high_resolution_clock::now();
		cout << "Graph has been written [" << chrono::duration_cast<chrono::milliseconds>(end - begin).count() << " ms]" << endl;
	}
}
