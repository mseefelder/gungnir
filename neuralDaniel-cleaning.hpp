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
        trackInfo = NULL;
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
        loadShader(binarizeShader, "binarize");
        loadShader(debugShader, "debug");

        //flags
        isSet = false;
        widthIsMax = false;

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

        //create ssbo that will support the entire tracking
        int numPixels = (regionDimensions[0]*regionDimensions[1]);
        int remain = numPixels%3;  
        int NRAM = (remain==0)?numPixels/3:((numPixels-remain)/3)+1;
        trackInfo = new ShaderStorageBufferInt(6+2*NRAM);

        Tucano::Misc::errorCheckFunc(__FILE__, __LINE__);

    }

    void generateDescriptor(Tucano::Texture* frame, Eigen::Vector2i &firstCorner, Eigen::Vector2i &spread)
    {
        //set binarization parameters
        binarize(frame, firstCorner, spread);
        //debug render binarized
        debug(frame, firstCorner, spread);
    }

    void binarize(Tucano::Texture* frame, Eigen::Vector2i &firstCorner, Eigen::Vector2i &spread)
    {
        binarizeShader.bind();

        binarizeShader.setUniform("frameTexture", frame->bind());
        binarizeShader.setUniform("center", center);
        binarizeShader.setUniform("lowerCorner", lowerCorner);
        binarizeShader.setUniform("dimensions", regionDimensions);

        quad.render();
        binarizeShader.unbind();
        frame->unbind();
    }

    void debug(Tucano::Texture* frame, Eigen::Vector2i &firstCorner, Eigen::Vector2i &spread)
    {
        debugShader.bind();

        debugShader.setUniform("frameTexture", frame->bind());

        quad.render();
        debugShader.unbind();
        frame->unbind();
    }

    Eigen::Vector2i viewport()
    {
        return regionDimensions;
    }

    virtual void firstFrame (Eigen::Vector2i viewport, Eigen::Vector2i firstCorner, Eigen::Vector2i spread, Tucano::Texture* frame, double& frameNorm)
    {

        setRegionDimensionsAndCenter(viewport, firstCorner, spread);
        trackInfo->clear();
        trackInfo->bindBase(0);
        generateDescriptor(frame, firstCorner, spread);
        trackInfo->unbindBase();
    }

    virtual void track (Tucano::Texture* frame, Eigen::Vector2i* firstCorner, Eigen::Vector2i* spread, int itermax, double& frameNorm)
    {
        trackInfo->clear();
        trackInfo->bindBase(0);
        //debug(frame, *firstCorner, *spread);
        generateDescriptor(frame, *firstCorner, *spread);
        trackInfo->unbindBase();
    }

private:

    Tucano::Shader binarizeShader;
    Tucano::Shader debugShader;

    bool widthIsMax;
    bool isSet;

    Eigen::Vector2i regionDimensions;
    Eigen::Vector2i center;
    Eigen::Vector2i frameViewport;
    Eigen::Vector2i lowerCorner;

    int lineSize;

    ShaderStorageBufferInt* trackInfo;

    Tucano::Mesh quad;

    #ifdef DEBUGVIEW
        Tucano::Mesh debugQuad;
        Tucano::Shader debugShader;
    #endif

};

#endif