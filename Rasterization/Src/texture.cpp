#include "texture.h"
#include "color.h"

#include <algorithm>
#include <vector>
#include <cstdint>

using namespace std;

namespace CGCore
{

inline void uint8_to_float(float dst[4], unsigned char *src) 
{
	uint8_t *src_uint8 = (uint8_t *)src;
	dst[0] = src_uint8[0] / 255.f;
	dst[1] = src_uint8[1] / 255.f;
	dst[2] = src_uint8[2] / 255.f;
	dst[3] = src_uint8[3] / 255.f;
}

inline void float_to_uint8(unsigned char *dst, float src[4]) 
{
	uint8_t *dst_uint8 = (uint8_t *)dst;
	dst_uint8[0] = (uint8_t)(255.f * max(0.0f, min(1.0f, src[0])));
	dst_uint8[1] = (uint8_t)(255.f * max(0.0f, min(1.0f, src[1])));
	dst_uint8[2] = (uint8_t)(255.f * max(0.0f, min(1.0f, src[2])));
	dst_uint8[3] = (uint8_t)(255.f * max(0.0f, min(1.0f, src[3])));
}

void Texture::generateMips(const MipLevel& mip0)
{
	mipmap.push_back(mip0);
	int startLevel = 0;

	// allocate sublevels
	int baseWidth = mipmap[startLevel].width;
	int baseHeight = mipmap[startLevel].height;
	int numSubLevels = (int)(log2f((float)max(baseWidth, baseHeight)));

	numSubLevels = min(numSubLevels, kMaxMipLevels - startLevel - 1);
	mipmap.resize(startLevel + numSubLevels + 1);

	int width = baseWidth;
	int height = baseHeight;
	for (int i = 1; i <= numSubLevels; i++) {

		MipLevel &level = mipmap[startLevel + i];

		// handle odd size texture by rounding down
		width = max(1, width / 2);
		//assert (width > 0);
		height = max(1, height / 2);
		//assert (height > 0);

		level.width = width;
		level.height = height;
		level.texels = vector<unsigned char>(4 * width * height);
	}

	// create mips
	int subLevels = numSubLevels - (startLevel + 1);
	for (int mipLevel = startLevel + 1; mipLevel < startLevel + subLevels + 1;
		mipLevel++) {

		MipLevel &prevLevel = mipmap[mipLevel - 1];
		MipLevel &currLevel = mipmap[mipLevel];

		int prevLevelPitch = prevLevel.width * 4; // 32 bit RGBA
		int currLevelPitch = currLevel.width * 4; // 32 bit RGBA

		unsigned char *prevLevelMem;
		unsigned char *currLevelMem;

		currLevelMem = (unsigned char *)&currLevel.texels[0];
		prevLevelMem = (unsigned char *)&prevLevel.texels[0];

		float wDecimal, wNorm, wWeight[3];
		int wSupport;
		float hDecimal, hNorm, hWeight[3];
		int hSupport;

		float result[4];
		float input[4];

		// conditional differentiates no rounding case from round down case
		if (prevLevel.width & 1) {
			wSupport = 3;
			wDecimal = 1.0f / (float)currLevel.width;
		}
		else {
			wSupport = 2;
			wDecimal = 0.0f;
		}

		// conditional differentiates no rounding case from round down case
		if (prevLevel.height & 1) {
			hSupport = 3;
			hDecimal = 1.0f / (float)currLevel.height;
		}
		else {
			hSupport = 2;
			hDecimal = 0.0f;
		}

		wNorm = 1.0f / (2.0f + wDecimal);
		hNorm = 1.0f / (2.0f + hDecimal);

		// case 1: reduction only in horizontal size (vertical size is 1)
		if (currLevel.height == prevLevel.height) {
			//assert (currLevel.height == 1);

			for (int i = 0; i < currLevel.width; i++) {
				wWeight[0] = wNorm * (1.0f - wDecimal * i);
				wWeight[1] = wNorm * 1.0f;
				wWeight[2] = wNorm * wDecimal * (i + 1);

				result[0] = result[1] = result[2] = result[3] = 0.0f;

				for (int ii = 0; ii < wSupport; ii++) {
					uint8_to_float(input, prevLevelMem + 4 * (2 * i + ii));
					result[0] += wWeight[ii] * input[0];
					result[1] += wWeight[ii] * input[1];
					result[2] += wWeight[ii] * input[2];
					result[3] += wWeight[ii] * input[3];
				}

				// convert back to format of the texture
				float_to_uint8(currLevelMem + (4 * i), result);
			}

			// case 2: reduction only in vertical size (horizontal size is 1)
		}
		else if (currLevel.width == prevLevel.width) {
			//assert (currLevel.width == 1);

			for (int j = 0; j < currLevel.height; j++) {
				hWeight[0] = hNorm * (1.0f - hDecimal * j);
				hWeight[1] = hNorm;
				hWeight[2] = hNorm * hDecimal * (j + 1);

				result[0] = result[1] = result[2] = result[3] = 0.0f;
				for (int jj = 0; jj < hSupport; jj++) {
					uint8_to_float(input, prevLevelMem + prevLevelPitch * (2 * j + jj));
					result[0] += hWeight[jj] * input[0];
					result[1] += hWeight[jj] * input[1];
					result[2] += hWeight[jj] * input[2];
					result[3] += hWeight[jj] * input[3];
				}

				// convert back to format of the texture
				float_to_uint8(currLevelMem + (currLevelPitch * j), result);
			}

			// case 3: reduction in both horizontal and vertical size
		}
		else {

			for (int j = 0; j < currLevel.height; j++) {
				hWeight[0] = hNorm * (1.0f - hDecimal * j);
				hWeight[1] = hNorm;
				hWeight[2] = hNorm * hDecimal * (j + 1);

				for (int i = 0; i < currLevel.width; i++) {
					wWeight[0] = wNorm * (1.0f - wDecimal * i);
					wWeight[1] = wNorm * 1.0f;
					wWeight[2] = wNorm * wDecimal * (i + 1);

					result[0] = result[1] = result[2] = result[3] = 0.0f;

					// convolve source image with a trapezoidal filter.
					// in the case of no rounding this is just a box filter of width 2.
					// in the general case, the support region is 3x3.
					for (int jj = 0; jj < hSupport; jj++)
						for (int ii = 0; ii < wSupport; ii++) {
							float weight = hWeight[jj] * wWeight[ii];
							uint8_to_float(input, prevLevelMem +
								prevLevelPitch * (2 * j + jj) +
								4 * (2 * i + ii));
							result[0] += weight * input[0];
							result[1] += weight * input[1];
							result[2] += weight * input[2];
							result[3] += weight * input[3];
						}

					// convert back to format of the texture
					float_to_uint8(currLevelMem + currLevelPitch * j + 4 * i, result);
				}
			}
		}
	}
}

Color Sampler2D::sampleNearest(const Texture& tex, float u, float v, int level )
{
	const MipLevel& mip = tex.mipmap[level];

	int su = (int)floor(u * mip.width);
	int sv = (int)floor(v * mip.height);

	if (su < 0 || su > mip.width || sv < 0 || sv > mip.height ) return Color::White;
	return Color(&mip.texels[0] + 4 * (su + sv * mip.width));
}


Color Sampler2D::sampleBilinear(const Texture& tex, float u, float v, int level)
{
	
	if (u < 0. || u > 1. || v < 0. || v > 1.) return Color::White;

	const MipLevel& mip = tex.mipmap[level];
	u = u * (float)mip.width;
	v = v * (float)mip.height;

	if ( u < 0.5 || u > (float)mip.width - 0.5 || v < 0.5 || v > (float)mip.height - 0.5)
	{
		int su = u, sv = v;
		return Color(&mip.texels[0] + 4 * (su + sv * mip.width));
	}

	int su1 = round(u), sv1 = round(v);
	_ASSERT(su1 >= 0 && su1 < mip.width);
	_ASSERT(sv1 >= 0 && sv1 < mip.height);
	int su0 = su1 - 1,  sv0 = sv1 - 1;
	_ASSERT(su0 >= 0 && su0 < mip.width);
	_ASSERT(sv0 >= 0 && sv0 < mip.height);
	Color c1(&mip.texels[0] + 4 * (su0 + sv0 * mip.width));
	Color c2(&mip.texels[0] + 4 * (su1 + sv0 * mip.width));
	Color c3(&mip.texels[0] + 4 * (su0 + sv1 * mip.width));
	Color c4(&mip.texels[0] + 4 * (su1 + sv1 * mip.width));

	//in u dir
	float tu = u - ((float)su0 + 0.5);
	Color c12 = c1 * (1.f - tu) + c2 * tu;
	Color c34 = c3 * (1.f - tu) + c3 * tu;
	//in v dir
	float tv = v - ((float)sv0 + 0.5);
	return c12 * (1 - tv) + c34 * tv;
}

Color Sampler2D::sampleTrilinear (const Texture& tex, float u, float v, float u_scale, float v_scale)
{
	float L = log2f( max(u_scale * tex.mipmap[0].width, v_scale * tex.mipmap[0].height));
	int Llow = max(0.f, floor(L));
	int Lhigh = Llow + 1; _ASSERT(Lhigh <= kMaxMipLevels);
	float t = L - Llow;
	
	return (1 - t) * sampleBilinear(tex, u, v, Llow) + t * sampleBilinear(tex, u, v, Lhigh);
}

}