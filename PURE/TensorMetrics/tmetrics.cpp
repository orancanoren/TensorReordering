#include "tmetrics.hpp"  
#include <string>
#include <fstream>
#include <list>
#include <vector>
#include <climits>
#include <algorithm>
#include <numeric> // std::inner_product()
#include <iostream>
#include <cassert>
#include <iterator>
#include <chrono>
using namespace std;

Tmetrics::Tmetrics(const string & in_file, bool no_values, bool verbose) 
	: no_values(no_values), current_mode(0), verbose(verbose) {
	ifstream is(in_file);
	if (!is.is_open()) {
		cout << "Cannot open the provided tensor file" << endl;
		exit(1);
	}
	// 0 - Determine the dimension of the tensor
	string first_line;
	getline(is, first_line);

	int dimension = 0, index = 0;
	while (index != string::npos) {
		index = first_line.find(' ', index + 1);
		dimension++;
	}

	if (!no_values)
		dimension--;
	
	// 1 - Read the input stream
	diagonal.resize(dimension, INT_MIN);
	is.seekg(0);
	while (!is.eof()) {
		vector<int> current_coordinates(dimension);
		for (int i = 0; i < dimension; i++) {
			int component;
			is >> component;
			current_coordinates[i] = component;
			diagonal[i] = max(diagonal[i], component);
		}
		if (!no_values) {
			float value;
			is >> value;
		}
		coords.push_back(Coordinate(current_coordinates));
	}
	cout << "Dimensions: ";
	for (vector<int>::const_iterator it = diagonal.cbegin(); it != diagonal.cend(); it++) {
		cout << *it;
		if (next(it, 1) != diagonal.end()) {
			cout << "x";
		}
	}
	cout << endl << "--------------------------------" << endl;
}

// CLASS Tmetrics | Public Member Function Definitions

void Tmetrics::all_metrics_all_modes() {
	cout << "Outputing the files" << endl
		<< "File format:" << endl
		<< "<slice 1 distance metric> <slice 1 normalized pair metric> <slice 1 pair metric>" << endl
		<< "..." << endl
		<< "<slice n distance metric> <slice n normalized pair metric> <slice n pair metric>" << endl
		<< "--------------------------------------------------------" << endl;
	chrono::high_resolution_clock::time_point begin = chrono::high_resolution_clock::now(), end;
	const int dimension = diagonal.size();
	for (int mode1 = 0; mode1 < dimension - 1; mode1++) {
		for (int mode2 = mode1 + 1; mode2 < dimension; mode2++) {
			if (verbose) {
				cout << "Computing modes " << mode1 << " " << mode2 << endl;
			}
			metrics_on_modes(mode1, mode2);
		}
	}
	end = chrono::high_resolution_clock::now();
	cout << "Metric computation complete [ total - " << chrono::duration_cast<chrono::milliseconds>(end - begin).count() << " ms]" << endl;
}

// CLASS Tmetrics | Private Member Function Definitions

void Tmetrics::metrics_on_modes(uint mode1, uint mode2) {
	assert(mode1 < diagonal.size() && mode2 < diagonal.size());

	cout << "Modes " << mode1 << " - " << mode2 << endl
		<< "-------------------------" << endl;
	chrono::high_resolution_clock::time_point begin, end;
	// 0 - Sort the coordinate matrix
	Comparator comp(mode1, mode2);
	if (verbose) {
		cout << "Sorting the coordinates" << endl;
		begin = chrono::high_resolution_clock::now();
	}
	coords.sort(comp);
	if (verbose) {
		cout << "Coordinates sorted [" << chrono::duration_cast<chrono::milliseconds>(end - begin).count() << " ms]" << endl
			<< "Setting slice indexes" << endl;
	}

	// 1 - Determine the slices
	list<int> slice_indexes; // <first_slice_end> -> <second_slice_end> -> ... -> <final_slice_end>
	vector<int> current_coordinates = coords.begin()->coor;
	int index = 1;
	for (list<Coordinate>::const_iterator it = next(coords.cbegin(), 1); it != coords.cend(); it++, index++) {
		for (uint i = 0; i < it->coor.size(); i++) {
			if ((i != mode1 && i != mode2) && (current_coordinates[i] != it->coor[i])) {
				current_coordinates = it->coor;
				slice_indexes.push_back(index);
			}
		}
	}

	// 2 - For each slice, compute the metrics and output them
	ofstream os("mode_" + to_string(mode1) + "_" + to_string(mode2) + ".metric");

	int previous = 0;
	if (verbose) {
		cout << "Computing the metrics" << endl;
		begin = chrono::high_resolution_clock::now();
	}
	begin = chrono::high_resolution_clock::now();
	for (list<int>::const_iterator index = slice_indexes.cbegin(); index != slice_indexes.cend(); index++) {
		double * metrics = all_metrics(previous, *index, mode1, mode2);
		os << metrics[0] << " " << metrics[1] << " " << metrics[2] << " " << endl;
		if (verbose) {
			cout << metrics[0] << " " << metrics[1] << " " << metrics[2] << " " << endl;
		}
		previous = *index;
	}
	os.close();
	end = chrono::high_resolution_clock::now();
	if (verbose) {
		cout << "Mode " << mode1 << " " << mode2 << " metrics are computed [" << chrono::duration_cast<chrono::milliseconds>(end - begin).count() << " ms]" << endl;
	}
}

double * Tmetrics::all_metrics(uint low, uint high, uint mode1, uint mode2) const {
	double max_distance = INT_MIN;
	double max_value_normalized = INT_MIN;
	double max_value = INT_MIN;

	for (list<Coordinate>::const_iterator it = next(coords.cbegin(), low); it != next(coords.begin(), high); it++) {
		max_distance = max(max_distance, distance_to_diagonal(*it, mode1, mode2)); // distance metric
																				   // compute the pairwise differences and acquire the max
		for (unsigned int i = 0; i < diagonal.size() - 1; i++) { // pairwise difference metrics
			for (unsigned int j = i + 1; j < diagonal.size(); j++) {
				max_value_normalized = max(abs((static_cast<double>(it->coor[i]) / diagonal[i]) - (static_cast<double>(it->coor[j]) / diagonal[j])), max_value_normalized); // normalized  difference metric
				max_value = max(static_cast<double>(abs((it->coor[i] - it->coor[j]))), max_value); // original difference metric
			}
		}
	}

	double * metrics = new double[3];
	metrics[0] = max_distance;
	metrics[1] = max_value_normalized;
	metrics[2] = max_value;
	return metrics;
}

double Tmetrics::distance_to_diagonal(const Coordinate & coord, uint mode1, uint mode2) const {
	// A = (0, 0) B = (mode1, mode2)
	const pair<uint, uint> P = { coord.coor[mode1], coord.coor[mode2] };
	const pair<uint, uint> diagonal = { mode1, mode2 };
	// PA vector is equivalent to P
	
	double t = (P.first * diagonal.first + P.second + diagonal.second)
		/ (diagonal.first * diagonal.first + diagonal.second + diagonal.second);

	double distance = P.first - (t * diagonal.first) + P.second - (t * diagonal.second);
	return sqrt(distance);
}

double Tmetrics::max_distance_to_diagonal(uint mode1, uint mode2) const {
	double max_distance = INT_MIN;
	for (list<Coordinate>::const_iterator it = coords.begin(); it != coords.end(); it++) {
		max_distance = max(max_distance, distance_to_diagonal(*it, mode1, mode2));
	}
	return max_distance;
}

double Tmetrics::metric_1(bool normalize) const {
	// Pre-condition: Assumes that the tensor dimension is greater than 1!
	double max_value = INT_MIN;
	for (list<Coordinate>::const_iterator it = coords.begin(); it != coords.end(); it++) {
		double current_value;
		// compute the pairwise differences and acquire the max
		for (unsigned int i = 0; i < diagonal.size() - 1; i++) {
			for (unsigned int j = i + 1; j < diagonal.size(); j++) {
				double difference;
				if (normalize)
					difference = abs((static_cast<double>(it->coor[i]) / diagonal[i]) - (static_cast<double>(it->coor[j]) / diagonal[j]));
				else
					difference = static_cast<double>(abs((it->coor[i] - it->coor[j])));
				current_value = max(current_value, difference);
			}
		}
		max_value = max(max_value, current_value);
	}
	return max_value;
}

bool Tmetrics::Comparator::operator() (const Coordinate & lhs, const Coordinate & rhs) const {
	assert(mode1 != mode2 && lhs.coor.size() == rhs.coor.size());
	// operator() implements the `strict less` opearation
	const uint dimension = lhs.coor.size();
	for (uint i = 0; i < dimension; i++) {
		// Treat coordinate values as if mode1 & mode2 were at the end
		if ((i != mode1 && i != mode2)) {
			if (lhs.coor[i] < rhs.coor[i])
				return true;
			else if (lhs.coor[i] > rhs.coor[i])
				return false;
		}
	}
	if (mode1 > mode2 && lhs.coor[mode1] < rhs.coor[mode1]) {
		return true;
	}
	else if (mode2 > mode1 && lhs.coor[mode2] < rhs.coor[mode2]) {
		return true;
	}
	return false;
}