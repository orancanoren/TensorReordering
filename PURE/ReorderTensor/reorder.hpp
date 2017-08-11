#ifndef _REORDER_HPP
#define _REORDER_HPP

#include <vector>

typedef unsigned int uint;

class Reorder {
public:
	Reorder(std::vector<uint> new_labels);

	void relabel();
private:
	const std::vector<uint> new_labels;
};


#endif