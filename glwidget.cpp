#include "glwidget.hpp"
#include <QDebug>

GLWidget::GLWidget(QWidget *parent) : Tucano::QtPlainWidget(parent)
{
	initd = false;
	markROI = true;
    regionDefined = false;
    qValueReady = false;
	camera = NULL;
    //frame = NULL;
    nextCameraIndex = 0;
    maxCamIndex = 50;

    ROIcorner = Eigen::Vector2i(0,0);
    ROIspread = Eigen::Vector2i(0,0);
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
    ///find and set camera to read from the first working webcam
    //try
    //{
    //    nextCameraIndex = findWorkingCam(&camera, nextCameraIndex);
    //}
    //catch (exception& e)
    //{
    //    throw;
    //}

    ///initialize frame container
    //frame = new cv::Mat;

    // the default is /shaders from your running dir
    string shaders_dir("../shaders/");

    rendertexture.setShadersDir(shaders_dir);
    rendertexture.initialize();
    rect.setShadersDir(shaders_dir);
    rect.initialize();
    meanShift.setShadersDir(shaders_dir);
    meanShift.initialize();

    //temporary
    std::cout<<"Reading frame..."<<std::endl;
    //*camera >> frame;
    camera->read(frame);
    std::cout<<"Frame has been read! Flipping..."<<std::endl;
	cv::flip(frame, frame, 0);
	std::cout<<"Frame has been flipped! Converting to texture..."<<std::endl;

	//frameTextureID = frameTexture.create(GL_TEXTURE_2D, GL_RGB, frame.cols, frame.rows, GL_BGR, GL_UNSIGNED_BYTE, frame.ptr());
    frameTexture = new Tucano::Texture();
	frameTexture->create(GL_TEXTURE_2D, GL_RGB, frame.cols, frame.rows, GL_BGR, GL_UNSIGNED_BYTE, frame.ptr());
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
			std::cout<<"Found camera! \n w: "<<scratch.cols<<"; h: "<<scratch.rows<<std::endl;
            this->resizeGL(scratch.cols, scratch.rows);
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
	frameTexture->create(GL_TEXTURE_2D, GL_RGB, frame.cols, frame.rows, GL_BGR, GL_UNSIGNED_BYTE, frame.ptr());
	

	// renders the given image, not that we are setting a fixed viewport that follows the widgets size
    // so it may not be scaled correctly with the image's size (just to keep the example simple)
    Eigen::Vector2i viewport (this->width(), this->height());
    rendertexture.renderTexture(*frameTexture, viewport);//debug commented

    if(regionDefined){
        if(!qValueReady)
        {
            meanShift.setRegionDimensionsAndCenter(viewport, ROIcorner, ROIspread);
            meanShift.histogramQ(frameTexture);
            qValueReady = true;
        }
        else
        {
            // //rendertexture.renderTexture(*(meanShift.roiPointer()), meanShift.viewport());//debug
            meanShift.histogramP(frameTexture);
            // //std::cout<<"corner & spread: before: \nc:"<<ROIcorner<<"\n & \ns:"<<ROIspread<<std::endl;
            meanShift.meanshift(&ROIcorner, &ROIspread);
            // //std::cout<<"corner & spread: after: \nc:"<<ROIcorner<<"\n & \ns:"<<ROIspread<<std::endl;
            //regionDefined = false;
            meanShift.histogramP(frameTexture);
            Eigen::Vector2f mS = meanShift.meanshift(&ROIcorner, &ROIspread);
            int iter = 1;
            while (mS.norm() > 1 && iter < 30){
                meanShift.histogramP(frameTexture);
                mS = meanShift.meanshift(&ROIcorner, &ROIspread);
                iter++;
            }
            //std::cout<<" "<<iter;
        }
    }

    //Eigen::Vector2f firstCorner (0.3,0.3);
    //Eigen::Vector2f spread (0.2,0.2);
    //rendertexture.renderTexture(*frameTexture, viewport);//debug
    rect.renderTexture(viewport, ROIcorner, ROIspread);
    

    update();
}

/**
* @brief Callback for mouse press event.
*
* The mouse press starts a rotation or a translation if Shift is pressed.
* @param event The mouse event that triggered the callback.
*/
void GLWidget::mousePressEvent (QMouseEvent * event)
{
    setFocus ();
    //markROI = true;
    
    //Eigen::Vector2f screen_pos (
    //    ((2*((float)event->x()/this->width()))-1.0)
    //    , 
    //    ((-2*((float)event->y()/this->height()))+1.0)
    //    );
    Eigen::Vector2i screen_pos (
        (event->x())
        , 
        abs(event->y()-this->height())
        );

    if (event->modifiers() & Qt::ShiftModifier){}
    else
    {
        if (event->button() == Qt::LeftButton)
        {
            if(markROI) ROIcorner = screen_pos;
        }
        if (event->button() == Qt::RightButton)
        {

        }
    }
    //std::cout<<ROIcorner[0]<<","<<ROIcorner[1]<<" & "<<ROIspread[0]<<","<<ROIspread[1]<<std::endl;
}

/**
 * @brief Callback for mouse move event.
 *
 * If rotating or translating, this method updates the trackball position.
 * @param event The mouse event that triggered the callback.
 */
void GLWidget::mouseMoveEvent (QMouseEvent * event)
{
    //Eigen::Vector2f screen_pos (
    //    ((2*((float)event->x()/this->width()))-1.0)
    //    , 
    //    ((-2*((float)event->y()/this->height()))+1.0)
    //    );
    Eigen::Vector2i screen_pos (
        (event->x())
        , 
        abs(event->y()-this->height())
        );

    if (event->modifiers() & Qt::ShiftModifier && event->buttons() & Qt::LeftButton){}
    else
    {
        if (event->buttons() & Qt::LeftButton)
        {
            if(markROI) ROIspread = screen_pos;
        }
        if (event->buttons() & Qt::RightButton)
        {
        }
    }
}

/**
 * @brief Callback for mouse release event.
 *
 * Stops rotation or translation.
 * @param event The mouse event that triggered the callback.
 */
void GLWidget::mouseReleaseEvent (QMouseEvent * event)
{
    //Eigen::Vector2f screen_pos (
    //    ((2*((float)event->x()/this->width()))-1.0)
    //    , 
    //    ((-2*((float)event->y()/this->height()))+1.0)
    //    );
    Eigen::Vector2i screen_pos (
        (event->x())
        , 
        abs(event->y()-this->height())
        );

    if (event->button() == Qt::LeftButton)
    {
        if(markROI) 
        {
            ROIspread = screen_pos;
            regionDefined = true;
            qValueReady = false;
            markROI = false;
        }
    }
    if (event->button() == Qt::RightButton)
    {
        
    }
    std::cout<<ROIcorner[0]<<","<<ROIcorner[1]<<" & "<<ROIspread[0]<<","<<ROIspread[1]<<std::endl;
}

/*
http://www.stat.wisc.edu/~mchung/teaching/MIA/reading/diffusion.gaussian.kernel.pdf.pdf
*/