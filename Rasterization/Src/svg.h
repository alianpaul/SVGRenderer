#ifndef SVG_H
#define SVG_H

#include "color.h"
#include "matrix3x3.h"
#include "tinyxml2.h"
#include "texture.h"

#include <vector>

namespace CGCore
{

typedef enum e_SVGElementType
{
	NONE = 0,
	POINT,
	LINE,
	POLYLINE,
	RECT,
	POLYGON,
	ELLIPSE,
	IMAGE,
	GROUP,
}SVGELementType;

struct Style {
	Color strokeColor;
	Color fillColor;
	float strokeWidth;
	float miterLimit;
};

struct SVGElement
{

	SVGElement(SVGELementType _type)
		: type(_type), transform(Matrix3x3::identity())
	{}

	virtual ~SVGElement() {}

	SVGELementType type;
	/*Common element for all SVGElement*/
	//Style
	Style style;
	//Transformation list
	Matrix3x3 transform;
};

struct Group : SVGElement {

	Group() : SVGElement(GROUP) { }
	std::vector<SVGElement*> elements;

	~Group();
};

struct Point : SVGElement{

	Point() : SVGElement(POINT) { }
	Vector2D position;

};

struct Line : SVGElement {

	Line() : SVGElement(LINE) { }
	Vector2D from;
	Vector2D to;

};

struct Polyline : SVGElement {

	Polyline() : SVGElement(POLYLINE) { }
	std::vector<Vector2D> points;

};

struct Rect : SVGElement {

	Rect() : SVGElement(RECT) { }
	Vector2D position;
	Vector2D dimension;

};

struct Polygon : SVGElement {

	Polygon() : SVGElement(POLYGON) { }
	std::vector<Vector2D> points;

};

struct Ellipse : SVGElement {

	Ellipse() : SVGElement(ELLIPSE) { }
	Vector2D center;
	Vector2D radius;

};

struct Image : SVGElement {

	Image() : SVGElement(IMAGE) { }
	Vector2D position;
	Vector2D dimension;
	Texture tex;
};

struct SVG {

	~SVG();
	float width, height;
	std::vector<SVGElement*> elements;

};

class SVGParser {
public:

	static void load(const char* filename, SVG* svg);
	static void save(const char* filename, const SVG* svg);

private:

	// parse a svg file
	static void parseSVG(tinyxml2::XMLElement* xml, SVG* svg);

	// parse shared properties of svg elements
	static void parseElement(tinyxml2::XMLElement* xml, SVGElement* element);

	// parse type specific properties
	static void parsePoint(tinyxml2::XMLElement* xml, Point*  point);
	static void parseLine(tinyxml2::XMLElement* xml, Line*  line);
	static void parsePolyline(tinyxml2::XMLElement* xml, Polyline* polyline);
	static void parseRect(tinyxml2::XMLElement* xml, Rect*   rect);
	static void parsePolygon(tinyxml2::XMLElement* xml, Polygon*  polygon);
	static void parseEllipse(tinyxml2::XMLElement* xml, Ellipse*  ellipse);
	static void parseImage(tinyxml2::XMLElement* xml, Image*  image);
	static void parseGroup(tinyxml2::XMLElement* xml, Group*    group);


}; // class SVGParser

}

#endif

