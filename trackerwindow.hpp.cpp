#ifndef __TRACKERWINDOW__
#define __TRACKERWINDOW__

#include <GL/glew.h>

#include <rendertexture.hpp>
#include <utils/qtplainwidget.hpp>
#include <opencv/cv.hpp>
#include <highgui.h>
#include "cameraexception.hpp"
#include "drawrectangle.hpp"
#include "meanshiftshader.hpp"

using namespace std;

class TrackerWindow 
{

public:

    explicit TrackerWindow(void);
    ~TrackerWindow();
    
    /**
     * @brief Initializes the shader effect
	 * @param width Window width in pixels
	 * @param height Window height in pixels
     */
    void initialize(int width, int height);

    /**
     * Repaints screen buffer.
     **/
    virtual void paintGL();

	Effects::TrackerWindowator* getEffect(void)
	{
		return &tesselator;
	}

	void changeMesh(void);

private:

	// A simple phong shader for rendering meshes
    Effects::TrackerWindowator tesselator;

	// A fly through camera
	//Tucano::Flycamera flycamera;
	Tucano::Trackballcamera flycamera;

	// Light represented as a camera
	Tucano::Camera light;
	
	// Meshes
	Mesh tri;
	Mesh quad;

	//Which mesh should be rendered
	bool renderquad;
};

#endif // TRACKERWINDOW
