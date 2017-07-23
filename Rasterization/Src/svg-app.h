#ifndef SVG_APP_H
#define SVG_APP_H

#include "application.h"

#include <vector>

namespace CGCore
{

struct SVG;
class  SoftwareRenderer;


class AppSVG : public Application
{

public:

	AppSVG(const char* filename);
	~AppSVG();

	void init() override;

	void render() override;

	void resize(size_t w, size_t h) override;

	void mouse_event(int key, int event, unsigned char mods) override;

	void cursor_event(float x, float y) override;

	void keyboard_event(int key, int event, unsigned char mods) override;

	void scroll_event(float offset_x, float offset_y) override;

private:

	std::vector<SVG*>	m_svgTabs;
	size_t				m_curTab;

	bool				m_drawZoom;

	bool				m_leftMouseClicked;
	bool				m_rightMouseClicked;

	size_t				m_widowWidth;
	size_t				m_widowHeight;

	SoftwareRenderer   *m_renderer;
};

}

#endif