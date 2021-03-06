#ifndef __GLWIDGET__
#define __GLWIDGET__

#include <GL/glew.h>
#include <texture.hpp>

#include <cameraexception.hpp>
#include <Eigen/Dense>

using namespace std;

class FrameServer
{

protected:
    /// VARIABLES
    
    /// Is initialized
    bool initd;

    /// Max camera search index
    int maxCamIndex;

    /// Camera dimensions
    Eigen::Vector2i size;

public:

    FrameServer()
    {
        nextCameraIndex = -1;
        maxCamIndex = 50;
        size = Eigen::Vector2i(0,0);
    }

    ~FrameServer()
    {
        delete camera;
    }

    /**
     * @brief Initializes the shader effect
     */
    void initialize()
    {
        try
        {
            size = findWorkingCam(&camera, nextCameraIndex);
        }
        catch (exception& e)
        {
            throw;
        }    
    }

    bool captureFrame()
    {
        camera->read(temp);
        return !temp.empty();
    }

    bool captureFlipped()
    {
        camera->read(temp);
        cv::flip(temp, temp, 0);
        return !temp.empty();
    }

    bool serveFrame()//Tucano::Texture* texture)
    {
        if(captureFlipped())
        {
            frame = temp;
            return true;
        } 
        return false;
    }

    bool firstFrame()
    {
        if(captureFlipped())
        {
            frame = temp;
            return true;
        } 
        return false;
    }

    Eigen::Vector2i getSize()
    {
        return size;
    }

    cv::Mat* getFramePointer()
    {
        return &frame;
    }

private:
    ///camera in use
    cv::VideoCapture* camera;
    
    ///frame where we read the camera frames to
    cv::Mat frame;
    cv::Mat temp;
    
    ///if we want to get another camera, call findWorkingCam starting from here
    int nextCameraIndex;

    //FUNCTIONS
    Eigen::Vector2i findWorkingCam(cv::VideoCapture** targetCamera, int &starter)
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
                imwrite("findWorkingCam.png", scratch);
                //Store next index
                starter = targetCameraIndex++;
                //Return camera dimensions
                return Eigen::Vector2i (scratch.cols, scratch.rows);
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
        return Eigen::Vector2i (0,0);
    }

};

#endif // GLWIDGET
