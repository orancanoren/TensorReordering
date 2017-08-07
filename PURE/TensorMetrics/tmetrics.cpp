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
using namespace std;

Tmetrics::Tmetrics(const string & in_file, bool no_values) 
	: no_values(no_values), current_mode(0) {
	ifstream is(in_file);
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
			int value;
			is >> value;
		}
		coords.push_back(Coordinate(current_coordinates));
	}
	cout << "Tensor information --------------" << endl
		<< "Dimensions: ";
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
	const int dimension = diagonal.size();
	for (int mode1 = 0; mode1 < dimension - 1; mode1++) {
		for (int mode2 = mode1 + 1; mode2 < dimension; mode2++) {
			metrics_on_modes(mode1, mode2);
		}
	}
}

// CLASS Tmetrics | Private Member Function Definitions

double Tmetrics::distance_to_diagonal(const Coordinate & coord) const {
	// A = (0, 0, ..., 0) B = (n1, n2, ..., n_n)
	const vector<int> & PA = coord.coor;
	double t = inner_product(PA.begin(), PA.end(), diagonal.begin(), 0)
		/ inner_product(diagonal.begin(), diagonal.end(), diagonal.begin(), 0);

	double distance = 0;
	for (unsigned int i = 0; i < diagonal.size(); i++) {
		distance += (PA[i] - (t * diagonal[i])) * (PA[i] - (t * diagonal[i]));
	}
	return sqrt(distance);
}

bool Tmetrics::Comparator::operator() (const Coordinate & lhs, const Coordinate & rhs) const{
	assert(mode1 != mode2);
	// operator() implements the `strict less` opearation
	const int dimension = lhs.coor.size();
	for (int i = 0; i < dimension; i++) {
		// Treat coordinate values as if mode1 & mode2 were at the end
		if ((i != mode1 && i != mode2) && (lhs.coor[i] < rhs.coor[i])) {
			return true;
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

void Tmetrics::metrics_on_modes(int mode1, int mode2) {
	assert(mode1 < diagonal.size() && mode2 < diagonal.size());

	// 0 - Sort the coordinate matrix
	Comparator comp(*this, mode1, mode2);
	coords.sort(comp);

	// 1 - Determine the slices
	list<int> slice_indexes; // <first_slice_end> -> <second_slice_end> -> ... -> <final_slice_end>
	vector<int> current_coordinates = coords.begin()->coor;
	int index = 0;
	for (list<Coordinate>::const_iterator it = coords.cbegin(); it != coords.cend(); it++, index++) {
		if (it->coor != current_coordinates) {
			current_coordinates = it->coor;
			slice_indexes.push_back(index);
		}
	}

	// 2 - For each slice, compute the metrics and output them
	ofstream os("mode_" + to_string(mode1) + "_" + to_string(mode2) + ".metric");
	os << "Format: <max distance metric - slice1> <normalized max value metric - slice 1> <max value metric - slice 1> <max distance metric - slice2> ..." << endl;
	int previous = 0;
	for (list<int>::const_iterator index = slice_indexes.cbegin(); index != slice_indexes.cend(); index++) {
		double * metrics = all_metrics(previous, *index);
		os << metrics[0] << " " << metrics[1] << " " << metrics[2] << " ";
		previous = *index;
	}
}

double * Tmetrics::all_metrics(int low, int high) const {
	double max_distance = INT_MIN;
	double max_value_normalized = INT_MIN;
	double max_value = INT_MIN;

	for (list<Coordinate>::const_iterator it = next(coords.cbegin(), low); it != next(coords.begin(), high); it++) {
		max_distance = max(max_distance, distance_to_diagonal(*it)); // distance metric
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

double Tmetrics::max_distance_to_diagonal() const {
	double max_distance = INT_MIN;
	for (list<Coordinate>::const_iterator it = coords.begin(); it != coords.end(); it++) {
		max_distance = max(max_distance, distance_to_diagonal(*it));
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