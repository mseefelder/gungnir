#ifndef __GLWIDGET__
#define __GLWIDGET__

#include <rendertexture.hpp>
#include <utils/qtplainwidget.hpp>
#include <opencv/cv.hpp>
#include <highgui.h>
#include "cameraexception.hpp"
#include "drawrectangle.hpp"
#include "meanshiftshader.hpp"

using namespace std;

class GLWidget : public Tucano::QtPlainWidget
{
    Q_OBJECT

protected:
    /// VARIABLES
    
    /// Is initialized
    bool initd;

    // Rendering quad
    Tucano::Mesh *quad;

    /// Path where shaders are stored
    string shaders_dir;

    /// FUNCTIONS

public:

    /// VARIABLES
    

    /// FUNCTIONS
    explicit GLWidget(QWidget *parent);
    ~GLWidget();
    
    /**
     * @brief Initializes the shader effect
     */
    void initialize();

    /**
     * Repaints screen buffer.
     */
    virtual void paintGL();

    void resizeGL(int w, int h)
    {
        resize(w, h);
        glViewport(0, 0, this->width(), this->height());
        viewport = Eigen::Vector2i(this->width(), this->height());

        updateGL();
    }

signals:

public slots:

private:
    //VARIABLES

    /// Render image effect (simply renders a texture)
    Effects::RenderTexture rendertexture;

    /// Render image effect (simply renders a texture)
    Effects::drawRectangle rect;

    Effects::Meanshift meanShift;

    /// Texture to hold input image
    Tucano::Texture* frameTexture;

    /// Region of interest parameters
    Eigen::Vector2i ROIcorner; //where the rectangle begins
    Eigen::Vector2i ROIspread; //how far does it spread in each axis
    Eigen::Vector2i viewport;

    //FUNCTIONS
    int findWorkingCam(cv::VideoCapture** targetCamera, int starter);

protected:

    //events go in here

};

#endif // GLWIDGET
