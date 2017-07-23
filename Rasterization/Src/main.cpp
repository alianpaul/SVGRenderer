#include "viewer.h"
#include "svg-app.h"

using namespace CGCore;

int main()
{
	AppSVG app("05_lion.svg");
	//AppSVG app("04_sun.svg");
	//AppSVG app("02_cube.svg");
	//AppSVG app("test7.svg");

	//AppSVG app("test4.svg");

	Viewer viewer;
	viewer.setApplication(&app);
	viewer.init("SVG-alianpaul", 640, 640);
	viewer.start();
}