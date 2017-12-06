#include "relabel.hpp"
#include <fstream>
#include <string>
#include <chrono>
#include <iostream>
#include <sstream>

using namespace std;

Relabel::Relabel(const string perm_file, bool verbose) : verbose(verbose) {
	chrono::high_resolution_clock::time_point begin = chrono::high_resolution_clock::now(), end;
	cout << "Start: reading permutation file" << endl;
	// 1 - Create the read stream
	ifstream perm_is(perm_file);
	if (!perm_is.is_open()) {
		cerr << "Cannot open the permutation file " << perm_file << endl;
	}
	if (verbose) {
		cout << endl << "Permutation file was successfuly opened\n";
	}

	// 2 - Read the permutation file first
	// 2.1 - read header info [dimension widths and # of labels]
	string line_buffer;
	getline(perm_is, line_buffer);
	if (line_buffer[0] != '%') {
		cerr << "permutation file is incompatible - header info missing" << endl;
	}
	istringstream iss(line_buffer);
	iss >> line_buffer;
	while (!iss.eof()) {
		string current_width;
		iss >> current_width;
		try {
			dimension_widths.push_back(stoi(current_width));
		}
		catch (...) {
			break;
		}
	}

	getline(perm_is, line_buffer);
	iss = istringstream(line_buffer);
	iss >> line_buffer;
	if (line_buffer != "%") {
		cerr << "permutation file is incompatible - header info missing" << endl;
	}
	uint num_vertices;
	iss >> num_vertices;
	permutation_labels.resize(num_vertices);
	if (verbose) {
		cout << "header file was successfully read from permutation file\n" << endl;
	}

	cout << "Tensor dimensions: ";
	for (int i = 0; i < dimension_widths.size(); i++) {
		cout << dimension_widths[i];
		if (i + 1 < dimension_widths.size()) {
			cout << "x";
		}
	}
	cout << endl;

	if (verbose) {
		cout << "reading new labels from permutation file" << endl << endl;
	}
	for (int i = 0; i < num_vertices; i++) {
		uint label_i;
		perm_is >> label_i;
		permutation_labels[i] = label_i;
	}
	end = chrono::high_resolution_clock::now();
	cout << "End: reading permutation file [" << chrono::duration_cast<chrono::milliseconds>(end - begin).count() << " ms]" << endl;
}

void Relabel::relabel_tensor(const string tensor_file, const string output_file) {
	ifstream tns_is(tensor_file);
	if (!tns_is.is_open()) {
		cerr << "Cannot open the tensor file " << tensor_file << endl;
	}
	if (verbose) {
		cout << endl << "Successfully opened tensor file" << endl;
	}
	ofstream os(output_file);

	// skip the header info of the tensor file if it exists
	streampos prev_position = tns_is.tellg();
	string line_buffer;
	uint num_header_lines = 0;
	getline(tns_is, line_buffer);
	while (line_buffer[0] == '%') {
		prev_position = tns_is.tellg();
		num_header_lines++;
		getline(tns_is, line_buffer);
	}
	tns_is.seekg(prev_position);
	if (verbose) {
		cout << "read " << num_header_lines << " line(s) of header info in tensor file\n\n";
	}

	chrono::high_resolution_clock::time_point begin = chrono::high_resolution_clock::now(), end;
	const uint dimension = dimension_widths.size();
	while (!tns_is.eof()) {
		uint current_coordinate, value;
		for (int i = 0; i < dimension; i++) {
			tns_is >> current_coordinate;
			os << getTensorCoordinate(current_coordinate) << ' ';
		}
		tns_is >> value;
		os << value << endl;
	}
	end = chrono::high_resolution_clock::now();
	cout << "End: create relabeled tensor file [" << chrono::duration_cast<chrono::milliseconds>(end - begin).count() << " ms]\n";
}

uint Relabel::getTensorCoordinate(const uint & label) const {
	if (label < dimension_widths[0]) {
		return label;
	}
	
	uint offset = dimension_widths[0];
	for (int i = 1; i < dimension_widths.size(); i++) {
		if (label < dimension_widths[i]) {
			return offset + label;
		}
		else {
			offset += dimension_widths[i];
		}
	}
}