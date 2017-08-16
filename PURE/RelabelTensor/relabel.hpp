#ifndef _RELABEL_HPP
#define _RELABEL_HPP

#include <vector>
#include <string>

typedef unsigned int uint;

class Relabel {
public:
	Relabel(std::string tensor_file, std::string perm_file, bool verbose, bool no_values);
private:
	std::vector< uint > coordiantes;
	std::vector< uint > permutation;
};

#endif