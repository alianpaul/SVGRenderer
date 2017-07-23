#ifndef SOFTWARE_RENDER_H
#define SORTWARE_RENDER_H

#include "matrix3x3.h"
#include <vector>

namespace CGCore
{

struct SVG;
struct SVGElement;
struct Color;
struct Texture;

class SoftwareRenderer
{

public:
	SoftwareRenderer() : 
		m_cursorX(0.f), m_cursorY(0.f)
	{}
	~SoftwareRenderer() {}

	/*
	 * Display the framebuffer content to screen through OpenGL
	 */

	void displayPixels();

	void displayZoom();

	void resize(size_t w, size_t h);

	void setSVG(const SVG* svg);

	void setViewport(float cx, float cy, float cspan);

	void updateViewport(float ux, float uy, float uspan);

	void redraw();
	
	float	m_cursorX;
	float	m_cursorY;

	size_t  m_width;  //window width
	size_t	m_height; //window height

private:

	

	void rasterizePoint(float x, float y, Color color);

	void rasterizeLine(float x0, float y0, float x1, float y1, Color color, float width = 1.0f);

	void rasterizeLineAntialiasing(float x0, float y0, float x1, float y1, Color color, float width = 1.0f);

	void rasterizeTriangle(float x0, float y0, float x1, float y1, float x2, float y2, Color color);

	void rasterizeImage(float x0, float y0, float x1, float y1, const Texture& tex);
	/*for testing of MLAA*/
	void rasterizeMLAA_case1();

private:
	//SVG Element drawing
	void drawSVG();

	void drawSVGElement(const SVGElement* element, Matrix3x3 transMtx);

	void drawSVGPoint(const SVGElement* element, Matrix3x3 transMtx);

	void drawSVGLine(const SVGElement* element, Matrix3x3 transMtx);

	void drawSVGRect(const SVGElement* element, Matrix3x3 transMtx);

	void drawSVGPolygon(const SVGElement* element, Matrix3x3 transMtx);

	void drawSVGGroup(const SVGElement* element, Matrix3x3 transMtx);

	void drawSVGImage(const SVGElement* element, Matrix3x3 transMtx);

	inline Vector2D	  transform(const Vector2D &point, Matrix3x3 transMtx)
	{
		Vector3D u(point.x, point.y, 1);
		u = transMtx * u;
		return Vector2D(u.x / u.z, u.y / u.z);
	}

	Matrix3x3  getNDCToScreen();
	Matrix3x3  getSVGToNDC();

	const SVG				   *m_svg;
	std::vector<unsigned char>  m_framebuffer;

	//view port
	float						m_cx, m_cy, m_span;

};

}
#endif