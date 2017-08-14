#ifndef _CONVERTER_H
#define _CONVERTER_H

#include <string>

class Converter {
public:
	Converter(std::string infile, std::string outfile, bool verbose = false);
private:
	// Member variables
	std::string outfile;
};

#endif