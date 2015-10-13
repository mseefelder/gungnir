#ifndef __TRACKERWINDOW__
#define __TRACKERWINDOW__

#include <GL/glew.h>

#include <rendertexture.hpp>
#include <opencv/cv.hpp>
#include <highgui.h>

using namespace std;

class TrackerWindow 
{

public:

    explicit TrackerWindow(void)
    {
    	frameTexture = NULL;
    	initd = false;
    }

    ~TrackerWindow()
    {
    	delete frameTexture;
    }
    
    /**
     * @brief Initializes the shader effect
	 * @param width Window width in pixels
	 * @param height Window height in pixels
     */
    void initialize(int width, int height)
    {
    	// the default is /shaders from your running dir
	    string shaders_dir("../shaders/");

	    //Set viewportSize vector
	    viewportSize = Eigen::Vector2i(width,height);

	    // set effect
	    rendertexture.setShadersDir(shaders_dir);
	    rendertexture.initialize();

	    //create texture
	    //frameTexture->create(GL_TEXTURE_2D, GL_RGB, viewportSize[0], viewportSize[1], GL_BGR, GL_UNSIGNED_BYTE, NULL);

	    /// set this widget as initialized
    	initd = true;
    }

    virtual bool setAndPaint(cv::Mat* frame)
    {
    	if(!initd)
	        return false;

	    imwrite("setAndPaint.png", *frame);

		glClearColor(1.0, 1.0, 1.0, 0.0);
	    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	    std::cout<<frame->cols<<" x "<<frame->rows<<std::endl;

	    frameTexture = new Tucano::Texture;
		
	    try
	    {
	    	frameTexture->create(GL_TEXTURE_2D, GL_RGB, frame->cols, frame->rows, GL_BGR, GL_UNSIGNED_BYTE, frame->ptr());
	    }
	    catch( exception& e)
	    {
	    	throw;
	    	std::cout<<"ERROR in setAndPaint: "<<std::endl;
	    	Tucano::Misc::errorCheckFunc(__FILE__, __LINE__);
	    }

		

		// renders the given image, not that we are setting a fixed viewport that follows the widgets size
	    // so it may not be scaled correctly with the image's size (just to keep the example simple)
	    Eigen::Vector2i viewport (viewportSize[0], viewportSize[1]);
	    rendertexture.renderTexture(*frameTexture, viewport);

	    return true;
    }

    /**
     * Repaints screen buffer.
     **/
    virtual void paintGL(cv::Mat* frame)
    {
	 	
		if(!initd)
	        return;

		glClearColor(1.0, 1.0, 1.0, 0.0);
	    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
		
		frameTexture->update(frame->ptr());

		// renders the given image, not that we are setting a fixed viewport that follows the widgets size
	    // so it may not be scaled correctly with the image's size (just to keep the example simple)
	    Eigen::Vector2i viewport (viewportSize[0], viewportSize[1]);
	    rendertexture.renderTexture(*frameTexture, viewport);  	
    }

	//Effects::Tracker* getEffect(void)
	//{
	//	return &tracker;
	//}

	Tucano::Texture* texPointer()
	{
		return frameTexture;
	}

private:

	// A simple phong shader for rendering meshes
    //Effects::Tracker tracker;

	/// Render image effect (simply renders a texture)
    Effects::RenderTexture rendertexture;

    /// Texture to hold input image
    Tucano::Texture* frameTexture;

    /// Path where shaders are stored
    string shaders_dir;

    /// Viewport size
    Eigen::Vector2i viewportSize;

	bool initd;
};

#endif // TRACKERWINDOW
