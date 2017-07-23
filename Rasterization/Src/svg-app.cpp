#include "svg-app.h"
#include "console.h"
#include "misc.h"

#include "software-renderer.h"
#include "svg.h"

#include <cstdio>

using namespace std;

namespace CGCore
{

AppSVG::AppSVG(const char* filename) : 
m_curTab(0), m_drawZoom(false), m_leftMouseClicked(false), m_rightMouseClicked(false)
{
	SVG * svg = new SVG();
	SVGParser::load(filename, svg);

	m_svgTabs.push_back(svg);
}

AppSVG::~AppSVG()
{
	for each (SVG* psvg in m_svgTabs)
	{
		delete psvg;
	}
	delete m_renderer;
}

void AppSVG::init()
{
	m_renderer = new SoftwareRenderer();
	//Set the SVG to render
	m_renderer->setSVG (m_svgTabs[m_curTab]);
}

void AppSVG::render()
{
	m_renderer->displayPixels();

	if (m_drawZoom)
		m_renderer->displayZoom();
}

void AppSVG::resize(size_t w, size_t h)
{
	m_renderer->resize(w, h);
}

void AppSVG::cursor_event(float x, float y)
{

	if (m_rightMouseClicked)
	{
		float dx = (x - m_renderer->m_cursorX) / m_renderer->m_width * m_svgTabs[m_curTab]->width;
		float dy = (y - m_renderer->m_cursorY) / m_renderer->m_height * m_svgTabs[m_curTab]->height;
		m_renderer->updateViewport( -dx, -dy, 1);
	}

	m_renderer->m_cursorX = x;
	m_renderer->m_cursorY = y;
}

void AppSVG::mouse_event(int key, int event, unsigned char mods)
{
	if (key == MOUSE_RIGHT)
	{
		if (event == EVENT_PRESS)
			m_rightMouseClicked = true;
		if (event == EVENT_RELEASE)
			m_rightMouseClicked = false;
	}
}

void AppSVG::scroll_event(float offset_x, float offset_y)
{
	if (offset_x || offset_y) {
		float scale = 1 + 0.05 * (offset_x + offset_y);
		scale = std::min(1.5f, std::max(0.5f, scale));
		m_renderer->updateViewport(0, 0, scale);
	}
}

void AppSVG::keyboard_event(int key, int event, unsigned char mods)
{

	if (event != EVENT_PRESS)
		return;

	if (key >= '0' && key <= '9' && key - '0' < m_svgTabs.size())
	{
		m_curTab = key - '0';
		m_renderer->setSVG(m_svgTabs[0]);
		m_renderer->redraw();
		return;
	}

	switch (key)
	{
	case 'Z':
		m_drawZoom = !m_drawZoom;
		break;
	default:
		break;
	}

}

}