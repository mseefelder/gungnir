#ifndef __TRACKERWINDOW__
#define __TRACKERWINDOW__

#include <GL/glew.h>

#include "singlepass.hpp"
#include "drawrectangle.hpp"
#include "neuralDaniel.hpp"

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
    	markROI = false;
    	ROIDefined = false;
    	qValueReady = false;
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

	    // set effects
	    rendertexture.setShadersDir(shaders_dir);
	    rendertexture.initialize();
	    rect.setShadersDir(shaders_dir);
	    rect.initialize();
	    meanShift.setShadersDir(shaders_dir);
	    meanShift.initialize();

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
	    double d = 1.0;
	    rendertexture.render(*frameTexture, viewport, d);

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

	    // NEURAL
	    	cv::Scalar frameMean = cv::mean(*frame);
	    	//double frameNorm = (max(max(frameMean[0],frameMean[1]),frameMean[2])+min(min(frameMean[0],frameMean[1]),frameMean[2]))/2.0;
			//double frameNorm = cv::norm(frameMean);
			double frameNorm = getThresh(frameMean);

		frameTexture->update(frame->ptr());

		// renders the given image, not that we are setting a fixed viewport that follows the widgets size
	    // so it may not be scaled correctly with the image's size (just to keep the example simple)
	    Eigen::Vector2i viewport (viewportSize[0], viewportSize[1]);

	    //if(!qValueReady)
	    	rendertexture.render(*frameTexture, viewport, frameNorm);

	    if(ROIDefined)
	    {
	        if(!qValueReady)
	        {
	            meanShift.firstFrame(viewport, ROIcorner, ROIspread, frameTexture, frameNorm);
	            qValueReady = true;
	        }
	        else
	        {
	            meanShift.track(frameTexture, &ROIcorner, &ROIspread, 100, frameNorm);
	        }
	    }

	    rect.renderTexture(viewport, ROIcorner, ROIspread);
    }

	//Effects::Tracker* getEffect(void)
	//{
	//	return &tracker;
	//}

	Tucano::Texture* texPointer()
	{
		return frameTexture;
	}

	void startROI(Eigen::Vector2i &corner)
	{
		markROI = true;
		ROIDefined = false;
		ROIcorner = corner;
	}

	void updateROI(Eigen::Vector2i &current)
	{
		if(markROI)
			ROIspread = current;
	}

	void endROI(Eigen::Vector2i &spread)
	{
		if (markROI)
		{
			ROIspread = spread;
			markROI = false;
			ROIDefined = true;
			qValueReady = false;
		}
	}

	double getThresh(cv::Scalar meanPixel)
	{
		int max, min;
		double result;

		max = (meanPixel[0]>meanPixel[1])?0:1;
		max = (meanPixel[max]>meanPixel[2])?max:2;

		min = (meanPixel[0]<meanPixel[1])?0:1;
		min = (meanPixel[max]<meanPixel[2])?max:2;

		switch(max)
		{
			//Blue is max
			case 0:
				result = 4.0 + (meanPixel[2]-meanPixel[1])/(meanPixel[max]-meanPixel[min]);
				result *= 60.0;
				if (result < 0.0)
				{
					result += 360.0;
				}
				result = result/360.0;
				break;

			//Green is max
			case 1:
				result = 2.0 + (meanPixel[0]-meanPixel[2])/(meanPixel[max]-meanPixel[min]);
				result *= 60.0;
				if (result < 0.0)
				{
					result += 360.0;
				}
				result = result/360.0;
				break;

			//Red is max
			case 2:
				result = (meanPixel[1]-meanPixel[0])/(meanPixel[max]-meanPixel[min]);
				result *= 60.0;
				if (result < 0.0)
				{
					result += 360.0;
				}
				result = result/360.0;
				break;
		}

		return result;
	}

private:

	// A simple phong shader for rendering meshes
    //Effects::Tracker tracker;

	/// Render image effect (simply renders a texture)
    SinglePass rendertexture;

    /// Draw ROI as a rectangle
    Effects::drawRectangle rect;

    /// Process meanshift
    Meanshift meanShift;

    /// Texture to hold input image
    Tucano::Texture* frameTexture;

    /// Path where shaders are stored
    string shaders_dir;

    /// Viewport size
    Eigen::Vector2i viewportSize;

    /// where the rectangle begins
    Eigen::Vector2i ROIcorner;

    /// how far does it spread in each axis 
    Eigen::Vector2i ROIspread;

    /// has this been initialized?
	bool initd;

	/// are we marking the Region of Interest?
	bool markROI;

	/// is the Region of Interest Defined?
    bool ROIDefined;

    /// Have we calculated the values for the first frame?
    bool qValueReady;
};

#endif // TRACKERWINDOW
