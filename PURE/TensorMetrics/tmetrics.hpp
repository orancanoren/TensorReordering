#ifndef _TMETRICS_HPP
#define _TMETRICS_HPP

#include <list>
#include <string>
#include <vector>
#include <exception>
#include <fstream>

typedef unsigned int uint;

class Tmetrics {
public:
	Tmetrics(const std::string & in_file, bool no_values = false, bool verbose = false);

	void all_metrics_all_modes();

private:
	struct Coordinate {
		Coordinate(const std::vector<int> & vec) : coor(vec) { }
		std::vector<int> coor;

	};

	struct Comparator {
		Comparator(uint mode1, uint mode2)
			: mode1(mode1), mode2(mode2) { }

		bool operator() (const Coordinate & lhs, const Coordinate & rhs) const;

		const int mode1, mode2;
	};

	std::list<Coordinate> coords;
	bool no_values;
	std::vector<int> diagonal;

	double distance_to_diagonal(const Coordinate & coord, uint mode1, uint mode2) const;
	void metrics_on_modes(uint mode1, uint mode2); // prints out slice metrics for a given pair of tensor modes
	double * all_metrics(uint low, uint high, uint mode1, uint mode2) const;
	double max_distance_to_diagonal(uint mode1, uint mode2) const;
	double metric_1(bool normalize = true) const;

	mutable unsigned int current_mode;
	bool verbose;
};

#endif