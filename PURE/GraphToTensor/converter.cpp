#include "converter.hpp"
#include <string>
#include <fstream>

using namespace std;

Converter::Converter(string infile, string outfile, bool verbose) 
	: outfile(outfile) {
	// 0 - Create input stream
	ifstream is(infile);

	// 1 - 
}