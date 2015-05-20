#include "glwidget.hpp"
#include <QDebug>

GLWidget::GLWidget(QWidget *parent) : Tucano::QtPlainWidget(parent)
{
	initd = false;
	markROI = true;
	camera = NULL;
    //frame = NULL;
    nextCameraIndex = 0;
    maxCamIndex = 10;
}

GLWidget::~GLWidget()
{

}

void GLWidget::initialize (void)
{
	/// resize OpenGL viewport to current size
    glViewport(0, 0, this->width(), this->height());

    ///find and set camera to read from the first working webcam
    try
    {
    	nextCameraIndex = findWorkingCam(&camera, nextCameraIndex);
    }
    catch (exception& e)
    {
    	throw;
    }

    ///initialize frame container
    //frame = new cv::Mat;

    // the default is /shaders from your running dir
    string shaders_dir("./shaders/");

    rendertexture.setShadersDir(shaders_dir);
    rendertexture.initialize();
    rect.setShadersDir(shaders_dir);
    rect.initialize();

    //temporary
    std::cout<<"Reading frame..."<<std::endl;
    //*camera >> frame;
    camera->read(frame);
    std::cout<<"Frame has been read! Flipping..."<<std::endl;
	cv::flip(frame, frame, 0);
	std::cout<<"Frame has been flipped! Converting to texture..."<<std::endl;

	//frameTextureID = frameTexture.create(GL_TEXTURE_2D, GL_RGB, frame.cols, frame.rows, GL_BGR, GL_UNSIGNED_BYTE, frame.ptr());
	frameTexture.create(GL_TEXTURE_2D, GL_RGB, frame.cols, frame.rows, GL_BGR, GL_UNSIGNED_BYTE, frame.ptr());
	std::cout<<"Frame is now on the GPU!"<<std::endl;
    //temporary


    /// set this widget as initialized
    initd = true;
}

int GLWidget::findWorkingCam (cv::VideoCapture** targetCamera, int starter)
{
	//start searching for camera at given index
	int targetCameraIndex = starter;

	while(targetCameraIndex < maxCamIndex)
	{
		//reset camera with new index
		if(*targetCamera)
		{
			(*targetCamera)->release();
			delete camera;	
		}
		(*targetCamera) = new cv::VideoCapture(targetCameraIndex);

		//try to read a frame. If ->read() succeeds, it returns true and
		//the while loop is broken. If not, false is returned, continuing the loop
		cv::Mat scratch; 
		if ((*targetCamera)->read(scratch) != false)
		{
			std::cout<<"Found camera!"<<std::endl;
			break;
		}

		//Increase camera index to continue search
		targetCameraIndex++;
	}
	//throw error if no camera is found
	if (targetCameraIndex == maxCamIndex)
	{
		cameraException camex;
		throw camex;
	}

	//returns the next index
	return targetCameraIndex++;
}

void GLWidget::paintGL (void)
{
	if(!initd)
        return;

	makeCurrent();

	glClearColor(1.0, 1.0, 1.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);


    
    //*camera >> frame;
    camera->read(frame);
	cv::flip(frame, frame, 0);
	frameTexture.create(GL_TEXTURE_2D, GL_RGB, frame.cols, frame.rows, GL_BGR, GL_UNSIGNED_BYTE, frame.ptr());
	

	// renders the given image, not that we are setting a fixed viewport that follows the widgets size
    // so it may not be scaled correctly with the image's size (just to keep the example simple)
    Eigen::Vector2i viewport (this->width(), this->height());
    rendertexture.renderTexture(frameTexture, viewport);

    Eigen::Vector2f firstCorner (0.3,0.3);
    Eigen::Vector2f spread (0.2,0.2);
    rect.renderTexture(viewport, firstCorner, spread);

    update();
}