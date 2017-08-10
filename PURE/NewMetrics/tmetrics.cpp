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
	: no_values(no_values), verbose(verbose) {
	// 0 - Create the input stream
	ifstream is(in_file);
	if (!is.is_open()) {
		cout << "Cannot open the provided tensor file" << endl
			<< "****************************************" << endl;
		exit(1);
	}
	// 1 - Determine the dimension of the tensor
	string first_line;
	getline(is, first_line);

	int dimension = 0, index = 0;
	while (index != string::npos) {
		index = first_line.find(' ', index + 1);
		dimension++;
	}

	if (!no_values)
		dimension--;

	// 2 - Read the input stream
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

void Tmetrics::mode_dependent_metrics() {
	for (uint i = 0; i < diagonal.size(); i++) {
		pair<double, double> metrics = fiber_metrics(i);
		ofstream os("mode_" + to_string(i) + ".metric");
		os << metrics.first << " " << metrics.second << endl;
		os.close();
	}
}

void Tmetrics::mode_independent_metrics() {
	chrono::high_resolution_clock::time_point begin, end;
	if (verbose) {
		cout << "Start: computation of mode independent metrics" << endl;
		begin = chrono::high_resolution_clock::now();
	}

	double distance_sum = 0;
	pair<uint, double> pairwise_metrics_sum = { 0, 0 };
	for (list<Coordinate>::const_iterator coordinate = coords.cbegin(); coordinate != coords.cend(); coordinate++) {
		distance_sum += distance_to_diagonal(coordinate);
		pair<uint, double> pairwise_metrics = pairwise_difference(coordinate);
		pairwise_metrics_sum.first += pairwise_metrics.first;
		pairwise_metrics_sum.second += pairwise_metrics.second;
	}

	if (verbose) {
		end = chrono::high_resolution_clock::now();
		cout << "End: computation of mode independent metrics [" << chrono::duration_cast<chrono::milliseconds>(end - begin).count() << " ms]" << endl;
	}
	cout << "average distance to diagonal: " << distance_sum / coords.size() << endl
		<< "average normalized pairwise difference: " << pairwise_metrics_sum.second / coords.size() << endl
		<< "average pairwise difference: " << pairwise_metrics_sum.first / coords.size() << endl;
}

// CLASS Tmetrics | Private Member Function Definitions

pair<double, double> Tmetrics::fiber_metrics(uint mode) {
	chrono::high_resolution_clock::time_point begin, end;

	// 0 - Create fibers
	createFibers(mode);

	// 1 - For each fiber, compute the bandwidth
	// [for now] -> compute the average FB of the current mode
	double bandwidth_sum = 0;
	double density_sum = 0;

	if (verbose) {
		cout << "Start: Fiber bandwidth & density computation" << endl;
		begin = chrono::high_resolution_clock::now();
	}

	list<Coordinate>::const_iterator it = coords.cbegin();
	int iterator_position = 0;
	int fiber_count = 0;
	for (list<int>::const_iterator index = fiber_indices.cbegin(); index != fiber_indices.cend(); index++, fiber_count++, it++) {
		int low_bound = INT_MAX, high_bound = INT_MIN;
		int nnz_count = 0;
		for (; iterator_position != *index; iterator_position++) {
			low_bound = min(low_bound, it->coor[mode]);
			high_bound = max(high_bound, it->coor[mode]);
			nnz_count++;
		}
		const int bandwidth = high_bound - low_bound; // Bandwidth of one fiber in the mode
		bandwidth_sum += bandwidth;
		density_sum += static_cast<double>(bandwidth) / nnz_count;
	}

	if (verbose) {
		end = chrono::high_resolution_clock::now();
		cout << "End: Fiber bandwidth & density computation [" << chrono::duration_cast<chrono::milliseconds>(end - begin).count() << " ms]" << endl;
	}
	return{ bandwidth_sum / fiber_count, density_sum / fiber_count };
}

double Tmetrics::distance_to_diagonal(const list<Coordinate>::const_iterator & coord_iter) const {
	chrono::high_resolution_clock::time_point begin, end;

	// A = (0, 0, ..., 0) B = (n_1, n_2, ..., n_k) for k-dim. tensor
	const vector<int> & P = coord_iter->coor;
	// PA vector is equivalent to P & BA vector is equivalent to <diagonal>

	double t = inner_product(P.cbegin(), P.cend(), diagonal.cbegin(), 0)
		/ inner_product(diagonal.cbegin(), diagonal.cend(), diagonal.begin(), 0);

	double distance = 0;
	for (uint i = 0; i < diagonal.size(); i++) {
		distance += P[i] - (t * diagonal[i]);
	}
	distance = sqrt(distance);
	
	return distance;
}

pair<uint, double> Tmetrics::pairwise_difference(const std::list<Coordinate>::const_iterator & coordinates) const {
	// Pre-condition: Assumes that the tensor dimension is greater than 1!
	pair<uint, double> max_values = { INT_MIN, INT_MIN };

	for (uint i = 0; i < diagonal.size() - 1; i++) {
		double normalized_diff;
		uint diff;
		for (uint j = i + 1; j < diagonal.size(); j++) {
			normalized_diff = abs((static_cast<double>(coordinates->coor[i]) / diagonal[i]) - (static_cast<double>(coordinates->coor[j]) / diagonal[j]));
			diff = abs((coordinates->coor[i] - coordinates->coor[j]));
		}
		max_values.first = max(max_values.first, diff);
		max_values.second = max(max_values.second, normalized_diff);
	}

	return max_values;
}

bool Tmetrics::Comparator::operator() (const Coordinate & lhs, const Coordinate & rhs) const {
	assert(mode < lhs.coor.size() && lhs.coor.size() == rhs.coor.size());

	// operator() implements the `strict less` opearation
	const uint dimension = lhs.coor.size();
	for (uint i = 0; i < dimension; i++) {
		// Treat coordinate values as if mode1 & mode2 were at the end
		if (i != mode) {
			if (lhs.coor[i] < rhs.coor[i])
				return true;
			else if (lhs.coor[i] > rhs.coor[i])
				return false;
		}
	}
	if (lhs.coor[mode] < rhs.coor[mode]) {
		return true;
	}
	return false;
}

void Tmetrics::createFibers(uint mode) {
	// 0 - Sort the coordinates according to the current mode
	chrono::high_resolution_clock::time_point begin, end;
	if (verbose) {
		cout << "Start: Sorting coordinates WRT mode " << mode << endl;
		begin = chrono::high_resolution_clock::now();
	}
	Comparator comparison_func(mode);
	coords.sort(comparison_func);
	if (verbose) {
		end = chrono::high_resolution_clock::now();
		cout << "End: Sorting coordinates WRT mode " << mode << " [" << chrono::duration_cast<chrono::milliseconds>(end - begin).count() << " ms]" << endl
			<< "Start: Detecting fiber indices" << endl;
		begin = chrono::high_resolution_clock::now();
	}

	// 1 - Detect the indices of fibers
	vector<int> current_coordinates = coords.cbegin()->coor;
	uint coordinate_index = 0;
	for (list<Coordinate>::const_iterator it = next(coords.cbegin(), 1); it != coords.cend(); it++, coordinate_index++) {
		for (uint i = 0; i < it->coor.size(); i++) {
			if (i != mode && it->coor[i] != current_coordinates[i]) {
				current_coordinates = it->coor;
				fiber_indices.push_back(coordinate_index);
			}

		}
	}
	if (verbose) {
		end = chrono::high_resolution_clock::now();
		cout << "End: Detecting fiber indices" << " [" << chrono::duration_cast<chrono::microseconds>(end - begin).count() << " ms]" << endl;
	}
}