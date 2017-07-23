#include "mlaa.h"

#include <algorithm>

using namespace std;

namespace CGCore
{

MLAntialias::MLAntialias(unsigned char *framebuffer, size_t w, size_t h) 
	: m_framebuffer(framebuffer), m_height(h), m_width(w)
{
	findAPrimaryEdges();
}

#define IS_EDGE(PIXEL1, PIXEL2) \
	(abs((PIXEL1)[0] - (PIXEL2)[0]) > 16 || \
	 abs((PIXEL1)[1] - (PIXEL2)[1]) > 16 || \
	 abs((PIXEL1)[2] - (PIXEL2)[2]) > 16)

void MLAntialias::findAPrimaryEdges()
{
	for (int yPri = 0; yPri < m_height - 1; ++yPri)
	{
		//Find a primary edge
		int xPriBegin, xPriEnd;
		for (xPriBegin = xPriEnd = 0; xPriEnd < m_width; ++xPriEnd)
		{

			unsigned char * topPixel = &m_framebuffer[0] + 4 * (xPriEnd + yPri * m_width);
			unsigned char * bottomPixel = &m_framebuffer[0] + 4 * (xPriEnd + (yPri + 1) * m_width);

			if (!IS_EDGE(topPixel, bottomPixel))
			{
				if (xPriBegin == xPriEnd)
					++xPriBegin;
				else
				{
					//printf("Begin (%d, %d) End (%d, %d)\n", xPriBegin, yPri, xPriEnd, yPri);
					bool isEdgel = false;
					if (xPriBegin != 0)
					{
						unsigned char * pl = m_framebuffer + 4 * (xPriBegin - 1 + yPri * m_width);
						unsigned char * pr = m_framebuffer + 4 * (xPriBegin + yPri * m_width);
						isEdgel = IS_EDGE(pl, pr);
					}
					unsigned char * pl = m_framebuffer + 4 * (xPriEnd - 1 + yPri * m_width);
					unsigned char * pr = m_framebuffer + 4 * (xPriEnd     + yPri * m_width);
					bool isEdger = IS_EDGE(pl, pr);
					m_rowEdges.emplace_back(xPriBegin, xPriEnd, yPri, isEdgel, isEdger);
					
					xPriBegin = xPriEnd;
					++xPriBegin;
				}
			}
		}

		if (xPriBegin != xPriEnd)
		{
			//printf("Begin (%d, %d) End (%d, %d)\n", xPriBegin, yPri, xPriEnd, yPri);
			//antialiasEdge(xPriBegin, yPri, xPriEnd, yPri);
			bool isEdgel = false;
			if (xPriBegin != 0)
			{
				unsigned char * pl = m_framebuffer + 4 * (xPriBegin - 1 + yPri * m_width);
				unsigned char * pr = m_framebuffer + 4 * (xPriBegin + yPri * m_width);
				isEdgel = IS_EDGE(pl, pr);
			}
			bool isEdger = false;
			m_rowEdges.emplace_back(xPriBegin, xPriEnd, yPri, isEdgel, isEdger);
		}
	}

	for (int xPri = 0; xPri < m_width - 1; ++xPri)
	{
		//Find a primary edge
		int yPriBegin, yPriEnd;
		for (yPriBegin = yPriEnd = 0; yPriEnd < m_height; ++yPriEnd)
		{

			unsigned char * leftPixel = &m_framebuffer[0] + 4 * (xPri + yPriEnd * m_width);
			unsigned char * rightPixel = &m_framebuffer[0] + 4 * (xPri + 1 + yPriEnd * m_width);

			if (!IS_EDGE(leftPixel, rightPixel))
			{
				if (yPriBegin == yPriEnd)
					++yPriBegin;
				else
				{
					//printf("Begin (%d, %d) End (%d, %d)\n", xPri, yPriBegin, xPri, yPriEnd);
					bool isEdget = false;
					if (yPriBegin != 0)
					{
						unsigned char * pt = m_framebuffer + 4 * (xPri + (yPriBegin - 1) * m_width);
						unsigned char * pb = m_framebuffer + 4 * (xPri + yPriBegin * m_width);
						isEdget = IS_EDGE(pt, pb);
					}
					unsigned char * pt = m_framebuffer + 4 * (xPri + (yPriEnd - 1) * m_width);
					unsigned char * pb = m_framebuffer + 4 * (xPri + yPriEnd * m_width);
					bool isEdgeb = IS_EDGE(pt, pb);
					m_colEdges.emplace_back(yPriBegin, yPriEnd, xPri,isEdget, isEdgeb);
					//antialiasEdge(xPriBegin, yPri, xPriEnd, yPri);
					yPriBegin = yPriEnd;
					++yPriBegin;
				}
			}
		}

		if (yPriBegin != yPriEnd)
		{
			bool isEdget = false;
			if (yPriBegin != 0)
			{
				unsigned char * pt = m_framebuffer + 4 * (xPri + (yPriBegin - 1) * m_width);
				unsigned char * pb = m_framebuffer + 4 * (xPri + yPriBegin * m_width);
				isEdget = IS_EDGE(pt, pb);
			}
			bool isEdgeb = false;
			//printf("Begin (%d, %d) End (%d, %d)\n", xPri, yPriBegin, xPri, yPriEnd);
			m_colEdges.emplace_back(yPriBegin, yPriEnd, xPri, isEdget, isEdgeb);;
			//antialiasEdge(xPriBegin, yPri, xPriEnd, yPri);
		}
	}
}

void MLAntialias::resolve()
{
	for (size_t i = 0; i < m_rowEdges.size(); ++i)
	{
		antialiasRowEdge(m_rowEdges[i].begin, m_rowEdges[ i ].end, m_rowEdges[ i ].pri,
			m_rowEdges[i].isEdgeBegin, m_rowEdges[i].isEdgeEnd);
	}

	for (size_t i = 0; i < m_colEdges.size();  ++i)
	{
		antialiasColEdge(m_colEdges[i].begin, m_colEdges[i].end, m_colEdges[i].pri,
			m_colEdges[i].isEdgeBegin, m_colEdges[i].isEdgeEnd);
	}
}

#define UPDATE_PIXEL(PIXELTO, PIXELFROM, A) \
	(PIXELTO)[0] = (1 - A) * (PIXELTO)[0] + A * (PIXELFROM)[0];		\
	(PIXELTO)[1] = (1 - A) * (PIXELTO)[1] + A * (PIXELFROM)[1];		\
	(PIXELTO)[2] = (1 - A) * (PIXELTO)[2] + A * (PIXELFROM)[2];	

void MLAntialias::antialiasRowEdge(int xBegin, int xEnd, int yPri, bool isEdgeBegin, bool isEdgeEnd)
{

	//Horizontal primary edge
	/* (x0,y0) | (x1, y0)			(x2, y0) | (x3, y0)
				-- -- -- -- -- -- -- -- -- --
	   (x0,y1) | (x1, y1)			(x2, y1) | (x3, y1)
	*/
	int y0 = yPri, y1 = yPri + 1;
	int x0 = xBegin - 1, x1 = xBegin, x2 = xEnd - 1, x3 = xEnd;

	if (xBegin != 0)
	{
		//left L
		int yTo = isEdgeBegin ? y0 : y1;
		int yFrom = isEdgeBegin ? y1 : y0;

		float lenEdge = (xEnd != m_width) ? xEnd - xBegin : xEnd - xBegin + 1;

		float c = 0.5;
		float k = -(1.f / lenEdge);

		for (float i = 0.f; i < lenEdge / 2; ++i)
		{
			float h1 = k * i + c;
			float h2 = max(k * (i + 1.f) + c, 0.f);
			float area = (h1 + h2) / 2.f;

			int xTo = x1 + i, xFrom = x1 + i;
			unsigned char * pTo = m_framebuffer + 4 * (xTo + yTo * m_width);
			unsigned char * pFrom = m_framebuffer + 4 * (xFrom + yFrom * m_width);
			UPDATE_PIXEL(pTo, pFrom, area);
		}

	}

	if (xEnd != m_width)
	{
		//right L
		unsigned char * pl = m_framebuffer + 4 * (x2 + y0 * m_width);
		unsigned char * pr = m_framebuffer + 4 * (x3 + y0 * m_width);
		bool isEdge = IS_EDGE(pl, pr);

		int yTo = isEdgeEnd ? y0 : y1;
		int yFrom = isEdgeEnd ? y1 : y0;

		float lenEdge = (xBegin != 0) ? xEnd - xBegin : xEnd - xBegin + 1;

		float c = 0.5;
		float k = 1.f / lenEdge;

		for (float i = 0.f; i > -lenEdge / 2; --i)
		{
			float h1 = k * i + c;
			float h2 = max(k * (i - 1.f) + c, 0.f);
			float area = (h1 + h2) / 2.f;

			int xTo = x2 + i, xFrom = x2 + i;
			unsigned char * pTo = m_framebuffer + 4 * (xTo + yTo * m_width);
			unsigned char * pFrom = m_framebuffer + 4 * (xFrom + yFrom * m_width);
			UPDATE_PIXEL(pTo, pFrom, area);
		}

	}

}

void MLAntialias::antialiasColEdge(int yBegin, int yEnd, int xPri, bool isEdgeBegin, bool isEdgeEnd)
{
	//Vertical primary edge
   /*(x0, y0)   (x1, y0)
		-- --   -- --
	 (x0, y1) | (x1, y1)
	   		  |
	          |
	 (x0, y2) | (x1, y2)
	    -- --   -- --
	 (x0, y3)   (x1, y3)
	*/

	int x0 = xPri, x1 = xPri + 1;
	int y0 = yBegin - 1, y1 = yBegin, y2 = yEnd - 1, y3 = yEnd;
	//top L
	if (yBegin != 0)
	{
		unsigned char * pt = m_framebuffer + 4 * (x0 + y0 * m_width);
		unsigned char * pb = m_framebuffer + 4 * (x0 + y1 * m_width);
		bool isEdge = IS_EDGE(pt, pb);

		int xTo = (isEdgeBegin) ? x0 : x1;
		int xFrom = (isEdgeBegin) ? x1 : x0;

		float lenEdge = (yEnd != m_height) ? yEnd - yBegin : yEnd - yBegin + 1;

		float b = 0.5f;
		float a = 1.f / lenEdge;

		for (float i = 0.f; i > -lenEdge / 2.f; --i)
		{
			float h1 = a * i + b;
			float h2 = max((a * (i - 1.f) + b), 0.f);
			float area = (h1 + h2) / 2.f;

			int yTo = y1 - i, yFrom = y1 - i;
			unsigned char * pTo = m_framebuffer + 4 * (xTo + yTo * m_width);
			unsigned char * pFrom = m_framebuffer + 4 * (xFrom + yFrom * m_width);
			UPDATE_PIXEL(pTo, pFrom, area);
		}

	}

	//bottom L
	if (yEnd != m_height)
	{

		unsigned char * pt = m_framebuffer + 4 * (x0 + y2 * m_width);
		unsigned char * pb = m_framebuffer + 4 * (x0 + y3 * m_width);
		bool isEdge = IS_EDGE(pt, pb);

		int xTo = (isEdgeEnd) ? x0 : x1;
		int xFrom = (isEdgeEnd) ? x1 : x0;

		float lenEdge = (yBegin != 0) ? yEnd - yBegin : yEnd - yBegin + 1;

		float b = 0.5f;
		float a = -1.f / lenEdge;

		for (float i = 0.f; i < lenEdge / 2.f; ++i)
		{
			float h1 = a * i + b;
			float h2 = max(a * (i + 1) + b, 0.f);
			float area = (h1 + h2) / 2.f;

			int yTo = y2 - i, yFrom = y2 - i;
			unsigned char * pTo = m_framebuffer + 4 * (xTo + yTo * m_width);
			unsigned char * pFrom = m_framebuffer + 4 * (xFrom + yFrom * m_width);
			UPDATE_PIXEL(pTo, pFrom, area);
		}

	}
}

}