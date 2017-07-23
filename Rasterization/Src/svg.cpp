#include "svg.h"
#include "console.h"
#include "tinyxml2.h"
#include "misc.h"
#include "png.h"

#include <sstream>
#include <algorithm>

using namespace std;
using namespace tinyxml2;

namespace CGCore
{

void SVGParser::load(const char* filename, SVG* svg)
{

	XMLDocument doc;
	doc.LoadFile(filename);
	if (doc.Error())
	{
		doc.PrintError();
		exit(1);
	}

	XMLElement* root = doc.FirstChildElement("svg");
	if (!root)
	{
		out_err("Not an SVG file");
	}

	string wcstr = root->Attribute("width");
	string hcstr = root->Attribute("height");

	
	svg->width = stof(wcstr);
	svg->height = stof(hcstr);

	parseSVG(root, svg);
}

void SVGParser::parseSVG(XMLElement* xml, SVG* svg)
{
	XMLElement* elem = xml->FirstChildElement();
	while (elem)
	{
		string elementType(elem->Value());
		
		if (elementType == "line") 
		{
			Line* line = new Line();
			parseLine(elem, line);
			svg->elements.push_back(line);
		}
		else if (elementType == "polygon")
		{
			Polygon *polygon = new Polygon();
			parsePolygon(elem, polygon);
			svg->elements.push_back(polygon);
		} 
		else if (elementType == "g")
		{
			Group *group = new Group();
			parseGroup(elem, group);
			svg->elements.push_back(group);
		}
		else if (elementType == "rect")
		{
			float w = elem->FloatAttribute("width");
			float h = elem->FloatAttribute("height");
			// treat zero-size rectangles as points
			if (w == 0 && h == 0) {
				Point* point = new Point();
				parsePoint(elem, point);
				svg->elements.push_back(point);
			}
			else {
				Rect* rect = new Rect();
				parseRect(elem, rect);
				svg->elements.push_back(rect);
			}
		}
		else if (elementType == "image")
		{
			Image *image = new Image();
			parseImage(elem, image);
			svg->elements.push_back(image);
		}

		elem = elem->NextSiblingElement();
	}
}


void SVGParser::parseElement(XMLElement* xml, SVGElement* element)
{
	//Parse style
	Style* style = &element->style;
	//fillColor
	const char* fill = xml->Attribute("fill");
	if (fill) style->fillColor = Color::fromHex(fill);
	const char* fill_opacity = xml->Attribute("fill-opacity");
	if (fill_opacity) style->fillColor.a = atof(fill_opacity);

	//Stroke
	const char* stroke = xml->Attribute("stroke");
	const char* stroke_opacity = xml->Attribute("stroke-opacity");
	if (stroke) {
		style->strokeColor = Color::fromHex(stroke);
		if (stroke_opacity) style->strokeColor.a = atof(stroke_opacity);
	}
	else {
		style->strokeColor = Color::Black;
		style->strokeColor.a = 0;
	}
	xml->QueryFloatAttribute("stroke-width", &style->strokeWidth);
	xml->QueryFloatAttribute("stroke-miterlimit", &style->miterLimit);

	//Parse transformation
	const char* trans = xml->Attribute("transform");
	if (trans) {

		// NOTE (sky):
		// This implements the SVG transformation specification. All the SVG 
		// transformations are supported as documented in the link below:
		// https://developer.mozilla.org/en-US/docs/Web/SVG/Attribute/transform
		// consolidate transformation

		Matrix3x3 transform = Matrix3x3::identity();

		string trans_str = trans; size_t paren_l, paren_r;
		while (trans_str.find_first_of('(') != string::npos) {

			paren_l = trans_str.find_first_of('(');
			paren_r = trans_str.find_first_of(')');

			string type = trans_str.substr(0, paren_l);
			string data = trans_str.substr(paren_l + 1, paren_r - paren_l - 1);

			if (type == "matrix") {

				string matrix_str = data;
				replace(matrix_str.begin(), matrix_str.end(), ',', ' ');

				stringstream ss(matrix_str);
				float a; float b; float c; float d; float e; float f;
				ss >> a; ss >> b; ss >> c; ss >> d; ss >> e; ss >> f;

				Matrix3x3 m;
				m(0, 0) = a; m(0, 1) = c; m(0, 2) = e;
				m(1, 0) = b; m(1, 1) = d; m(1, 2) = f;
				m(2, 0) = 0; m(2, 1) = 0; m(2, 2) = 1;
				transform = transform * m;

			}
			else if (type == "translate") {

				stringstream ss(data);
				float x; if (!(ss >> x)) x = 0;
				float y; if (!(ss >> y)) y = 0;

				Matrix3x3 m = Matrix3x3::identity();

				m(0, 2) = x;
				m(1, 2) = y;

				transform = transform * m;

			}
			else if (type == "scale") {

				stringstream ss(data);
				float x; if (!(ss >> x)) x = 1;
				float y; if (!(ss >> y)) y = 1;

				Matrix3x3 m = Matrix3x3::identity();

				m(0, 0) = x;
				m(1, 1) = y;

				transform = transform * m;

			}
			else if (type == "rotate") {

				stringstream ss(data);
				float a; if (!(ss >> a)) a = 0;
				float x; if (!(ss >> x)) x = 0;
				float y; if (!(ss >> y)) y = 0;

				if (x != 0 || y != 0) {

					Matrix3x3 m = Matrix3x3::identity();

					m(0, 0) = cos(a*PI / 180.0f); m(0, 1) = -sin(a*PI / 180.0f);
					m(1, 0) = sin(a*PI / 180.0f); m(1, 1) = cos(a*PI / 180.0f);

					m(0, 2) = -x * cos(a*PI / 180.0f) + y * sin(a*PI / 180.0f) + x;
					m(1, 2) = -x * sin(a*PI / 180.0f) - y * cos(a*PI / 180.0f) + y;

					transform = transform * m;

				}
				else {

					Matrix3x3 m = Matrix3x3::identity();

					m(0, 0) = cos(a*PI / 180.0f); m(0, 1) = -sin(a*PI / 180.0f);
					m(1, 0) = sin(a*PI / 180.0f); m(1, 1) = cos(a*PI / 180.0f);

					transform = transform * m;
				}

			}
			else if (type == "skewX") {

				stringstream ss(data);
				float a; ss >> a;

				Matrix3x3 m = Matrix3x3::identity();

				m(0, 1) = tan(a*PI / 180.0f);

				transform = transform * m;

			}
			else if (type == "skewY") {

				stringstream ss(data);
				float a; ss >> a;

				Matrix3x3 m = Matrix3x3::identity();

				m(1, 0) = tan(a*PI / 180.0f);

				transform = transform * m;

			}
			else {
				cerr << "unknown transformation type: " << type << endl;
			}

			size_t end = paren_r + 2;
			trans_str.erase(0, end);
		}

		element->transform = transform;
	}

}

void SVGParser::parsePoint(XMLElement* xml, Point* point) {
	parseElement(xml, point);
	point->position = Vector2D(xml->FloatAttribute("x"),
		xml->FloatAttribute("y"));
}

void SVGParser::parseLine(XMLElement* xml, Line*  line)
{
	parseElement(xml, line);

	line->from = Vector2D(xml->FloatAttribute("x1"), xml->FloatAttribute("y1"));
	line->to   = Vector2D(xml->FloatAttribute("x2"), xml->FloatAttribute("y2"));
}

void SVGParser::parsePolygon(XMLElement* xml, Polygon*  polygon)
{
	parseElement(xml, polygon);

	stringstream points(xml->Attribute("points"));
	float x, y;
	char c;
	while (points >> x >> c >> y) {
		polygon->points.push_back(Vector2D(x, y));
	}
}

void SVGParser::parseGroup(XMLElement* xml, Group*  group)
{

	parseElement(xml, group);
	XMLElement* elem = xml->FirstChildElement();
	while (elem)
	{
		string elementType(elem->Value());

		if (elementType == "line")
		{
			Line* line = new Line();
			parseLine(elem, line);
			group->elements.push_back(line);
		}
		else if (elementType == "polygon")
		{
			Polygon *polygon = new Polygon();
			parsePolygon(elem, polygon);
			group->elements.push_back(polygon);
		}
		else if (elementType == "g")
		{
			Group * group = new Group();
			parseGroup(elem, group);
			group->elements.push_back(group);
		}
		else if (elementType == "rect")
		{
			float w = elem->FloatAttribute("width");
			float h = elem->FloatAttribute("height");
			// treat zero-size rectangles as points
			if (w == 0 && h == 0) {
				Point* point = new Point();
				parsePoint(elem, point);
				group->elements.push_back(point);
			}
			else {
				Rect* rect = new Rect();
				parseRect(elem, rect);
				group->elements.push_back(rect);
			}
		}

		elem = elem->NextSiblingElement();
	}
}

void SVGParser::parseRect(XMLElement* xml, Rect* rect)
{
	parseElement(xml, rect);

	rect->position = Vector2D(xml->FloatAttribute("x"),
		xml->FloatAttribute("y"));
	rect->dimension = Vector2D(xml->FloatAttribute("width"),
		xml->FloatAttribute("height"));
}

void SVGParser::parseImage(XMLElement* xml, Image* image)
{
	parseElement(xml, image);

	image->position = Vector2D(	xml->FloatAttribute("x"),
								xml->FloatAttribute("y"));
	image->dimension = Vector2D(xml->FloatAttribute("width"),
								xml->FloatAttribute("height"));

	const char* file = xml->Attribute("xlink:href");
	PNG png; PNGParser::load(file, png);
	
	MipLevel mip0;
	mip0.width = png.width;
	mip0.height = png.height;
	mip0.texels = png.pixels;

	image->tex.width = mip0.width;
	image->tex.height = mip0.height;
	image->tex.generateMips(mip0);
	return;
}

void SVGParser::save(const char* filename, const SVG* svg)
{
	
}

Group::~Group()
{
	for each (SVGElement *pe in elements)
	{
		delete pe;
	}
}

SVG::~SVG()
{
	for each (SVGElement *pe in elements)
	{
		delete pe;
	}
}

}