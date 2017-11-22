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
	chrono::high_resolution_clock::time_point begin, end;
	if (verbose) {
		cout << "Start: read the tensor file" << endl;
		begin = chrono::high_resolution_clock::now();
	}
	diagonal.resize(dimension, 0);
	is.seekg(0);
	uint line_count = 0;
	while (!is.eof()) {
		line_count++;
		vector<uint> current_coordinates(dimension);
		for (int i = 0; i < dimension; i++) {
		  if (is.eof()) break;
			uint component;
			is >> component;
			if (is.fail()) break;

			current_coordinates[i] = component;
			diagonal[i] = diagonal[i] > component ? diagonal[i] : component;
		}
		if (!no_values) {
			double value;
			is >> value;
		}
		coords.push_back(Coordinate(current_coordinates));
	}
	cout << "line count: " << line_count << endl;
	if (verbose) {
		end = chrono::high_resolution_clock::now();
		cout << "End: read the tensor file [" << chrono::duration_cast<chrono::milliseconds>(end - begin).count() << " ms]" << endl;
	}

	cout << "Dimensions: ";
	for (vector<uint>::const_iterator it = diagonal.cbegin(); it != diagonal.cend(); it++) {
		cout << *it;
		if (next(it, 1) != diagonal.end()) {
			cout << "x";
		}
		}
	cout << endl;
	
	diagonal_self_dot_product = dot_product(diagonal, diagonal);
}

// CLASS Tmetrics | Public Member Function Definitions

void Tmetrics::mode_dependent_metrics() {
	cout << "--------- Mode Dependent Metrics ---------" << endl
		 << "<avg. fiber bandwidth> <avg. fiber density>" << endl;
	for (uint i = 0; i < diagonal.size(); i++) {
		ModeDependentMetrics metrics = fiber_metrics(i);
		cout << endl << "mode " + to_string(i)  << ": " << metrics.fiber_bandwidth << " " << metrics.fiber_density << endl << endl;
	}
}

void Tmetrics::mode_independent_metrics() const {
	// Calculation of mode independent metrics are done locally
	// Computed metrics are: avg. distance to super diagonal, 
	// avg. normalized pairwise coordinate difference,
	// avg. pairwise coordinate differenceo
	cout << "-------- Mode Independent Metrics --------" << endl;
	chrono::high_resolution_clock::time_point begin, end;
	if (verbose) {
		cout << "Start: computation of mode independent metrics" << endl;
		begin = chrono::high_resolution_clock::now();
	}

	double distance_average = 0.0;
	pair<double, double> pairwise_metrics_sum = { 0.0, 0.0 };
	for (list<Coordinate>::const_iterator coordinate = coords.cbegin(); coordinate != coords.cend(); coordinate++) {
		distance_average += distance_to_diagonal(coordinate) / coords.size();
		pair<uint, double> pairwise_metrics = pairwise_difference(coordinate);
		pairwise_metrics_sum.first += static_cast<double>(pairwise_metrics.first) / coords.size();
		pairwise_metrics_sum.second += pairwise_metrics.second / coords.size();
	}

	if (verbose) {
		end = chrono::high_resolution_clock::now();
		cout << "End: computation of mode independent metrics [" << chrono::duration_cast<chrono::milliseconds>(end - begin).count() << " ms]" << endl;
	}
	cout << "average distance to diagonal: " << distance_average << endl
		<< "average normalized pairwise difference: " << pairwise_metrics_sum.second << endl
		<< "average pairwise difference: " << pairwise_metrics_sum.first << endl;
}

// CLASS Tmetrics | Private Member Function Definitions

ModeDependentMetrics Tmetrics::fiber_metrics(uint mode) {
	chrono::high_resolution_clock::time_point begin, end;

	// 0 - Create fibers
	createFibers(mode);
	fiber_count = fiber_indices.size() - 1;

	// 1 - For each fiber, compute the bandwidth
	// [for now] -> compute the average FB of the current mode
	double average_bandwidth= 0;
	double average_density = 0;

	if (verbose) {
		cout << "Start: Fiber bandwidth & density computation" << endl;
		begin = chrono::high_resolution_clock::now();
	}

	list<Coordinate>::const_iterator it = coords.cbegin();
	uint iterator_position = 0;
	// traversal over fiber indices
	uint total_nnz_count = 0;
	for (list<uint>::const_iterator index = fiber_indices.cbegin(); index != fiber_indices.cend(); index++) {
		uint low_bound = UINT_MAX, high_bound = 0;
		uint nnz_count = 0;
		// traversal over fiber coordinates
		while (iterator_position <= *index) {
			low_bound = low_bound < it->coor[mode] ? low_bound : it->coor[mode];
			high_bound = high_bound > it->coor[mode] ? high_bound : it->coor[mode];
			nnz_count++;
			it++;
			iterator_position++;
		}
		const double bandwidth = high_bound - low_bound + 1; // Bandwidth of one fiber in the mode
		average_bandwidth += bandwidth / fiber_count;
		//cout << "fiber count: " << fiber_count << endl;
		average_density += (bandwidth / nnz_count) / fiber_count;
		total_nnz_count += nnz_count;
	}
	cout << total_nnz_count << endl;
	if (verbose) {
		end = chrono::high_resolution_clock::now();
		cout << "End: Fiber bandwidth & density computation [" << chrono::duration_cast<chrono::milliseconds>(end - begin).count() << " ms]" << endl;
	}

	ModeDependentMetrics metrics(average_bandwidth, average_density);
	return metrics;
}

double Tmetrics::distance_to_diagonal(const list<Coordinate>::const_iterator & coord_iter) const {
	chrono::high_resolution_clock::time_point begin, end;

	// A = (0, 0, ..., 0) B = (n_1, n_2, ..., n_k) for k-dim. tensor
	const vector<uint> & P = coord_iter->coor;
	// PA vector is equivalent to P & BA vector is equivalent to <diagonal>

	double t = dot_product(P, diagonal)
		/ diagonal_self_dot_product;

	double distance = 0;
	for (uint i = 0; i < diagonal.size(); i++) {
		distance += (P[i] - (t * diagonal[i])) * (P[i] - (t * diagonal[i]));
	}
	distance = sqrt(distance);
	
	return distance;
}

pair<uint, double> Tmetrics::pairwise_difference(const std::list<Coordinate>::const_iterator & coordinates) const {
	// Pre-condition: Assumes that the tensor dimension is greater than 1!
	pair<uint, double> max_values = { 0, INT_MIN };

	for (uint i = 0; i < diagonal.size() - 1; i++) {
		double normalized_diff;
		uint diff;
		for (uint j = i + 1; j < diagonal.size(); j++) {
			uint component1 = coordinates->coor[i], component2 = coordinates->coor[j];
			normalized_diff = abs((static_cast<double>(component1) / diagonal[i]) - (static_cast<double>(component2) / diagonal[j]));
			diff = component1 > component2 ? (component1 - component2) : (component2 - component1);
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
	// 0 - Sort the coordinates WRT the current mode
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
	fiber_indices.clear();
	vector<uint> current_coordinates = coords.cbegin()->coor;
	uint coordinate_index = 1;
	for (list<Coordinate>::const_iterator it = next(coords.cbegin(), 1); it != coords.cend(); it++, coordinate_index++) {
		for (uint i = 0; i < it->coor.size(); i++) {
			if (i != mode && it->coor[i] != current_coordinates[i]) { // if iterator coordinates differ from <current_coordinates>
				current_coordinates = it->coor;
				fiber_indices.push_back(coordinate_index - 1);
			}
		}
	}
	fiber_indices.push_back(coordinate_index - 1); // push the last index to indicate the end of final fiber
	if (verbose) {
		end = chrono::high_resolution_clock::now();
		cout << "End: Detecting fiber indices" << " [" << chrono::duration_cast<chrono::milliseconds>(end - begin).count() << " ms]" << endl;
	}
}

// Mark: Class Tmetrics | Utilities

uint Tmetrics::dot_product(const vector<uint> & u1, const vector<uint> & u2) const {
	assert(u1.size() == u2.size());

	const uint dim = u1.size();
	uint result = 0;
	for (uint i = 0; i < dim; i++) {
		result += u1[i] * u2[i];
	}
	return result;
}
