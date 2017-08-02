#include "tmetrics.hpp"
#include <string>
#include <fstream>
#include <list>
#include <vector>
#include <climits>
#include <algorithm> // std::min()
#include <numeric> // std::inner_product()
using namespace std;

Tmetrics::Tmetrics(const string & in_file, bool no_values) 
	: no_values(no_values) {
	ifstream is(in_file);
	// 0 - Determine the dimension of the tensor
	string first_line;
	getline(is, first_line);

	int dimension, index = 0;
	dimension = 0;
	while (index != string::npos) {
		index = first_line.find(' ', index + 1);
		dimension++;
	}
	
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
}

// CLASS Tmetrics | Public Member Function Definitions

double * Tmetrics::all_metrics() const {
	double max_distance = INT_MIN;
	double max_value_normalized = INT_MIN;
	double max_value = INT_MIN;

	for (list<Coordinate>::const_iterator it = coords.begin(); it != coords.end(); it++) {
		max_distance = max(max_distance, distance_to_diagonal(*it)); // distance metric
		// compute the pairwise differences and acquire the max
		for (unsigned int i = 0; i < diagonal.size() - 1; i++) { // pairwise difference metrics
			for (unsigned int j = i + 1; j < diagonal.size(); j++) {
				max_value_normalized = max(abs((static_cast<double>(it->coor[i]) / diagonal[i]) - (static_cast<double>(it->coor[j]) / diagonal[j])), max_value_normalized); // normalized  difference metric
				max_value = max(static_cast<double>(abs((it->coor[i] - it->coor[j]))), max_value); // original difference metric
			}
		}
	}

	double metrics[3] = { max_distance, max_value_normalized, max_value };
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