#ifndef _TMETRICS_HPP
#define _TMETRICS_HPP

#include <list>
#include <string>
#include <vector>
#include <exception>
#include <fstream>
#include <chrono>

typedef unsigned int uint;

struct ModeDependentMetrics {
	ModeDependentMetrics(double bandwidth, double density, double dev)
		: fiber_bandwidth(bandwidth), fiber_density(density), std_dev(dev) { }
	double fiber_bandwidth;
	double fiber_density;
	double std_dev;
};

class Tmetrics {
public:
	Tmetrics(const std::string & in_file, bool no_values = false, bool verbose = false);

	void mode_dependent_metrics(); // ALL fiber bandwidth | fiber density metrics for all modes | fiber occupation std. dev.
	void mode_independent_metrics(); // For all NNZ, avg. distance to diag. | pairwise diff. avg | normalized pairwise diff.
private:
	struct Coordinate {
		Coordinate(const std::vector<uint> & vec) : coor(vec) { }
		std::vector<uint> coor;
	};

	struct Comparator {
		Comparator(uint mode)
			: mode(mode) { }

		bool operator() (const Coordinate & lhs, const Coordinate & rhs) const;

		const uint mode;
	};

	// Member variables
	std::list<Coordinate> coords;
	std::vector<uint> diagonal; // for mode independent metrics
	bool no_values; // CLI option
	bool verbose; // CLI option
	std::list<uint> fiber_indices; // stores the index of coords for which a new fiber begins
	double diagonal_self_dot_product;

	// Mode dependent metrics
	ModeDependentMetrics fiber_metrics(uint mode); // for one fiber, returns the avg. fiber bandwidth & density

	// Mode independent metrics
	std::pair<uint, double> pairwise_difference(const std::list<Coordinate>::const_iterator &) const; // for a given coordinate, returns the max pairwise difference
	double distance_to_diagonal(const std::list<Coordinate>::const_iterator &) const;

	// Utilities
	void createFibers(uint mode);
	uint dot_product(const std::vector<uint> & u1, const std::vector<uint> & u2) const;
	double std_dev(const std::list<uint> & numbers, const double x_bar) const;
};

#endif