#ifndef TEXTURE_H
#define TEXTURE_H

#include <vector>

namespace CGCore
{

class Color;

static const int kMaxMipLevels = 14;

struct MipLevel {
	size_t width;
	size_t height;
	std::vector<unsigned char> texels;
};

struct Texture
{
	size_t width;
	size_t height;
	std::vector<MipLevel> mipmap;
	void generateMips(const MipLevel& mip0);
};

class Sampler2D
{
public:
	static Color sampleNearest(const Texture& tex, float u, float v, int level = 0);
	static Color sampleBilinear(const Texture& tex, float u, float v, int level = 0);
	static Color sampleTrilinear(const Texture& tex, float u, float v, float u_scale, float v_scale);
};

}

#endif