#ifndef __MEANSHIFT__
#define __MEANSHIFT___

#include <tucano.hpp>

#define NBINS 16

namespace Effects
{

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
        lineFbo = NULL;

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
        lineFbo = NULL; //depends on region size
        meanShiftFbo = NULL;

        lineSize = 0;

        quad.createQuad();
    }

    virtual void setRegionDimensionsAndCenter (Eigen::Vector2i viewport, Eigen::Vector2f firstCorner, Eigen::Vector2f spread)
    {
        std::cout<<"fC = "<<firstCorner[0]<<", "<<firstCorner[1]<<"; \n spread = "<<spread[0]<<" ,"<<spread[1]<<"; \n viewport = "<<viewport[0]<<", "<<viewport[1]<<";"<<std::endl;

        frameViewport = viewport;

        regionDimensions = Eigen::Vector2i(
            abs((spread[0]-firstCorner[0]))*frameViewport[0],
            abs((spread[1]-firstCorner[1]))*frameViewport[1]
        );

        center = Eigen::Vector2f(
            ((spread[0]+firstCorner[0])/2),
            ((spread[1]+firstCorner[1])/2)
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

        lineFbo = new Tucano::Framebuffer(1, NBINS, 1, GL_TEXTURE_2D, GL_RGB32F, GL_RGB, GL_FLOAT);
        //lineFbo = new Tucano::Framebuffer(3, NBINS, 1, GL_TEXTURE_2D, GL_R32F, GL_RED, GL_FLOAT);

        qHistogramTexture = new Tucano::Texture();
        qHistogramTexture->create(GL_TEXTURE_2D, GL_RGB32F, lineSize, 1, GL_RGB, GL_FLOAT, NULL);
        
        pHistogramTexture = new Tucano::Texture();
        pHistogramTexture->create(GL_TEXTURE_2D, GL_RGB32F, lineSize, 1, GL_RGB, GL_FLOAT, NULL);

        regionFbo->clearAttachments();
        lineFbo->clearAttachments();
        meanShiftFbo->clearAttachments();

        std::cout<<"Params: \n frameViewport: x="<<frameViewport[0]<<" y="<<frameViewport[1]<<"\n";
        std::cout<<"regionDimensions: x="<<regionDimensions[0]<<" y="<<regionDimensions[1]<<"\n";
        std::cout<<"center: x="<<center[0]<<" y="<<center[1]<<std::endl;

        Tucano::Misc::errorCheckFunc(__FILE__, __LINE__);

    }

    virtual void histogram (Tucano::Texture* frame, Tucano::Texture* hTex)
    {
        //std::cout<<"histogram: Calculate pixel values"<<std::endl;
        //Calculate pixel values-------------------------------------------------

        //bind regionFbo
        regionFbo->clearAttachments();
        regionFbo->bindRenderBuffer(0);

        pshader.bind();

        pshader.setUniform("frameTexture", frame->bind());
        pshader.setUniform("center", center);
        pshader.setUniform("dimensions", regionDimensions);
        pshader.setUniform("viewport", frameViewport);

        //render
        quad.render();

        pshader.unbind();

        frame->unbind();
        regionFbo->unbind();


        Eigen::Vector4f preTemp;
        for(int regX = 0; regX < regionDimensions[0]; regX++)
        {
            for(int regY = 0; regY < regionDimensions[1]; regY++)
            {
                preTemp = regionFbo->readPixel(0, Eigen::Vector2i(regX, regY));
                std::cout<<"Region: pixel values"<<preTemp[0]<<" "<<preTemp[1]<<" "<<preTemp[2]<<" "<<preTemp[3]<<std::endl;
            }
        }

        //std::cout<<"histogram: Sum up histogram to one line"<<std::endl;
        //Sum up histogram to one line-------------------------------------------
        lineFbo->clearAttachments();
        lineFbo->bindRenderBuffer(0);

        histogramshader.bind();

        histogramshader.setUniform("pvalues", (regionFbo->getTexture(0))->bind());
        pshader.setUniform("dimensions", regionDimensions);

        //render
        quad.render();

        histogramshader.unbind();
        
        (regionFbo->getTexture(0))->unbind();
        lineFbo->unbind();

        std::cout<<"histogram: CPU: sum up and build histogram to texture"<<std::endl;
        //CPU: sum up and build histogram to texture-----------------------------
        
        std::cout<<"histogram: clean histogramRaw array"<<std::endl;
        //clean histogramRaw array
        for (int i = 0; i < NBINS*3; ++i)
        {
            std::cout<<i<<std::endl;
            histogramRaw[i] = 0.0;
        }

        //
        Eigen::Vector4f temp;
        float divider = 0.0;
        int bin[3];
        
        std::cout<<"histogram: read first pixel."<<std::endl;
        for(int binIndex = 0; binIndex < NBINS; binIndex++)
        {
            temp = lineFbo->readPixel(0, Eigen::Vector2i(0, binIndex));
            std::cout<<"pixel values"<<temp[0]<<" "<<temp[1]<<" "<<temp[2]<<" "<<temp[3]<<std::endl;
        }

        std::cout<<"histogram: calculate the first histogram values"<<std::endl;
        //calculate the first histogram values
        divider = temp[3];
        bin[0] = (int) (3*floor(temp[0]));
        histogramRaw[bin[0]] += temp[3];
        bin[1] = (int) (3*floor(temp[1])+1);
        histogramRaw[bin[1]] += temp[3];
        bin[2] = (int) (3*floor(temp[2])+2);
        histogramRaw[bin[2]] += temp[3];

        std::cout<<"histogram: calculate the rest of the histogram values"<<std::endl;
        std::cout<<"lineSize: "<<lineSize<<std::endl;
        //calculate the rest of the histogram values
        //for (int j = 1; j < lineSize; ++j)
        for (int column = 1; column < 3; ++column)
        {
            for (int row = 0; row < NBINS; ++row)
            {
                /* Re-make code
                std::cout<<column<<" "<<row<<std::endl;

                temp = lineFbo->readPixel(0, Eigen::Vector2i(column, row));

                divider += temp[3];
                bin[0] = (int) (3*floor(temp[0]));
                bin[1] = (int) (3*floor(temp[1])+1);
                bin[2] = (int) (3*floor(temp[2])+2);
                
                std::cout<<"pixel values"<<temp[0]<<" "<<temp[1]<<" "<<temp[2]<<" "<<temp[3]<<std::endl;
                std::cout<<"bins"<<bin[0]<<" "<<bin[1]<<" "<<bin[2]<<std::endl;

                histogramRaw[bin[0]] += temp[3];
                histogramRaw[bin[1]] += temp[3];
                histogramRaw[bin[2]] += temp[3];
                */
            }
        }

        std::cout<<"histogram: divide all histogram values"<<std::endl;
        //divide all histogram values
        for (int k = 0; k < NBINS*3; ++k)
        {
            histogramRaw[k] /= divider;
        }

        std::cout<<"histogram: CPU: sum up and build histogram to texture"<<std::endl;
        //update hTex with histogramRaw
        hTex->update(histogramRaw);
    }

    virtual void histogramQ (Tucano::Texture* frame)
    {
        histogram(frame, qHistogramTexture);
    }

    virtual void histogramP (Tucano::Texture* frame)
    {
        histogram(frame, pHistogramTexture);
    }

    /**
     * @brief Calculates the meanshift vector    
     */
    virtual Eigen::Vector2f meanshift (Eigen::Vector2f* corner, Eigen::Vector2f* spread)
    {
        //Calculate pixel values-------------------------------------------------

        //bind regionFbo
        meanShiftFbo->bindRenderBuffer(0);

        meanshiftshader.bind();

        meanshiftshader.setUniform("pvalues", (regionFbo->getTexture(0))->bind());
        meanshiftshader.setUniform("center", center);
        meanshiftshader.setUniform("dimensions", regionDimensions);
        meanshiftshader.setUniform("qHistogram", qHistogramTexture->bind());
        meanshiftshader.setUniform("pHistogram", pHistogramTexture->bind());
        meanshiftshader.setUniform("viewport", frameViewport);


        //render
        quad.render();

        meanshiftshader.unbind();

        (regionFbo->getTexture(0))->unbind();
        qHistogramTexture->unbind();
        pHistogramTexture->unbind();
        meanShiftFbo->unbind();

        //Sum up histogram to one line-------------------------------------------
        lineFbo->clearAttachments();
        lineFbo->bindRenderBuffer(0);

        sumshader.bind();

        sumshader.setUniform("pvalues", (regionFbo->getTexture(0))->bind());

        //render
        quad.render();

        sumshader.unbind();
        
        (regionFbo->getTexture(0))->unbind();
        lineFbo->unbind();

        //CPU: sum up and build histogram to texture-----------------------------

        //
        Eigen::Vector4f temp;
        float xComponent = 0.0;
        float yComponent = 0.0;
        float divider = 0.0;
        
        temp = lineFbo->readPixel(0, Eigen::Vector2i(0, 0));

        //
        divider = temp[2];
        xComponent += temp[0];
        yComponent += temp[1];

        for (int i = 1; i < lineSize; ++i)
        {
            temp = lineFbo->readPixel(0, Eigen::Vector2i(lineSize, 0));

            divider += temp[2];
            xComponent += temp[0];
            yComponent += temp[1];
        }

        
        xComponent /= divider;
        yComponent /= divider;

        Eigen::Vector2f meanShiftVector(xComponent, yComponent);

        *corner += meanShiftVector;
        *spread += meanShiftVector;
        center += meanShiftVector;

        return meanShiftVector;
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
    Tucano::Framebuffer* lineFbo;

    float* histogramRaw;

    Tucano::Texture* qHistogramTexture;
    Tucano::Texture* pHistogramTexture;

    Eigen::Vector2i regionDimensions;
    Eigen::Vector2f center;
    Eigen::Vector2i frameViewport;

    int lineSize;

    Tucano::Mesh quad;
};

}



#endif

// deprecated function

// virtual void histogram (Tucano::Texture* frame, Tucano::Texture* hTex, float* divider)
//     {
//         //std::cout<<"histogram: Calculate pixel values"<<std::endl;
//         //Calculate pixel values-------------------------------------------------

//         //bind regionFbo
//         regionFbo->clearAttachments();
//         regionFbo->bindRenderBuffer(0);

//         pshader.bind();

//         pshader.setUniform("frameTexture", frame->bind());
//         pshader.setUniform("center", center);
//         pshader.setUniform("dimensions", regionDimensions);

//         //render
//         quad.render();

//         pshader.unbind();

//         frame->unbind();
//         regionFbo->unbind();

//         //std::cout<<"histogram: Sum up histogram to one line"<<std::endl;
//         //Sum up histogram to one line-------------------------------------------
//         GLuint readUnit = (regionFbo->getTexture(0))->bind();
//         glBindImageTexture(readUnit, (regionFbo->getTexture(0))->texID(), 0, GL_FALSE, 0, GL_READ_WRITE,
//                        GL_R32F);
//         GLuint writeUnit = hTex->bind();
//         glBindImageTexture(writeUnit, hTex->texID(), 0, GL_FALSE, 0, GL_READ_WRITE,
//                        GL_RGBA32F);

//         histogramshader.bind();

//         histogramshader.setUniform("region", (GLint)readUnit);
//         histogramshader.setUniform("histogram", (GLint)writeUnit);//if error, cast to GLint
//         pshader.setUniform("dimensions", regionDimensions);

//         //render
//         glDispatchCompute(1,1,1);

//         glBindImageTexture(0, (regionFbo->getTexture(0))->texID(), 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
//         glBindImageTexture(0, hTex->texID(), 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
//         glBindImageTexture(0, 0, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

//         (regionFbo->getTexture(0))->unbind();
//         hTex->unbind();

//         histogramshader.unbind();

//         Tucano::Misc::errorCheckFunc(__FILE__, __LINE__);

//         //std::cout<<"histogram: CPU: sum up and build histogram to texture"<<std::endl;
//         //CPU: sum up and build histogram to texture-----------------------------
//         //Fetch histogram from GPU
        
//         hTex->bind();

//         if (histogramRaw) delete histogramRaw;
//         histogramRaw = new float[3*NBINS*4];
//         std::cout<<"histogram: debug pre "<<histogramRaw[0]<<std::endl;
//         //std::cout<<"histogram: divider "<<hist[0]<<std::endl;
//         //for (int i = 0; i < 3*NBINS*4; ++i)
//         //{
//         //    hist[i] = 42;
//         //}

//         glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

//         glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, histogramRaw);

//         Tucano::Misc::errorCheckFunc(__FILE__, __LINE__);

//         hTex->unbind();
//         *divider = 0;
//         //for (int i = 0; i < NBINS*3; ++i)
//         //{
//         //    *divider += histogramRaw[i];
//         //}
//         std::cout<<"histogram: debug pos "<<histogramRaw[0]<<std::endl;
        

//         /*
//         std::cout<<"histogram: clean histogramRaw array"<<std::endl;
//         //clean histogramRaw array
//         for (int i = 0; i < NBINS*3; ++i)
//         {
//             std::cout<<i<<std::endl;
//             histogramRaw[i] = 0.0;
//         }

//         //
//         Eigen::Vector4f temp;
//         float divider = 0.0;
//         int bin[3];
        
//         std::cout<<"histogram: read first pixel."<<std::endl;
//         temp = lineFbo->readPixel(0, Eigen::Vector2i(0, 0));
//         std::cout<<"pixel values"<<temp[0]<<" "<<temp[1]<<" "<<temp[2]<<" "<<temp[3]<<std::endl;

//         std::cout<<"histogram: calculate the first histogram values"<<std::endl;
//         //calculate the first histogram values
//         divider = temp[3];
//         bin[0] = (int) (3*floor(temp[0]));
//         histogramRaw[bin[0]] += temp[3];
//         bin[1] = (int) (3*floor(temp[1])+1);
//         histogramRaw[bin[1]] += temp[3];
//         bin[2] = (int) (3*floor(temp[2])+2);
//         histogramRaw[bin[2]] += temp[3];

//         std::cout<<"histogram: calculate the rest of the histogram values"<<std::endl;
//         std::cout<<"lineSize: "<<lineSize<<std::endl;
//         //calculate the rest of the histogram values
//         for (int j = 1; j < lineSize; ++j)
//         {
//             std::cout<<j<<std::endl;

//             temp = lineFbo->readPixel(0, Eigen::Vector2i(j, 0));

//             divider += temp[3];
//             bin[0] = (int) (3*floor(temp[0]));
//             bin[1] = (int) (3*floor(temp[1])+1);
//             bin[2] = (int) (3*floor(temp[2])+2);
            
//             std::cout<<"pixel values"<<temp[0]<<" "<<temp[1]<<" "<<temp[2]<<" "<<temp[3]<<std::endl;
//             std::cout<<"bins"<<bin[0]<<" "<<bin[1]<<" "<<bin[2]<<std::endl;

//             histogramRaw[bin[0]] += temp[3];
//             histogramRaw[bin[1]] += temp[3];
//             histogramRaw[bin[2]] += temp[3];
//         }

//         std::cout<<"histogram: divide all histogram values"<<std::endl;
//         //divide all histogram values
//         for (int k = 0; k < NBINS*3; ++k)
//         {
//             histogramRaw[k] /= divider;
//         }

//         std::cout<<"histogram: CPU: sum up and build histogram to texture"<<std::endl;
//         //update hTex with histogramRaw
//         hTex->update(histogramRaw);
//         */
//     }