#ifndef APPLICATION_H
#define APPLICATION_H

namespace CGCore
{

class Application
{
public:
	virtual ~Application() {}

	/**
	* Initialize the application.
	* A application may have some initialization work to do before it is ready
	* to be used. The viewer will call the init function before using the
	* application in drawing.
	*/
	virtual void init() = 0;

	/**
	* Draw content.
	* Applications are free to define their own routines for drawing to the
	* context. The viewer calls this function on every frame update.
	*/
	virtual void render() = 0;

	/**
	* Respond to buffer resize.
	* The viewer will inform the application of a context resize by calling
	* this function. The application has complete freedom to handle resizing,
	* and a good application should handle resizes properly.
	* \param w The new width of the context
	* \param h The new height of the context
	*/
	virtual void resize(size_t w, size_t h) = 0;

	/**
	* Respond to cursor events.
	* The viewer itself does not really care about the cursor but it will take
	* the GLFW cursor events and forward the ones that matter to  the
	* application.
	* The arguments are defined in screen space coordinates ( (0,0) at top
	* left corner of the window and (w,h) at the bottom right corner.
	* \param x the x coordinate of the cursor
	* \param y the y coordinate of the cursor
	*/
	virtual void cursor_event(float x, float y) {}

	/**
	* Respond to zoom event.
	* Like cursor events, the viewer itself does not care about the mouse wheel
	* either, but it will take the GLFW wheel events and forward them directly
	* to the application.
	* \param offset_x Scroll offset in x direction
	* \param offset_y Scroll offset in y direction
	*/
	virtual void scroll_event(float offset_x, float offset_y) {}

	/**
	* Respond to mouse click event.
	* The viewer will always forward mouse click events to the application.
	* \param key The key that spawned the event. The mapping between the
	*        key values and the mouse buttons are given by the macros defined
	*        at the top of this file.
	* \param event The type of event. Possible values are 0, 1 and 2, which
	*        corresponds to the events defined in macros (see misc.h).
	* \param mods if any modifier keys are held down at the time of the event
	*        modifiers are defined in macros.
	*/
	virtual void mouse_event(int key, int event, unsigned char mods) {}

	/**
	* Respond to keyboard event.
	* The viewer will always forward mouse key events to the application.
	* \param key The key that spawned the event. ASCII numbers are used for
	*        letter characters. Non-letter keys are selectively supported
	*        and are defined in macros.
	* \param event The type of event. Possible values are 0, 1 and 2, which
	*        corresponds to the events defined in macros (see misc.h).
	* \param mods if any modifier keys are held down at the time of the event
	*        modifiers are defined in macros.
	*/
	virtual void keyboard_event(int key, int event, unsigned char mods) {}

	/**
	* Run application in HDPI mode.
	*/
	void use_hdpi_reneder_target() { use_hdpi = true; }

protected:

	bool use_hdpi; ///< if the render target is using HIDPI
};

}

#endif