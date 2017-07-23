#ifndef TRIANGULAR_H
#define TRIANGULAR_H

#include <vector>

namespace CGCore
{

struct Polygon;
struct Vector2D;

/*Triangular Edge For edge testing
*/
struct Edge
{

	Edge(float x0, float y0, float x1, float y1, float xin, float yin)
	{
		float dx = x1 - x0, dy = y1 - y0;
		A = dy; B = -dx; C = -dy * x0 + dx * y0;
		if (A * xin + B * yin + C < 0)
		{
			A = -A; B = -B; C = -C;
		}
	}

	bool isIn(float x, float y)
	{
		return A * x + B * y + C >= 0;
	}

	float A;
	float B;
	float C;
};

struct Triangle
{
	Triangle(float x0, float y0, float x1, float y1, float x2, float y2)
		: e0(x0, y0, x1, y1, x2, y2), e1(x0, y0, x2, y2, x1, y1), e2(x1, y1, x2, y2, x0, y0)
	{}

	bool isIn(float x, float y)
	{
		return e0.isIn(x, y) && e1.isIn(x, y) && e2.isIn(x, y);
	}

	Edge e0;
	Edge e1;
	Edge e2;
};

// triangulates a polygon and save the result as a triangle list
void triangulate(const Polygon& polygon, std::vector<Vector2D>& triangles);

}

#endif