#include "convert.hpp"
#include <algorithm>
#include <fstream>
#include <string>
#include <iostream>
#include <chrono>
#include <iterator>
#include <unordered_map>
#include <cassert>

using namespace std;

Convert::Convert(const string & tensor_file, bool verbose, string output_file)
	: verbose(verbose), num_edges(0) {
	// Pre-condition: assumes the tensor file contains the tensor in 1-based COO
	chrono::high_resolution_clock::time_point begin, end;

	// 0 - Initialize the input stream
	if (output_file == "") {
		this->output_file = tensor_file + ".graph";
	}
	else {
		this->output_file = output_file;
	}

	ifstream is(tensor_file);
	if (!is.is_open()) {
		cout << "Cannot open the tensor file" << endl;
		exit(1);
	}

	if (verbose) {
		cout << "Start: obtain the tensor dimension & widths" << endl;
		begin = chrono::high_resolution_clock::now();
	}

	// 1 - Obtain the dimensions and widths
	// 1.1 - Get the dimension of the tensor (assumes last entry of the line is value)
	string first_line;
	getline(is, first_line);
	uint dimension = count(first_line.cbegin(), first_line.cend(), ' '); // dimension is equal to spaces in one line
	is.seekg(0); // reset the input stream to beginning

	// 1.2 - Obtain the maximum coordinates of each modes so that we can build an array of vertices
	super_diagonal.resize(dimension, 0);
	while (!is.eof()) {
		for (uint i = 0; i < dimension; i++) {
			if (is.eof())
				break;

			uint current_coordinate;
			is >> current_coordinate;
			super_diagonal[i] = max(super_diagonal[i], current_coordinate);
		}

		// read out the value
		double value;
		is >> value;
	}
	is.clear();
	is.seekg(0);

	if (verbose) {
		end = chrono::high_resolution_clock::now();
		cout << "End: obtain the tensor dimension & widths [" << chrono::duration_cast<chrono::milliseconds>(end - begin).count() << " ms]" << endl << endl;
	}

	cout << "Tensor Information ---------------" << endl
		<< "Dimension: ";
	for (vector<uint>::const_iterator dim = super_diagonal.cbegin(); dim != super_diagonal.cend(); dim++) {
		cout << *dim;
		if (next(dim, 1) != super_diagonal.cend()) {
			cout << "x";
		}
	}
	cout << endl;

	createGraph(is);
}

void Convert::createGraph(ifstream & is) {
	chrono::high_resolution_clock::time_point begin, end;

	if (verbose) {
		cout << endl
			<< "Start: initializing graph vertices" << endl;
		begin = chrono::high_resolution_clock::now();
	}

	// 1 - Use the diagonal info to construct the set of vertices with labels starting from 1
	uint num_vertices = 0;
	for (vector<uint>::const_iterator it = super_diagonal.cbegin(); it != super_diagonal.cend(); it++) {
		num_vertices += *it;
	}
	vertices.resize(num_vertices);

	if (verbose) {
		end = chrono::high_resolution_clock::now();
		cout << "End: initializing graph vertices [" << chrono::duration_cast<chrono::milliseconds>(end - begin).count() << " ms]" << endl
			<< "Begin: convertion to graph" << endl;
		begin = chrono::high_resolution_clock::now();
	}

	// 2 - Read the tensor file again, this time populating the vertices with their neighbors
	is.seekg(0);
	while (!is.eof()) {
		uint prev, cur;
		is >> prev;
		prev -= 1;
		if (is.eof()) break;

		for (uint i = 0; i < super_diagonal.size() - 1; i++) {
			if (is.eof()) break;

			is >> cur;
			cur = cur - 1 + super_diagonal[i];
			insertEdge(prev, cur);
			prev = cur;
		}

		// read out the value
		double value;
		is >> value;
	}

	if (verbose) {
		end = chrono::high_resolution_clock::now();
		cout << "End: convertion to graph [" << chrono::duration_cast<chrono::milliseconds>(end - begin).count() << " ms]" << endl;
	}
}

void Convert::write_graph() {
	chrono::high_resolution_clock::time_point begin, end;
	if (verbose) {
		begin = chrono::high_resolution_clock::now();
		cout << "Start: write graph" << endl;
	}

	// 0 - Initialize the output stream
	ofstream os(output_file);

	// First line of output: <num vertex> <num vertex> <num edges>
	os << vertices.size() << " " << vertices.size() << " " << num_edges << endl;

	// 1 - Output the graph in following format
	// <vertex 1> <vertex 2> <weight>
	uint label_counter = 0;
	for (vector<Vertex>::const_iterator it = vertices.begin(); it != vertices.cend(); it++, label_counter++) {
		for (unordered_map< uint, uint >::const_iterator neighbor = it->neighbors.cbegin(); neighbor != it->neighbors.cend(); neighbor++) {
			os << label_counter + 1 << " " << neighbor->first + 1<< " " << neighbor->second << endl;
		}
	}

	if (verbose) {
		end = chrono::high_resolution_clock::now();
		cout << "End: write graph [" << chrono::duration_cast<chrono::milliseconds>(end - begin).count() << " ms]" << endl;
	}
}

void Convert::insertEdge(uint u1, uint u2) {
	// Pre-condition: vertex <u1> and <u2> exists in <vertices>
	assert(u1 >= 0 && u1 < vertices.size() && u2 >= 0 && u2 < vertices.size());

	num_edges++;

	unordered_map< uint, uint > & u1_neighbors = vertices[u1].neighbors;
	unordered_map< uint, uint > & u2_neighbors = vertices[u2].neighbors;

	// 1 - Search for the edge
	unordered_map< uint, uint >::iterator edge = u1_neighbors.find(u2);
	
	// 1.1 - If the edge doesn't exists, create it and terminate
	if (edge == u1_neighbors.end()) {
		u1_neighbors.insert({ u2, 1 });
		vertices[u2].ancestors.push_back(u1);
		return;
	}

	// 2 - If the edge exists, update the weights of appropriate edges
	fixWeights(u1);
}

uint Convert::getMode(const uint vertex_id) const {
	uint mode_counter = 0;
	for (vector<uint>::const_iterator width = super_diagonal.cbegin(); width != super_diagonal.cend(); width++, mode_counter++) {
		if (vertex_id < *width)
			return mode_counter;
	}
}

void Convert::fixWeights(const uint source) {

	if (vertices[source].ancestors.empty())
		return;

	for (list<uint>::iterator ancestor = vertices[source].ancestors.begin(); ancestor != vertices[source].ancestors.end(); ancestor++) {
		unordered_map< uint, uint > & anc_neighbors = vertices[*ancestor].neighbors;
		unordered_map< uint, uint >::iterator edge = anc_neighbors.find(source);
		assert(edge != anc_neighbors.end());

		edge->second += 1;
		fixWeights(*ancestor);
	}
}