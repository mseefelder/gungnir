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
        maskAndFrame = NULL;
        NRAM = 0;
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
        loadShader(binConstantShader, "binarize");
        loadShader(binarizeShader, "binFrame");
        loadShader(descriptorShader, "descriptor");
        loadShader(classifierShader, "classifier");
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
        NRAM = (remain==0)?numPixels/3:((numPixels-remain)/3)+1;

        int indexes[numPixels];
        for (int i = 0; i < numPixels; ++i)
        {
            indexes[i] = i;
        }

        int maskBufferSize = ((numPixels)
                +(frameViewport[0]*frameViewport[1])
                +((frameViewport[0]-regionDimensions[0])*(frameViewport[1]-regionDimensions[1])));

        int mask[maskBufferSize];
        std::random_device rd; // obtain a random number from hardware
        std::mt19937 eng(rd()); // seed the generator
        std::uniform_int_distribution<> distr(0, numPixels-1); // define the range
        int currIndex, mistery;
        for (int i = 0; i < maskBufferSize; ++i)
        {
            mask[i] = 0;
        }
        for (int i = 0; i < numPixels; ++i)
        {
            mistery = distr(eng);
            currIndex = indexes[mistery];
            while(currIndex<0)
            {
                mistery++;
                if(mistery>=numPixels)
                    mistery = 0;
                currIndex = indexes[mistery];
            }
            mask[i] = currIndex;
            indexes[mistery] = -1;
        }
        for (int i = 0; i < numPixels; ++i)
        {
            std::cout<<mask[i]<<", ";
        }
        std::cout<<std::endl;

        trackInfo = new ShaderStorageBufferInt(6+2*NRAM);
        maskAndFrame = new ShaderStorageBufferInt(maskBufferSize, mask);
        Tucano::Misc::errorCheckFunc(__FILE__, __LINE__);

    }

    void generateDescriptor(Tucano::Texture* frame, Eigen::Vector2i &firstCorner, Eigen::Vector2i &spread)
    {
        //set binarization parameters
        binarize(frame, firstCorner, spread);
        //debug render binarized
        //debug(frame, firstCorner, spread);
        descriptorShader.bind();
        descriptorShader.setUniform("frameTexture", frame->bind());
        descriptorShader.setUniform("lowerCorner", lowerCorner);
        descriptorShader.setUniform("dimensions", regionDimensions);
        quad.render();
        descriptorShader.unbind();
        frame->unbind();
    }

    void binarize(Tucano::Texture* frame, Eigen::Vector2i &firstCorner, Eigen::Vector2i &spread)
    {
        binConstantShader.bind();

        GLint bindingPoint = frame->bind();

        binConstantShader.setUniform("frameTexture", bindingPoint);
        binConstantShader.setUniform("center", center);
        binConstantShader.setUniform("lowerCorner", lowerCorner);
        binConstantShader.setUniform("dimensions", regionDimensions);

        quad.render();
        binConstantShader.unbind();

        binarizeShader.bind();

        binarizeShader.setUniform("frameTexture", bindingPoint);
        binarizeShader.setUniform("dimensions", regionDimensions);
        binarizeShader.setUniform("viewport", frameViewport);

        quad.render();
        binarizeShader.unbind();
    
        frame->unbind();

    }

    void classify(Tucano::Texture* frame, Eigen::Vector2i &firstCorner, Eigen::Vector2i &spread)
    {
        //glViewport(0, 0, frameViewport[0]-regionDimensions[0], frameViewport[1]-regionDimensions[1]);
        glViewport(0, 0, (frameViewport[0]-regionDimensions[0])*NRAM, 
            (frameViewport[1]-regionDimensions[1])*NRAM);
        classifierShader.bind();

        classifierShader.setUniform("dimensions", regionDimensions);
        classifierShader.setUniform("viewport", frameViewport);

        quad.render();
        classifierShader.unbind();
        glViewport(0, 0, frameViewport[0], frameViewport[1]);
    }

    void debug(Tucano::Texture* frame, Eigen::Vector2i &firstCorner, Eigen::Vector2i &spread)
    {
        debugShader.bind();

        debugShader.setUniform("dimensions", regionDimensions);
        debugShader.setUniform("frameTexture", frame->bind());
        debugShader.setUniform("viewport", frameViewport);

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
        maskAndFrame->bindBase(1);
        generateDescriptor(frame, firstCorner, spread);
        trackInfo->unbindBase();
        maskAndFrame->unbindBase();
    }

    virtual void track (Tucano::Texture* frame, Eigen::Vector2i* firstCorner, Eigen::Vector2i* spread, int itermax, double& frameNorm)
    {
        //trackInfo->clear();
        trackInfo->bindBase(0);
        maskAndFrame->bindBase(1);
        binarize(frame, *firstCorner, *spread);
        classify(frame, *firstCorner, *spread);
        debug(frame, *firstCorner, *spread);
        trackInfo->unbindBase();
        maskAndFrame->unbindBase();
        //maskAndFrame->printBuffer();
    }

private:

    Tucano::Shader binConstantShader;
    Tucano::Shader binarizeShader;
    Tucano::Shader descriptorShader;
    Tucano::Shader classifierShader;
    Tucano::Shader debugShader;

    bool widthIsMax;
    bool isSet;

    Eigen::Vector2i regionDimensions;
    Eigen::Vector2i center;
    Eigen::Vector2i frameViewport;
    Eigen::Vector2i lowerCorner;

    int lineSize;
    int NRAM;

    ShaderStorageBufferInt* trackInfo;
    ShaderStorageBufferInt* maskAndFrame;

    Tucano::Mesh quad;

    #ifdef DEBUGVIEW
        Tucano::Mesh debugQuad;
        Tucano::Shader debugShader;
    #endif

};

#endif