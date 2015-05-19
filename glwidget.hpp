#ifndef __GLWIDGET__
#define __GLWIDGET__

#include <rendertexture.hpp>
#include <utils/qtplainwidget.hpp>
#include <cv.h>
#include <highgui.h>
#include "cameraexception.hpp"

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

    ///Max camera search index
    int maxCamIndex;

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
        glViewport(0, 0, this->width(), this->height());

        updateGL();
    }

signals:

public slots:

private:
    //VARIABLES
    cv::VideoCapture* camera;
    
    cv::Mat frame;
    
    int nextCameraIndex;

    /// Render image effect (simply renders a texture)
    Effects::RenderTexture rendertexture;

    /// Texture to hold input image
    Texture frameTexture;

    ///Frame Texture's index:
    //Gluint frameTextureID;

    //FUNCTIONS
    int findWorkingCam(cv::VideoCapture** targetCamera, int starter);

protected:

    //events go in here    

};

#endif // GLWIDGET
