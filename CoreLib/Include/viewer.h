#ifndef VIEWER_H
#define VIEWER_H

#include "corefwd.h"

namespace CGCore {

class Viewer {

public:

	~Viewer();

	void init(const char* title, size_t width, size_t height);
	void setApplication(Application *application);
	void start();

private:
	void update();

};

}

#endif