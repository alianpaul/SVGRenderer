#include "viewer.h"
#include "console.h"
#include "application.h"

#include <GLFW\glfw3.h>

namespace CGCore{

// Window
static bool HDPI; //Is enabled on this machine
static GLFWwindow *window;
static size_t buffer_w;
static size_t buffer_h;
static Application *application;

void err_callback(int error, const char *description);
void resize_callback(GLFWwindow *window, int width, int height);
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
void cursor_callback(GLFWwindow *window, double xpos, double ypos);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
void mouse_button_callback(GLFWwindow *window, int button, int action, int mods);

Viewer::~Viewer()
{
	glfwDestroyWindow(window);
	glfwTerminate();

	//Destroy window
}

void Viewer::setApplication(Application* app)
{
	application = app;
}

void Viewer::init(const char* title, size_t width, size_t height)
{
	glfwSetErrorCallback(err_callback);
	if (!glfwInit())
	{
		out_err("Error: Could not initialize GLFW");
		exit(1);
	}

	buffer_h = height;
	buffer_w = width;
	window = glfwCreateWindow(width, height, title, NULL, NULL);

	if (!window) {
		out_err("Error: could not create window!");
		glfwTerminate();
		exit(1);
	}

	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	// framebuffer event callbacks
	glfwSetFramebufferSizeCallback(window, resize_callback);

	// key event callbacks
	glfwSetKeyCallback(window, key_callback);

	// cursor event callbacks
	glfwSetCursorPosCallback(window, cursor_callback);

	// wheel event callbacks
	glfwSetScrollCallback(window, scroll_callback);

	// mouse button callbacks
	glfwSetInputMode(window, GLFW_STICKY_MOUSE_BUTTONS, 1);
	glfwSetMouseButtonCallback(window, mouse_button_callback);

	// enable alpha blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// resize components to current window size, get DPI
	glfwGetFramebufferSize(window, (int *)&buffer_w, (int *)&buffer_h);
	if (buffer_w > width)
		HDPI = true;

	// initialize application
	if (!application)
		out_err("Error: Application not set");
	else
		application->init();

	resize_callback(window, buffer_w, buffer_h);
}

void Viewer::start()
{
	while (!glfwWindowShouldClose(window))
	{
		update();
	}
}

void Viewer::update()
{
	// clear frame
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Use application render
	if (application)
		application->render();

	// swap buffers
	glfwSwapBuffers(window);

	// poll events
	glfwPollEvents();
}

void err_callback(int error, const char *description)
{
	out_err("GLFW Error: " << description);
}

void resize_callback(GLFWwindow *window, int width, int height)
{

	// get framebuffer size
	int w, h;
	glfwGetFramebufferSize(window, &w, &h);

	// update buffer size
	buffer_w = w;
	buffer_h = h;
	glViewport(0, 0, buffer_w, buffer_h);

	if (application)
		application->resize(w, h);
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) 
{

	if (action == GLFW_PRESS) {
		if (key == GLFW_KEY_ESCAPE) {
			glfwSetWindowShouldClose(window, true);
		}
		else if (key == GLFW_KEY_GRAVE_ACCENT) {
			//showInfo = !showInfo;
		}
	}

	application->keyboard_event(key, action, mods);
}

void cursor_callback(GLFWwindow *window, double xpos, double ypos) 
{

	// forward pan event to application
	if (HDPI) {
		float cursor_x = 2 * xpos;
		float cursor_y = 2 * ypos;
		application->cursor_event(cursor_x, cursor_y);
	}
	else {
		float cursor_x = xpos;
		float cursor_y = ypos;
		application->cursor_event(cursor_x, cursor_y);
	}
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) 
{

	application->scroll_event(xoffset, yoffset);
}

void mouse_button_callback(GLFWwindow *window, int button, int action, int mods) 
{

	application->mouse_event(button, action, mods);
}

}