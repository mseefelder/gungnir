#include <iostream>
#include <tucano.hpp>
#include <thread>
#include <trackerwindow.hpp>
#include <opencvframeserver.hpp>
#include "GLFW/glfw3.h"

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480

TrackerWindow *trackerWindow;
FrameServer frameServer;

void initialize (void)
{
	Misc::initGlew();
	trackerWindow = new TrackerWindow();	
	trackerWindow->initialize(WINDOW_WIDTH, WINDOW_HEIGHT);
	cout << "initialized" << endl;
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, 1);
}

static void mouseButtonCallback (GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	{
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);
		Eigen::Vector2f mouse(xpos, ypos);
	}
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
	{
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);
		Eigen::Vector2f mouse(xpos, ypos);
	}
}
static void cursorPosCallback(GLFWwindow* window, double xpos, double ypos)
{
	/*
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS)
	{
		trackerWindow->getCamera()->rotate(Eigen::Vector2f(xpos, ypos));
	}
	*/
}

void frameGrabber()
{
	while (!glfwWindowShouldClose(main_window))
	{
		frameServer.captureFrame();
	}
}


int main(int argc, char *argv[])
{
	GLFWwindow* main_window;

	if (!glfwInit()) 
	{
    	std::cerr << "Failed to init glfw" << std::endl;
		return 1;
	}

	main_window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Tracker", NULL, NULL);
	if (!main_window)
	{
		std::cerr << "Failed to create the GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(main_window);
	glfwSetKeyCallback(main_window, keyCallback);
	glfwSetMouseButtonCallback(main_window, mouseButtonCallback);
	glfwSetCursorPosCallback(main_window, cursorPosCallback);

	glfwSetInputMode(main_window, GLFW_STICKY_KEYS, true);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
   	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
   	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	initialize();
	frameServer.initialize();

	std::thread t1(frameGrabber);

	while (!glfwWindowShouldClose(main_window))
	{
		glfwMakeContextCurrent(main_window);
		frameServer.serveFrame(trackerWindow->texPointer());
		trackerWindow->paintGL();
		glfwSwapBuffers(main_window);

		glfwPollEvents();
	}

	glfwDestroyWindow(main_window);
	glfwTerminate();
	return 0;
}
