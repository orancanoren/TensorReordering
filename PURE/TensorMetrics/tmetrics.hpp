#ifndef _TMETRICS_HPP
#define _TMETRICS_HPP

#include <list>
#include <string>
#include <vector>
#include <exception>

class Tmetrics {
public:
	Tmetrics(const std::string & in_file, bool no_values);

	double * all_metrics() const;
	double max_distance_to_diagonal() const;
	double metric_1(bool normalize = true) const;
private:
	struct Coordinate {
		Coordinate(const std::vector<int> & vec) : coor(vec) { }
		std::vector<int> coor;
	};

	std::list<Coordinate> coords;
	bool no_values;
	std::vector<int> diagonal;

	double distance_to_diagonal(const Coordinate & coord) const;
};

#endif