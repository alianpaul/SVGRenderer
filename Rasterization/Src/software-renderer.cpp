#include "software-renderer.h"
#include "color.h"
#include "console.h"
#include "misc.h"
#include "vector2D.h"
#include "triangular.h"
#include "svg.h"
#include "mlaa.h"

#include <algorithm>
#include <cmath>

#include <GLFW\glfw3.h>

using namespace std;

namespace CGCore
{

/* Set the svg object to render. 
*/
void	SoftwareRenderer::setSVG(const SVG* svg)
{
	m_svg = svg;
	float cx = m_svg->width / 2.f;
	float cy = m_svg->height / 2.f;
	float span = 1.2 * max(m_svg->width, m_svg->height) / 2.f;
	setViewport(cx, cy, span);
}

void	SoftwareRenderer::setViewport(float cx, float cy, float cspan)
{
	m_cx = cx;
	m_cy = cy;
	m_span = cspan;
}

void	SoftwareRenderer::updateViewport(float ux, float uy, float uspan)
{
	m_cx += ux;
	m_cy += uy;
	m_span *= uspan;
	redraw();
}

void SoftwareRenderer::resize(size_t w, size_t h)
{
	m_width = w; m_height = h;
	m_framebuffer.clear();
	m_framebuffer.resize(4 * w * h, 255);

	drawSVG();
}

/*	Clear frame buffer and redraw current svg;
*/
void	SoftwareRenderer::redraw()
{
	memset(&m_framebuffer[0], 255, 4 * m_width * m_height);
	drawSVG();
}

/* Get the SVGToNDC transformation matrix according to current svg size;
* The SVGToNDC matrix transform the SVG coordinates to NDC coordinates. where the window corresponds
* to the [0, 1]^2 rect.
*																		 |
*__ __ __ __ __ __x											      _ _ _ _|_ _ _ _
|			 |								move to center		 |		 |		 |
|	   .	 |		(. is viewport center)  =============>    __ |_ __ __|__ __ _|_ __x
|			 |													 |		 |		 |
|__ __ __ __ |(w, h)											 |_ _ _ _|_ _ _ _|
|																		 |																		 |
y																		y

scale to span and move into [0, 1]^2 rect

 __ __ __ __ __ __ __x
| _ _ _ _ _ _ _ _ |
||				 ||
||               ||
||				 ||
||_ _ _ _ _	_ _	_||
|				  |
|__ __ __ __ __ __|(1, 1)
|
y
*
*/
Matrix3x3 SoftwareRenderer::getSVGToNDC()
{
	double m[9] = { 1, 0, -m_cx + m_span, 
					0, 1, -m_cy + m_span, 
					0, 0, 2 * m_span };
	return Matrix3x3(m);
}

Matrix3x3 SoftwareRenderer::getNDCToScreen()
{
	float  s = min(m_width, m_height);
	double m[9] = { s, 0, (m_width - s ) / 2.f,
					0, s, (m_height - s) / 2.f,
					0, 0, 1 };
	/*
	double m[9] = { s, 0, 0,
					0, s, 0,
					0, 0, 1 };
	*/
	return Matrix3x3(m);
}

void SoftwareRenderer::rasterizeMLAA_case1()
{
	rasterizePoint(2, 0, Color::Black);
	rasterizePoint(6, 1, Color::Black);
	rasterizeLine(0, 3, 2, 3, Color::Black);
	rasterizeLine(0, 4, 5, 4, Color::Black);
	rasterizeLine(1, 5, 7, 5, Color::Black);
	rasterizeLine(3, 6, 5, 6, Color::Black);
	rasterizePoint(m_width - 1, m_height - 1, Color::Black);
}

/*
* Draw the SVG to the framebuffer. We first initialize the m_SVGToScreen transformation matrix
* m_SVGToScreen = NDCToScreen * SVGToNDC. then we draw svg's each element.
*/
void SoftwareRenderer::drawSVG()
{
	//SVGToNDC
	Matrix3x3 SVGToNDC    = getSVGToNDC();
	Matrix3x3 NDCToScreen = getNDCToScreen();

	//Draw the SVG elements
	for (const SVGElement* elemnt : m_svg->elements)
	{
		drawSVGElement(elemnt, NDCToScreen * SVGToNDC);
	}

	//rasterizeMLAA_case1();
	
	MLAntialias mlaa(&m_framebuffer[0], m_width, m_height);
	mlaa.resolve();

	//Draw the canvas frame
	/*
	Vector2D a = transform(Vector2D(0., 0.f), m_SVGToScreen);						a.x--; a.y++;
	Vector2D b = transform(Vector2D(m_svg->width, 0.f), m_SVGToScreen);				b.x++; b.y++;
	Vector2D c = transform(Vector2D(0, m_svg->height), m_SVGToScreen);				c.x--; c.y--;
	Vector2D d = transform(Vector2D(m_svg->width, m_svg->height), m_SVGToScreen);	d.x++; d.y--;
	rasterizeLine(a.x, a.y, b.x, b.y, Color::Black);
	rasterizeLine(a.x, a.y, c.x, c.y, Color::Black);
	rasterizeLine(b.x, b.y, d.x, d.y, Color::Black);
	rasterizeLine(c.x, c.y, d.x, d.y, Color::Black);
	*/
}

/*
* Rasterize a point at location (x, y) with color.Just fill in the nearest pixel's RGBA
* Attention: The SVG coordinate
* ._ _ _ _ _ _ _ _ _X
* |				|
* |				|
* |				|
* |_ _ _ _ _ _ .|
* |			(w, h)
* |
* Y
* /param x: x axis
* /param y: y axis
* /param color: Color at this position
*/
void SoftwareRenderer::rasterizePoint(float x, float y, Color color)
{
	int ix = (int)floor(x); if (ix < 0 || ix >= m_width) return;
	int iy = (int)floor(y); if (iy < 0 || iy >= m_height) return;

	unsigned char *pixel = &m_framebuffer[0] + 4 * (ix + iy * m_width);

	//Perform Alpha blending
	float Oa = pixel[3];
	float Na = color.a;
	pixel[0] = (uint8_t)(color.r * 255. * Na + (1 - Na) * pixel[0]); //R
	pixel[1] = (uint8_t)(color.g * 255. * Na + (1 - Na) * pixel[1]); //G
	pixel[2] = (uint8_t)(color.b * 255. * Na + (1 - Na) * pixel[2]); //B
	pixel[3] = (uint8_t)((1. - (1. - Oa) * (1. - Na)) * 255);	     //A

}

/*
* Rasterize a line with color.
* Use Bresenham (https://www.cs.helsinki.fi/group/goa/mallinnus/lines/bresenh.html) to rasterize the line
*/
void SoftwareRenderer::rasterizeLine(float x0, float y0, float x1, float y1, Color color, float width)
{
	x0 = floor(x0) + 0.5;
	y0 = floor(y0) + 0.5;

	float slope = (x0 == x1) ? (slope = 1.0f / FLT_MIN) : ((y1 - y0) / (x1 - x0));
	float r; //Reference component
	float c; //Complement component
	float rend; //Reference componet end
	float cdiff;
	float cinc;
	if (slope > 1 || slope < -1)
	{
		//y+
		if (y0 > y1) { swap(x0, x1), swap(y0, y1); }
		r = y0; rend = y1;
		c = x0; cdiff = 0;
		cinc = 1.f / slope;
	}
	else
	{
		//x+
		if (x0 > x1) { swap(x0, x1), swap(y0, y1); }
		r = x0; rend = x1;
		c = y0; cdiff = 0;
		cinc = slope;
	}

	float &x = (slope > 1 || slope < -1) ? c : r;
	float &y = (slope > 1 || slope < -1) ? r : c;

	while (r <= rend)
	{
		//Draw current pixel

		int ix = floor(x);
		int iy = floor(y);
		if (ix < 0 || ix >= m_width) return;
		if (iy < 0 || iy >= m_height) return;
		unsigned char *pixel = &m_framebuffer[0] + 4 * (ix + iy * m_width);
		float Oa = pixel[3];
		float Na = color.a;
		pixel[0] = (uint8_t)(color.r * 255. * Na + (1 - Na) * pixel[0]); //R
		pixel[1] = (uint8_t)(color.g * 255. * Na + (1 - Na) * pixel[1]); //G
		pixel[2] = (uint8_t)(color.b * 255. * Na + (1 - Na) * pixel[2]); //B
		pixel[3] = (uint8_t)((1. - (1. - Oa) * (1. - Na)) * 255);	     //A

		//Update r,c, cdiff
		r += 1.f;
		float caccudiff = cdiff + cinc;
		if (caccudiff >= 0.5f)
		{
			c += 1.f;
			cdiff = caccudiff - 1;
		}
		else if (caccudiff <= -0.5f)
		{
			c -= 1.f;
			cdiff = caccudiff + 1;
		}
		else
		{
			cdiff = caccudiff;
		}
	}
}

/*
* Raterize a line with antialiasing.
* Use Xiaolin Wu's line algorithm https://unionassets.com/blog/algorithm-brezenhema-and-wu-s-line-299
*/
void SoftwareRenderer::rasterizeLineAntialiasing(float x0, float y0, float x1, float y1, Color color, float width)
{
	bool isSteep = abs(y0 - y1) > abs(x0 - x1);
	if ( isSteep ) { swap(x0, y0); swap(x1, y1); }
	if (x0 > x1) { swap(x0, x1); swap(y0, y1); }

	float gradient = (y1 - y0) / (x1 - x0);

	float x = floor(x0) + 0.5,  xend = floor(x1) + 0.5;
	float y = gradient * (x - x0) + y0;
	
	for (; x <= xend; x += 1.f, y += gradient)
	{
		float xs = x;
		float ys = floor(y) + 0.5;
		//Draw current two pixels
		float diff = y - ys;	 // diff = y - ysample point, sample piont(xs, floor(y) + 0.5)
		if (diff >= 0)
		{
			//Draw (xs, ys) (xs, ys + 1)
			if (abs(gradient) < 0.01 && abs(diff - 0.5f) < 0.01 ) { diff = 0.f; } // Prevent the line from be to light in color
			if (isSteep)
			{
				rasterizePoint(ys,       xs, (1.f - diff) * color);
				rasterizePoint(ys + 1.f, xs, diff         * color);
			}
			else
			{
				rasterizePoint(xs, ys,       (1.f - diff) * color);
				rasterizePoint(xs, ys + 1.f, diff         * color);
			}
		}
		else
		{
			//Draw (xs, ys) (xs, ys - 1)
			if (abs(gradient) < 0.01 && abs(diff + 0.5f) < 0.01) { diff = 0.f; }
			if (isSteep)
			{
				rasterizePoint(ys,       xs, (1.f + diff) * color);
				rasterizePoint(ys - 1.f, xs, -diff        * color);
			}
			else
			{
				rasterizePoint(xs, ys,        (1.f + diff) * color);
				rasterizePoint(xs, ys - 1.f,  - diff       * color);
			}
			

		}
	}
}

/* Rasterize a triangle
* __ __ __ __ __ __ __
*|					  |
*|					  |
*|					  |
*|					  |
*|					  |
*|__ __ __			  |
*|		  | 		  |
*|        | 		  |
*|        | 		  |
*|__ __ __|__ __ __ __|
*
* We first find the whole bounding box of the tringle, then we test each block in the bounding box.
* We use the early out strategy, if the pixels on the edges of the block not in the triangle, the whole 
* block will not across the triangle.
*/
void SoftwareRenderer::rasterizeTriangle(float x0, float y0, float x1, float y1, float x2, float y2, Color color)
{
	//1. Construct the 3 edges.
	Triangle triangle(x0, y0, x1, y1, x2, y2);
	
	//2. Find the Bounding box range of the triangle
	float xmin = min({ x0, x1, x2 }), ymin = min({y0, y1, y2});
	float xmax = max({ x0, x1, x2 }), ymax = max({y0, y1, y2});

	//3. Test the points in inner Bounding box with size 10 * 10
	float len = 10.f;

	for (float xbmin = floor(xmin); xbmin <= floor(xmax); xbmin += len)
	{
		for (float ybmin = floor(ymin); ybmin <= floor(ymax); ybmin += len)
		{
			//Test if the 
			bool isCross = false; //if this inner bounding box edges across the triangle

			float xsbmin = xbmin + 0.5, xsbmax = xsbmin + len - 1.f;
			float ysbmin = ybmin + 0.5, ysbmax = ysbmin + len - 1.f;

			for (float i = 0.f; i < len - 1.f; i += 1.f)
			{
				float x, y;
				//bottom
				x = xsbmin + i;
				y = ysbmin;
				if (triangle.isIn(x, y))
				{
					isCross = true;
					rasterizePoint(x, y, color);
				}

				//right
				x = xsbmax;
				y = ysbmin + i;
				if (triangle.isIn(x, y))
				{
					isCross = true;
					rasterizePoint(x, y, color);
				}

				//top
				x = xsbmax - i;
				y = ysbmax;
				if (triangle.isIn(x, y))
				{
					isCross = true;
					rasterizePoint(x, y, color);
				}

				//left
				x = xsbmin;
				y = ysbmax - i;
				if (triangle.isIn(x, y))
				{
					isCross = true;
					rasterizePoint(x, y, color);
				}

			}

			if (isCross)
			{
				for (float x = xsbmin + 1; x < xsbmax ; x += 1.f)
				{
					for (float y = ysbmin + 1; y < ysbmax; y += 1.f)
					{
						if (triangle.isIn(x, y))
						{
							rasterizePoint(x, y, color);
						}
					}
				}
			}

		}
	}

}

void SoftwareRenderer::rasterizeImage(float x0, float y0, float x1, float y1, const Texture& tex)
{
	float w = x1 - x0, h = y1 - y0;
	for (float y = floor(y0) + 0.5; y < y1; ++y)
	{
		float v = (y - y0) / h;
		for (float x = floor(x0) + 0.5; x < x1; ++x)
		{
			float u = (x - x0) / w;
			//Color c = Sampler2D::sampleNearest(tex, u, v);
			Color c = Sampler2D::sampleBilinear(tex, u, v);
			//Color c = Sampler2D::sampleTrilinear(tex, u, v, 1.0 / w, 1.0 / h);
			rasterizePoint(x, y, c);
		}
	}
}

void SoftwareRenderer::drawSVGElement(const SVGElement* element, Matrix3x3 transMtx)
{
	switch (element->type)
	{
	case POINT:
		drawSVGPoint(element, transMtx);
		break;
	case LINE:
		drawSVGLine(element, transMtx);
		break;
	case RECT:
		drawSVGRect(element, transMtx);
		break;
	case POLYGON:
		drawSVGPolygon(element, transMtx);
		break;
	case GROUP:
		drawSVGGroup(element, transMtx);
	case IMAGE:
		drawSVGImage(element, transMtx);
	default:
		break;
	}
}

void SoftwareRenderer::drawSVGPoint(const SVGElement* element, Matrix3x3 transMtx)
{
	const Point& point = static_cast<const Point&>(*element);
	Vector2D p = transform(point.position, transMtx * point.transform);
	rasterizePoint(p.x, p.y, point.style.strokeColor);
}

void SoftwareRenderer::drawSVGLine(const SVGElement* element, Matrix3x3 transMtx)
{
	const Line& line = static_cast<const Line&>(*element);
	Vector2D from	= transform(line.from, transMtx * line.transform);
	Vector2D to		= transform(line.to,   transMtx * line.transform);
	rasterizeLine(from.x, from.y, to.x, to.y, line.style.strokeColor);
}

void SoftwareRenderer::drawSVGRect(const SVGElement* element, Matrix3x3 transMtx)
{
	const Rect& rect = static_cast<const Rect&>(*element);
	Vector2D p0 = transform(rect.position,					transMtx * rect.transform);
	Vector2D p1 = transform(rect.position + rect.dimension,	transMtx * rect.transform);
	
	Color color = rect.style.strokeColor;
	if ( color.a != 0)
	{
		rasterizeLine(p0.x,p0.y,  p1.x,p0.y,  color);
		rasterizeLine(p0.x,p0.y,  p0.x,p1.y,  color);
		rasterizeLine(p1.x,p1.y,  p1.x,p0.y,  color);
		rasterizeLine(p1.x,p1.y,  p0.x,p1.y,  color);
	}

	color = rect.style.fillColor;
	if (color.a != 0)
	{
		rasterizeTriangle(p0.x,p0.y, p0.x,p1.y,  p1.x,p1.y, color);
		rasterizeTriangle(p0.x,p0.y, p1.x,p0.y,  p1.x,p1.y, color);
	}
}

void SoftwareRenderer::drawSVGPolygon(const SVGElement* element, Matrix3x3 transMtx)
{
	const Polygon& poly = static_cast<const Polygon&>(*element);
	vector<Vector2D> trgls;
	triangulate(poly, trgls);

	for (Vector2D& p : trgls)
	{
		p = transform(p, transMtx * poly.transform);
	}

	Color color = poly.style.fillColor;
	if (color.a != 0)
	{
		for (int i = 0; i < trgls.size(); i+=3)
		{
			rasterizeTriangle(	trgls[i].x,     trgls[i].y, 
								trgls[i + 1].x, trgls[i + 1].y, 
								trgls[i + 2].x, trgls[i + 2].y, 
								color);
		}
	}
	
	color = poly.style.strokeColor;
	if (color.a != 0)
	{
		int nPoints = poly.points.size();
		for (int i = 0; i < nPoints; ++i)
		{
			Vector2D p0 = transform(poly.points[i % nPoints],		transMtx * poly.transform);
			Vector2D p1 = transform(poly.points[(i + 1)% nPoints],  transMtx * poly.transform);
			rasterizeLine(p0.x,p0.y, p1.x,p1.y, color);
		}
	}


}

void SoftwareRenderer::drawSVGGroup(const SVGElement* element, Matrix3x3 transMtx)
{
	const Group& group = static_cast<const Group&>(*element);
	for (size_t i = 0; i < group.elements.size(); i++)
	{	
		drawSVGElement(group.elements[i], transMtx * group.transform);
	}
}

void SoftwareRenderer::drawSVGImage(const SVGElement* element, Matrix3x3 transMtx)
{
	const Image& image = static_cast<const Image&>(*element);
	Vector2D p0 = transform(image.position,					  transMtx * image.transform);
	Vector2D p1 = transform(image.position + image.dimension, transMtx * image.transform);

	rasterizeImage(p0.x, p0.y, p1.x, p1.y, image.tex);
}

void SoftwareRenderer::displayPixels()
{
	const unsigned char *pixels = &m_framebuffer[0];

	glViewport(0, 0, m_width, m_height);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, m_width, 0, m_height, 0, 0);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glTranslatef(-1, 1, 0);

	glRasterPos2f(0, 0);
	glPixelZoom(1.0, -1.0);
	glDrawPixels(m_width, m_height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
	glPixelZoom(1.0, 1.0);

	glPopAttrib();
	glMatrixMode(GL_PROJECTION); glPopMatrix();
	glMatrixMode(GL_MODELVIEW); glPopMatrix();
	
}

void SoftwareRenderer::displayZoom()
{
	
	size_t regionSize = 32; // we want zoom in regionSize * regionSize pixels in the original frame.

	size_t zoomFactor = 16; // each pixel upsample to zoomFactor * zoomFactor pixels
	size_t frameSize  = min(m_width, m_height);
	if ( regionSize * zoomFactor > frameSize * 0.4 )
	{
		zoomFactor = (frameSize * 0.4) / regionSize;
	}
	size_t zoomSize = regionSize * zoomFactor;

	int xbegin = m_cursorX - (regionSize / 2);
	int ybegin = (m_height - m_cursorY) - (regionSize / 2);
	xbegin = max(0, min(xbegin, (int)(m_width - regionSize)));
	ybegin = max(0, min(ybegin, (int)(m_height - regionSize)));

	//Grab the pixels from the frameBuffer to regionBuffer
	vector<unsigned char> regionBuffer(3 * regionSize * regionSize);
	glReadPixels(xbegin, ybegin, regionSize, regionSize, GL_RGB, GL_UNSIGNED_BYTE, &regionBuffer[0]);
	
	//Unsample to zoomBuffer
	
	vector<unsigned char> zoomBuffer(3 * zoomSize * zoomSize);
	for (size_t ry = 0; ry < regionSize; ++ry)
	{
		size_t zyoffset = ry * zoomFactor;
		for (size_t rx = 0; rx < regionSize; ++rx)
		{
			unsigned char* rpixel = &regionBuffer[0] + 3 * (rx + ry * regionSize);

			size_t zxoffset = rx * zoomFactor;
			for (size_t idy = 0; idy < zoomFactor; ++idy)
			{
				size_t zy = zyoffset + idy;
				for (size_t idx = 0; idx < zoomFactor; ++idx)
				{
					size_t zx = zxoffset + idx;
					unsigned char* zpixel = &zoomBuffer[0] + 3 * (zx + zy * zoomSize);

					zpixel[0] = rpixel[0];
					zpixel[1] = rpixel[1];
					zpixel[2] = rpixel[2];
				}
			}

		}	
	}

	// copy pixels to the screen using OpenGL
	glMatrixMode(GL_PROJECTION); glPushMatrix(); glLoadIdentity();
	glOrtho(0, m_width, 0, m_height, 0.01, 1000.);
	glMatrixMode(GL_MODELVIEW); glPushMatrix(); glLoadIdentity(); 
	glTranslated(0., 0., -1.);

	glRasterPos2i(m_width - zoomSize, m_height - zoomSize);
	glDrawPixels(zoomSize, zoomSize, GL_RGB, GL_UNSIGNED_BYTE, &zoomBuffer[0]);
	glMatrixMode(GL_PROJECTION); glPopMatrix();
	glMatrixMode(GL_MODELVIEW); glPopMatrix();
	
}

}