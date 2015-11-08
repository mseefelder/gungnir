#ifndef __MEANSHIFT__
#define __MEANSHIFT___

#include <tucano.hpp>

#define RAMBITS 4
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
        score = NULL;
        dummyFbo = NULL;
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
        loadShader(classifierShader, "classifier-new");
        loadShader(debugShader, "debug");

        //flags
        isSet = false;
        widthIsMax = false;

        lineSize = 0;

        quad.createQuad();
    }

    virtual void setRegionDimensionsAndCenter (Eigen::Vector2i viewport, Eigen::Vector2i firstCorner, Eigen::Vector2i spread, int nRegions)
    {
        frameViewport = viewport;
        numRegions = nRegions;
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
        int remain = numPixels%RAMBITS;  
        NRAM = (remain==0)?numPixels/RAMBITS:((numPixels-remain)/RAMBITS)+1;

        //create mask
        int indexes[numPixels];
        for (int i = 0; i < numPixels; ++i)
        {
            indexes[i] = i;
        }

        int maskBufferSize = ((2*numPixels)+(frameViewport[0]*frameViewport[1]));
                //+((frameViewport[0]-regionDimensions[0])*(frameViewport[1]-regionDimensions[1])));

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
            mask[mask[i]+numPixels] = i;
        }

        trackInfo = new ShaderStorageBufferInt(6+numPixels);
        maskAndFrame = new ShaderStorageBufferInt(maskBufferSize, mask);


        GLint dims[2];
        glGetIntegerv(GL_MAX_VIEWPORT_DIMS, &dims[0]);
        std::cout<<"Max viewport dims"<<dims[0]<<"x"<<dims[1]<<std::endl;

        searchWindowDimensions = Eigen::Vector2i(
            regionDimensions[0]*numRegions,
            regionDimensions[0]*numRegions
            );

        int scoreBufferSize = (searchWindowDimensions[0]*searchWindowDimensions[1]);
        score = new ShaderStorageBufferInt(scoreBufferSize);
        
        int classifyX, classifyY;
        classifyX = searchWindowDimensions[0]*(regionDimensions[0]/2);
        classifyY = searchWindowDimensions[1]*(regionDimensions[1]/2);
        /**
        classifyX = (frameViewport[0]-regionDimensions[0])*regionDimensions[0];
        classifyY = (frameViewport[1]-regionDimensions[1])*regionDimensions[1];
        float divider = sqrt(float(RAMBITS));
        if(classifyX>=classifyY)
            classifyX = int(ceil(classifyX/divider));
        else if(classifyY>classifyX)
            classifyY = int(ceil(classifyY/divider));
        /**/
        classifierSize = Eigen::Vector2i(classifyX, classifyY);
        std::cout<<"Trying           "<<classifierSize[0]<<"x"<<classifierSize[1]<<std::endl;

        dummyFbo = new Tucano::Framebuffer(classifierSize[0], classifierSize[1], 1, GL_TEXTURE_2D, GL_R8, GL_RED, GL_UNSIGNED_BYTE);

        Tucano::Misc::errorCheckFunc(__FILE__, __LINE__);
    }

    void generateDescriptor(Tucano::Texture* frame, Eigen::Vector2i &firstCorner, Eigen::Vector2i &spread)
    {
        //set binarization parameters
        firstBinarize(frame, firstCorner, spread);
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

    void firstBinarize(Tucano::Texture* frame, Eigen::Vector2i &firstCorner, Eigen::Vector2i &spread)
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

    void binarize(Tucano::Texture* frame, Eigen::Vector2i &firstCorner, Eigen::Vector2i &spread)
    {
        binarizeShader.bind();

        binarizeShader.setUniform("frameTexture", frame->bind());
        binarizeShader.setUniform("dimensions", regionDimensions);
        binarizeShader.setUniform("viewport", frameViewport);

        quad.render();
        binarizeShader.unbind();
    
        frame->unbind();
    }

    void classify(Tucano::Texture* frame, Eigen::Vector2i &firstCorner, Eigen::Vector2i &spread)
    {
        //glViewport(0, 0, frameViewport[0]-regionDimensions[0], frameViewport[1]-regionDimensions[1]);
        //glViewport(0, 0, (frameViewport[0]-regionDimensions[0]), 
        //    (frameViewport[1]-regionDimensions[1]));
        dummyFbo->bindRenderBuffer(0);
        glViewport(0, 0, classifierSize[0], classifierSize[1]);
        //GLint dims[4];
        //glGetIntegerv(GL_VIEWPORT, &dims[0]);
        //std::cout<<"Actual viewport size "<<dims[2]<<" "<<dims[3]<<std::endl;
        classifierShader.bind();

        Eigen::Vector2i searchWindowCorner = Eigen::Vector2i(
            lowerCorner[0]-(searchWindowDimensions[0]/2),
            lowerCorner[1]-(searchWindowDimensions[1]/2)
            );

        classifierShader.setUniform("SWsize",searchWindowDimensions);
        classifierShader.setUniform("SWcorner",searchWindowCorner);
        classifierShader.setUniform("totalSize",classifierSize);
        classifierShader.setUniform("frameSize",frameViewport);
        classifierShader.setUniform("ROIsize",regionDimensions);
        classifierShader.setUniform("rambits", RAMBITS);

        quad.render();

        classifierShader.unbind();
        dummyFbo->unbind();
        glViewport(0, 0, frameViewport[0], frameViewport[1]);
    }

    void debug(Tucano::Texture* frame, Eigen::Vector2i &firstCorner, Eigen::Vector2i &spread)
    {
        debugShader.bind();

        debugShader.setUniform("dimensions", regionDimensions);
        debugShader.setUniform("frameTexture", frame->bind());
        debugShader.setUniform("viewport", frameViewport);
        debugShader.setUniform("SWsize",searchWindowDimensions);

        quad.render();
        debugShader.unbind();
        frame->unbind();
    }

    Eigen::Vector2i viewport()
    {
        return regionDimensions;
    }

    virtual void firstFrame (Eigen::Vector2i viewport, Eigen::Vector2i firstCorner, Eigen::Vector2i spread, Tucano::Texture* frame, double& frameNorm, int nRegions)
    {

        setRegionDimensionsAndCenter(viewport, firstCorner, spread, nRegions);
        trackInfo->clear();
        trackInfo->bindBase(0);
        maskAndFrame->bindBase(1);
        generateDescriptor(frame, firstCorner, spread);
        trackInfo->unbindBase();
        maskAndFrame->unbindBase();
    }

    virtual void track (Tucano::Texture* frame, Eigen::Vector2i* firstCorner, Eigen::Vector2i* spread, int itermax, double& frameNorm)
    {
        score->clear();
        trackInfo->bindBase(0);
        maskAndFrame->bindBase(1);
        score->bindBase(2);
        binarize(frame, *firstCorner, *spread);
        classify(frame, *firstCorner, *spread);
        debug(frame, *firstCorner, *spread);
        trackInfo->unbindBase();
        maskAndFrame->unbindBase();
        score->unbindBase();

        int* scores;
        int maxIndex = 0;
        int maxValue = 0;
        int equalcounter = 0;
        int lastSize = 0;
        int arraySize = searchWindowDimensions[0]*searchWindowDimensions[1];
        score->readBuffer(&scores);
        for (int i = 0; i < arraySize; ++i)
        {
            if(scores[i] > maxValue)
            {
                equalcounter = 0;
                maxValue = scores[i];
                maxIndex = i;
                lastSize = abs((arraySize/2)-i);
            }
            else if(scores[i] == maxValue)
            {
                int size = abs((arraySize/2)-i);
                if (size<lastSize)
                {
                    maxIndex = i;
                }
                equalcounter++;
            }
        }
        std::cout<<"Best candidate: "<<maxIndex<<", with score: "<<(maxValue/float(NRAM))<<" | "<<equalcounter<<" copies..."<<std::endl;
        Eigen::Vector2i searchWindowCorner = Eigen::Vector2i(
            lowerCorner[0]-(searchWindowDimensions[0]/2),
            lowerCorner[1]-(searchWindowDimensions[1]/2)
            );
        int x = maxIndex%searchWindowDimensions[0];
        Eigen::Vector2i candidateCorner = Eigen::Vector2i(
            x,
            (maxIndex-x)/searchWindowDimensions[0]);
        *firstCorner = searchWindowCorner + candidateCorner;
        *spread = *firstCorner + regionDimensions;

    }

private:

    Tucano::Shader binConstantShader;
    Tucano::Shader binarizeShader;
    Tucano::Shader descriptorShader;
    Tucano::Shader classifierShader;
    Tucano::Shader debugShader;

    Tucano::Framebuffer* dummyFbo;

    bool widthIsMax;
    bool isSet;

    Eigen::Vector2i regionDimensions;
    Eigen::Vector2i searchWindowDimensions;
    Eigen::Vector2i center;
    Eigen::Vector2i frameViewport;
    Eigen::Vector2i lowerCorner;
    Eigen::Vector2i classifierSize;

    int numRegions;
    int lineSize;
    int NRAM;

    ShaderStorageBufferInt* trackInfo;
    ShaderStorageBufferInt* maskAndFrame;
    ShaderStorageBufferInt* score;

    Tucano::Mesh quad;

    #ifdef DEBUGVIEW
        Tucano::Mesh debugQuad;
        Tucano::Shader debugShader;
    #endif

};

#endif