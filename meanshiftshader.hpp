#ifndef __MEANSHIFT__
#define __MEANSHIFT___

#include <tucano.hpp>
#include "tracker.hpp"

#define NBINS 32
//#define DEBUGVIEW

/**
 * @brief Picks a 3D position from screen position
 */
class Meanshift : public Tracker
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

        //Setup fbo for Region of Interest and Line for summing
        regionFbo = new Tucano::Framebuffer(regionDimensions[0], regionDimensions[1], 1, GL_TEXTURE_2D, GL_RGBA32F, GL_RGBA, GL_FLOAT);
            // (red_bin, green_bin, blue_bin, value)
        meanShiftFbo = new Tucano::Framebuffer(regionDimensions[0], regionDimensions[1], 1, GL_TEXTURE_2D, GL_RGBA32F, GL_RGBA, GL_FLOAT);
            // (xvalue, yvalue, divider to sum, nothing)

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

        
        lineFboP = new Tucano::Framebuffer(1, NBINS, 1, GL_TEXTURE_2D, GL_RGB32F, GL_RGB, GL_FLOAT);
        lineFboQ = new Tucano::Framebuffer(1, NBINS, 1, GL_TEXTURE_2D, GL_RGB32F, GL_RGB, GL_FLOAT);
        sumFbo = new Tucano::Framebuffer(1, lineSize, 1, GL_TEXTURE_2D, GL_RGB32F, GL_RGB, GL_FLOAT);

        qHistogramTexture = new Tucano::Texture();
        qHistogramTexture->create(GL_TEXTURE_2D, GL_RGB32F, NBINS, 1, GL_RGB, GL_FLOAT, NULL);
        
        pHistogramTexture = new Tucano::Texture();
        pHistogramTexture->create(GL_TEXTURE_2D, GL_RGB32F, NBINS, 1, GL_RGB, GL_FLOAT, NULL);

        regionFbo->clearAttachments();
        lineFboP->clearAttachments();
        lineFboQ->clearAttachments();
        meanShiftFbo->clearAttachments();
        sumFbo->clearAttachments();

        std::cout<<"Params: \n frameViewport: x="<<frameViewport[0]<<" y="<<frameViewport[1]<<"\n";
        std::cout<<"regionDimensions: x="<<regionDimensions[0]<<" y="<<regionDimensions[1]<<"\n";
        std::cout<<"center: x="<<center[0]<<" y="<<center[1]<<std::endl;

        #ifdef DEBUGVIEW
            debugQuad.createQuad();
            loadShader(debugShader,"fulldbug");
        #endif

        Tucano::Misc::errorCheckFunc(__FILE__, __LINE__);

    }

    virtual void histogram (Tucano::Texture* frame, Tucano::Framebuffer* lineFbo, float* divider)
    {   
        //std::cout<<"histogram: Calculate pixel values"<<std::endl;
        //Calculate pixel values-------------------------------------------------

        //bind regionFbo
        regionFbo->clearAttachments();
        regionFbo->bindRenderBuffer(0);
        glViewport(0, 0, regionFbo->getWidth(), regionFbo->getHeight());

        pshader.bind();

        pshader.setUniform("frameTexture", frame->bind());
        pshader.setUniform("center", center);
        pshader.setUniform("lowerCorner", lowerCorner);
        pshader.setUniform("dimensions", regionDimensions);
        pshader.setUniform("viewport", frameViewport);

        //render
        quad.render();

        pshader.unbind();

        frame->unbind();
        regionFbo->unbind();

        /**
        Eigen::Vector4f preTemp;
        for(int regX = 0; regX < regionDimensions[0]; regX++)
        {
            for(int regY = 0; regY < regionDimensions[1]; regY++)
            {
                preTemp = regionFbo->readPixel(0, Eigen::Vector2i(regX, regY));
                std::cout<<"P/Q Bins: "<<preTemp[0]<<", "<<preTemp[1]<<", "<<preTemp[2]<<"| value: "<<preTemp[3]<<std::endl;
            }
        }
        /**/
        
        //std::cout<<"histogram: Sum up histogram to one line"<<std::endl;
        //Sum up histogram to one line-------------------------------------------
        lineFbo->clearAttachments();
        lineFbo->bindRenderBuffer(0);
        glViewport(0, 0, lineFbo->getWidth(), lineFbo->getHeight());

        histogramshader.bind();

        histogramshader.setUniform("pvalues", (regionFbo->getTexture(0))->bind());
        histogramshader.setUniform("dimensions", regionDimensions);

        //render
        quad.render();

        histogramshader.unbind();
        
        (regionFbo->getTexture(0))->unbind();
        lineFbo->unbind();

        //std::cout<<"histogram: CPU: sum up and build histogram to texture"<<std::endl;
        //CPU: sum up and build histogram to texture-----------------------------

        //
        Eigen::Vector4f temp;
        *divider = 0.0;
        int bin[3];
        
        
        //std::cout<<"histogram: lineFbo contents"<<std::endl;
        //THE DIVIDER TERM GETS CANCELLED OUT IN THE WEIGHT CALCULATION ------------------- IMPORTANT

        // for(int dbinIndex = 0; dbinIndex < NBINS; dbinIndex++)
        // {
        //     temp = lineFbo->readPixel(0, Eigen::Vector2i(0, dbinIndex));
        //     *divider += temp[0]+temp[1]+temp[2];
        //     /**std::cout<<" H Bin "<<dbinIndex<<": R:"<<temp[0]<<" G:"<<temp[1]<<" B:"<<temp[2]<<" "<<temp[3]<<std::endl;/**/
        // }
        
        //------------------------------------------------------------- ------------------- IMPORTANT
    }

    virtual void histogramQ (Tucano::Texture* frame)
    {
        histogram(frame, lineFboQ, &dividerQ);
    }

    virtual void histogramP (Tucano::Texture* frame)
    {
        histogram(frame, lineFboP, &dividerP);
    }

    float histogramDiff ()
    {
        float diff = 0.0;
        float qSize = 0.0;
        Eigen::Vector4f tempP;
        Eigen::Vector4f tempQ;
        for (int i = 0; i < NBINS; ++i)
        {
            tempP = lineFboP->readPixel(0, Eigen::Vector2i(0, i));
            tempQ = lineFboQ->readPixel(0, Eigen::Vector2i(0, i));
            qSize += (tempQ[0]*tempQ[0])+(tempQ[1]*tempQ[1])+(tempQ[2]*tempQ[2]);
            diff += (tempP[0]-tempQ[0])*(tempP[0]-tempQ[0])+(tempP[1]-tempQ[1])*(tempP[1]-tempQ[1])+(tempP[2]-tempQ[2])*(tempP[2]-tempQ[2]);
        }
        return sqrt(diff)/sqrt(qSize);
    }

    /**
     * @brief Calculates the meanshift vector    
     */
    virtual Eigen::Vector2f meanshift (Eigen::Vector2i* corner, Eigen::Vector2i* spread)
    {
        //Calculate pixel values-------------------------------------------------

        //bind regionFbo
        meanShiftFbo->clearAttachments();
        meanShiftFbo->bindRenderBuffer(0);
        glViewport(0, 0, meanShiftFbo->getWidth(), meanShiftFbo->getHeight());

        meanshiftshader.bind();

        meanshiftshader.setUniform("pvalues", (regionFbo->getTexture(0))->bind());
        meanshiftshader.setUniform("center", center);
        meanshiftshader.setUniform("dimensions", regionDimensions);
        meanshiftshader.setUniform("qHistogram", (lineFboQ->getTexture(0))->bind());//qHistogramTexture->bind());
        meanshiftshader.setUniform("pHistogram", (lineFboP->getTexture(0))->bind());//pHistogramTexture->bind());
        meanshiftshader.setUniform("viewport", frameViewport);
        //meanshiftshader.setUniform("dividerP", dividerP);
        //meanshiftshader.setUniform("dividerQ", dividerQ);
        meanshiftshader.setUniform("lowerCorner", lowerCorner);


        //render
        quad.render();

        meanshiftshader.unbind();

        (regionFbo->getTexture(0))->unbind();
        (lineFboQ->getTexture(0))->unbind();//qHistogramTexture->unbind();
        (lineFboP->getTexture(0))->unbind();//pHistogramTexture->unbind();
        meanShiftFbo->unbind();

        //debug
        /**
        Eigen::Vector4f preTemp;
        for(int regX = 0; regX < regionDimensions[0]; regX++)
        {
            for(int regY = 0; regY < regionDimensions[1]; regY++)
            {
                preTemp = meanShiftFbo->readPixel(0, Eigen::Vector2i(regX, regY));
                std::cout<<"Meanshift: x:"<<preTemp[0]<<" y:"<<preTemp[1]<<" d:"<<preTemp[2]<<" "<<preTemp[3]<<std::endl;
            }
        }
        /**/

        //Sum up histogram to one line-------------------------------------------
        sumFbo->clearAttachments();
        sumFbo->bindRenderBuffer(0);
        glViewport(0, 0, sumFbo->getWidth(), sumFbo->getHeight());

        sumshader.bind();

        sumshader.setUniform("pvalues", (meanShiftFbo->getTexture(0))->bind());
        sumshader.setUniform("dimensions", regionDimensions);
        sumshader.setUniform("widthIsMax", widthIsMax);

        //render
        quad.render();

        sumshader.unbind();
        
        (meanShiftFbo->getTexture(0))->unbind();
        sumFbo->unbind();
        //CPU: sum up and build histogram to texture-----------------------------

        Eigen::Vector4f temp;
        float xComponent = 0.0;
        float yComponent = 0.0;
        float divider = 0.0;

        for (int i = 0; i < lineSize; ++i)
        {
            temp = sumFbo->readPixel(0, Eigen::Vector2i(0, i));
            //std::cout<<"Pixel on sum: summed values"<<temp[0]<<" "<<temp[1]<<" "<<temp[2]<<" "<<temp[3]<<std::endl;
            xComponent += temp[0];
            yComponent += temp[1];
            divider += temp[2];
        }

        xComponent /= divider;
        yComponent /= divider;
        //std::cout<<"Lower corner: "<<lowerCorner<<std::endl;
        //std::cout<<"Float result: "<<xComponent<<", "<<yComponent<<std::endl;
        int intX = (int)round(xComponent);
        int intY = (int)round(yComponent);

        Eigen::Vector2i newCenter(intX, intY);
        Eigen::Vector2f newCenterF(xComponent, yComponent);
        Eigen::Vector2i meanShiftVector = newCenter-center;
        Eigen::Vector2f meanShiftVectorF = newCenterF - Eigen::Vector2f(center[0],center[1]);
        //std::cout<<"New center: "<<newCenter<<"\n meanshift vector: "<<meanShiftVector<<std::endl;

        *corner += meanShiftVector;
        *spread += meanShiftVector;
        lowerCorner += meanShiftVector;
        center = newCenter;

        return meanShiftVectorF;
    }

    Tucano::Texture* roiPointer ()
    {
        return (regionFbo->getTexture(0));
    }

    Eigen::Vector2i viewport()
    {
        return regionDimensions;
    }

    virtual void firstFrame (Eigen::Vector2i viewport, Eigen::Vector2i firstCorner, Eigen::Vector2i spread, Tucano::Texture* frame)
    {
        setRegionDimensionsAndCenter(viewport, firstCorner, spread);
        histogramQ(frame);

    }

    virtual void track (Tucano::Texture* frame, Eigen::Vector2i* corner, Eigen::Vector2i* spread, int itermax)
    {
        float shift = 2.0;
        int iter = 1;
        while (shift > 1.0 && iter < itermax){
            histogramP(frame);
            shift = meanshift(corner, spread).norm();
            iter++;
        }

        #ifdef DEBUGVIEW
            glViewport(0,0,frameViewport[0], frameViewport[1]);
            debugShader.bind();
            debugShader.setUniform("frameTexture", frame->bind());
            debugShader.setUniform("dimensions", regionDimensions);
            debugShader.setUniform("lowerCorner", lowerCorner);
            debugQuad.render();
            debugShader.unbind();
        #endif

        float d = histogramDiff();
        if (d > 0.09 && d < 0.1)
        {
            histogramQ(frame);
        }
        std::cout<<histogramDiff()<<std::endl;
        
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