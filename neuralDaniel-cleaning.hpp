#ifndef __MEANSHIFT__
#define __MEANSHIFT___

#include <tucano.hpp>
#include <tucano.hpp>

#define NBINS 32
//#define DEBUGVIEW

/**
 * @brief Picks a 3D position from screen position
 */
class Meanshift : public Tucano::Effect
{

public:

    /**
     * @brief Default constructor.
     */
    Meanshift ()
    {
        widthIsMax = false;
        isSet = false;

        regionFbo = NULL;
        meanShiftFbo = NULL;
        lineFboP = NULL;
        lineFboQ = NULL;
        sumFbo = NULL;

        histogramRaw = NULL;

        qHistogramTexture = NULL;
        pHistogramTexture = NULL;
    }

    /**
     * @brief Default destructor
     */
    virtual ~Meanshift (void ) {}

    /**
     * @brief Load and initialize shaders, flags and framebuffer objects
     */
    virtual void initialize (void)
    {
        //shaders
        loadShader(pshader, "p");
        loadShader(histogramshader, "histogram");
        loadShader(meanshiftshader, "meanshift");
        loadShader(sumshader, "sum");

        //raw histogram array
        histogramRaw = new float[3*NBINS];

        //flags
        isSet = false;
        widthIsMax = false;

        //framebuffers
        regionFbo = NULL; //depends on region size //new Tucano::Framebuffer(1, NBINS, 1, GL_TEXTURE_2D, GL_RGB32F, GL_RGB, GL_FLOAT)
        lineFboP = NULL; //depends on region size
        lineFboQ = NULL;
        meanShiftFbo = NULL;
        sumFbo = NULL;

        lineSize = 0;

        quad.createQuad();
    }

    virtual void setRegionDimensionsAndCenter (Eigen::Vector2i viewport, Eigen::Vector2i firstCorner, Eigen::Vector2i spread)
    {
        frameViewport = viewport;
        std::cout<<"fC = "<<firstCorner[0]<<", "<<firstCorner[1]<<"; \n spread = "<<spread[0]<<" ,"<<spread[1]<<"; \n viewport = "<<viewport[0]<<", "<<viewport[1]<<";"<<std::endl;

        lowerCorner = Eigen::Vector2i(
            min(firstCorner[0], spread[0]),
            min(firstCorner[1], spread[1])
        );

        regionDimensions = Eigen::Vector2i(
            (int)abs((spread[0]-firstCorner[0])),
            (int)abs((spread[1]-firstCorner[1]))
        );

        center = Eigen::Vector2i(
            (int)((spread[0]+firstCorner[0])/2),
            (int)((spread[1]+firstCorner[1])/2)
        );

        if(regionDimensions[0] >= regionDimensions[1])
        {
            lineSize = regionDimensions[0];
            widthIsMax = true;
        }
        else
        {
            lineSize = regionDimensions[1];
            widthIsMax = false;
        }

        std::cout<<"Params: \n frameViewport: x="<<frameViewport[0]<<" y="<<frameViewport[1]<<"\n";
        std::cout<<"regionDimensions: x="<<regionDimensions[0]<<" y="<<regionDimensions[1]<<"\n";
        std::cout<<"center: x="<<center[0]<<" y="<<center[1]<<std::endl;

        #ifdef DEBUGVIEW
            debugQuad.createQuad();
            loadShader(debugShader,"fulldbug");
        #endif

        Tucano::Misc::errorCheckFunc(__FILE__, __LINE__);

    }

    Tucano::Texture* roiPointer ()
    {
        return (regionFbo->getTexture(0));
    }

    Eigen::Vector2i viewport()
    {
        return regionDimensions;
    }

    virtual void firstFrame (Eigen::Vector2i viewport, Eigen::Vector2i firstCorner, Eigen::Vector2i spread, Tucano::Texture* frame, double& frameNorm)
    {
        setRegionDimensionsAndCenter(viewport, firstCorner, spread);
        
    }

    virtual void track (Tucano::Texture* frame, Eigen::Vector2i* corner, Eigen::Vector2i* spread, int itermax, double& frameNorm)
    {
        
    }

private:

    Tucano::Shader pshader;
    Tucano::Shader histogramshader;
    Tucano::Shader meanshiftshader;
    Tucano::Shader sumshader;

    bool widthIsMax;
    bool isSet;

    Tucano::Framebuffer* regionFbo;
    Tucano::Framebuffer* meanShiftFbo;
    Tucano::Framebuffer* lineFboP;
    Tucano::Framebuffer* lineFboQ;
    Tucano::Framebuffer* sumFbo;

    float* histogramRaw;

    Tucano::Texture* qHistogramTexture;
    Tucano::Texture* pHistogramTexture;

    Eigen::Vector2i regionDimensions;
    Eigen::Vector2i center;
    Eigen::Vector2i frameViewport;
    Eigen::Vector2i lowerCorner;

    int lineSize;
    float dividerP;
    float dividerQ;

    Tucano::Mesh quad;

    #ifdef DEBUGVIEW
        Tucano::Mesh debugQuad;
        Tucano::Shader debugShader;
    #endif

};

#endif