#ifndef _TMETRICS_HPP
#define _TMETRICS_HPP

#include <list>
#include <string>
#include <vector>
#include <exception>
#include <fstream>

class Tmetrics {
public:
	Tmetrics(const std::string & in_file, bool no_values);

	void all_metrics_all_modes();

private:
	struct Coordinate {
		Coordinate(const std::vector<int> & vec) : coor(vec) { }
		std::vector<int> coor;

	};

	struct Comparator {
		Comparator(const Tmetrics & metric_obj, int mode1, int mode2)
			: metric_obj(metric_obj), mode1(mode1), mode2(mode2) { }

		bool operator() (const Coordinate & lhs, const Coordinate & rhs) const;

		const Tmetrics & metric_obj;
		const int mode1, mode2;
	};

	std::list<Coordinate> coords;
	bool no_values;
	std::vector<int> diagonal;

	double distance_to_diagonal(const Coordinate & coord) const;
	void metrics_on_modes(int mode1, int mode2); // prints out slice metrics for a given pair of tensor modes
	double * all_metrics(int low, int high) const;
	double max_distance_to_diagonal() const;
	double metric_1(bool normalize = true) const;

	mutable unsigned int current_mode;
};

#endif