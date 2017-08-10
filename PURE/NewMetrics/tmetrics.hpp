#ifndef _TMETRICS_HPP
#define _TMETRICS_HPP

#include <list>
#include <string>
#include <vector>
#include <exception>
#include <fstream>
#include <chrono>

typedef unsigned int uint;

class Tmetrics {
public:
	Tmetrics(const std::string & in_file, bool no_values = false, bool verbose = false);

	void mode_dependent_metrics(); // ALL fiber bandwidth | fiber density metrics for all modes
	void mode_independent_metrics(); // For all NNZ, avg. distance to diag. | pairwise diff. avg | normalized pairwise diff.
private:
	struct Coordinate {
		Coordinate(const std::vector<int> & vec) : coor(vec) { }
		std::vector<int> coor;
	};

	struct Comparator {
		Comparator(uint mode)
			: mode(mode) { }

		bool operator() (const Coordinate & lhs, const Coordinate & rhs) const;

		const int mode;
	};

	// Member variables
	std::list<Coordinate> coords;
	std::vector<int> diagonal; // for mode independent metrics
	bool no_values; // CLI option
	bool verbose; // CLI option
	std::list<int> fiber_indices; // stores the index of coords for which a new fiber begins

	// Mode dependent metrics
	std::pair<double, double> fiber_metrics(uint mode); // for one fiber, returns the avg. fiber bandwidth & density

	// Mode independent metrics
	std::pair<uint, double> pairwise_difference(const std::list<Coordinate>::const_iterator &) const; // for a given coordinate, returns the max pairwise difference
	double distance_to_diagonal(const std::list<Coordinate>::const_iterator &) const;

	// Utilities
	void createFibers(uint mode);
};

#endif