#ifndef _RELABEL_HPP
#define _RELABEL_HPP

#include <vector>
#include <string>

typedef unsigned int uint;

class Relabel {
public:
	Relabel(const std::string permutation_file, bool verbose);

	void relabel_tensor(const std::string tensor_file, const std::string output_file);
private:
	std::vector<uint> tensor_coordiantes;
	std::vector<uint> permutation_labels;
	std::vector<uint> dimension_widths;

	bool verbose;

	uint getTensorCoordinate(const uint & label) const;
};

#endif