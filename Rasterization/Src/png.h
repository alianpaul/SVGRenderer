#ifndef PNG_H
#define PNG_H

#include <vector>

namespace CGCore
{

struct PNG {
	int width;
	int height;
	std::vector<unsigned char> pixels;
}; // class PNG

class PNGParser {
public:
	static int load(const unsigned char* buffer, size_t size, PNG& png);
	static int load(const char* filename, PNG& png);
	static int save(const char* filename, const PNG& png);
}; // class PNGParser


}

#endif