#include <iostream>
#include <tucano.hpp>
#include <thread>
#include <trackerwindow.hpp>
#include <opencvframeserver.hpp>
#include "GLFW/glfw3.h"

//#define WINDOW_WIDTH 640
//#define WINDOW_HEIGHT 480

TrackerWindow *trackerWindow;
FrameServer frameServer;
int WINDOW_WIDTH = 100;
int WINDOW_HEIGHT = 100;

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

void frameGrabber(GLFWwindow* main_window)
{
	std::cout<<"Thread running!"<<std::endl;
	while (!glfwWindowShouldClose(main_window))
	{
		if(frameServer.captureFrame()){
			std::this_thread::sleep_for (std::chrono::seconds(1));
		}
	}
	std::cout<<"Thread exiting..."<<std::endl;
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

   	frameServer.initialize();

	Eigen::Vector2i dimensions = frameServer.getSize();
	WINDOW_WIDTH = dimensions[0];
	WINDOW_HEIGHT = dimensions[1];

   	glfwSetWindowSize(main_window, WINDOW_WIDTH, WINDOW_HEIGHT);

	initialize();

	//while(!frameServer.captureFrame()){}
	//std::cout<<"Frame captured"<<std::endl;
	//while(!frameServer.firstFrame()){}
	//std::cout<<"First frame"<<std::endl;

	/**

	std::thread t1(frameGrabber,main_window);

	//first render
	while(!frameServer.firstFrame()){
		//std::cout<<"!";
	}
	std::cout<<std::endl<<"Popped frame!"<<std::endl;
	glfwMakeContextCurrent(main_window);
	trackerWindow->setAndPaint(frameServer.getFramePointer());
	glfwSwapBuffers(main_window);

	glfwPollEvents();

	//render/processing loop
	while (!glfwWindowShouldClose(main_window))
	{
		glfwMakeContextCurrent(main_window);
		frameServer.serveFrame();
		trackerWindow->paintGL(frameServer.getFramePointer());
		glfwSwapBuffers(main_window);

		glfwPollEvents();
	}

	/**/
	glfwDestroyWindow(main_window);
	glfwTerminate();
	return 0;
}
