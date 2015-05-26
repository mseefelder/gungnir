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

        lineSize = 0;

        quad.createQuad();
    }

    virtual void setRegionDimensionsAndCenter (Eigen::Vector2i viewport, Eigen::Vector2f firstCorner, Eigen::Vector2f spread)
    {
        frameViewport = viewport;

        regionDimensions = Eigen::Vector2f(
            (spread[0]-firstCorner[0]),
            (spread[1]-firstCorner[1])
        );

        center = Eigen::Vector2f(
            ((spread[0]+firstCorner[0])/2),
            ((spread[1]+firstCorner[1])/2)
        );

        //Setup fbo for Region of Interest and Line for summing
        regionFbo = new Tucano::Framebuffer(regionDimensions[0], regionDimensions[0], 1, GL_TEXTURE_2D, GL_RGBA32F, GL_RGBA, GL_FLOAT);
            // (red_bin, green_bin, blue_bin, value)
        meanShiftFbo = new Tucano::Framebuffer(regionDimensions[0], regionDimensions[0], 1, GL_TEXTURE_2D, GL_RGBA32F, GL_RGBA, GL_FLOAT);
            // (xvalue, yvalue, divider to sum, nothing)

        if(regionDimensions[0] >= regionDimensions[1])
        {
            lineFbo = new Tucano::Framebuffer(regionDimensions[0], 1, 1, GL_TEXTURE_2D, GL_RGBA32F, GL_RGBA, GL_FLOAT);
            lineSize = regionDimensions[0];
            widthIsMax = true;
        }
        else
        {
            lineFbo = new Tucano::Framebuffer(regionDimensions[1], 1, 1, GL_TEXTURE_2D, GL_RGBA32F, GL_RGBA, GL_FLOAT);
            lineSize = regionDimensions[1];
            widthIsMax = false;
        }

        qHistogramTexture = new Tucano::Texture();
        qHistogramTexture->create(GL_TEXTURE_2D, GL_RGB32F, lineSize, 1, GL_RGB, GL_FLOAT, NULL);
        
        pHistogramTexture = new Tucano::Texture();
        pHistogramTexture->create(GL_TEXTURE_2D, GL_RGB32F, lineSize, 1, GL_RGB, GL_FLOAT, NULL);

    }

    virtual void histogram (Tucano::Texture* frame, Tucano::Texture* hTex)
    {
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

        regionFbo->unbind();

        //Sum up histogram to one line-------------------------------------------
        lineFbo->clearAttachments();
        lineFbo->bindRenderBuffer(0);

        histogramshader.bind();

        histogramshader.setUniform("pvalues", (regionFbo->getTexture(0))->bind());

        //render
        quad.render();

        histogramshader.unbind();
        
        (regionFbo->getTexture(0))->unbind();
        lineFbo->unbind();

        //CPU: sum up and build histogram to texture-----------------------------
        
        //clean histogramRaw array
        for (int i = 0; i < NBINS*3; ++i)
        {
            histogramRaw[i] = 0.0;
        }

        //
        Eigen::Vector4f temp;
        float divider = 0.0;
        int bin[3];
        
        temp = lineFbo->readPixel(0, Eigen::Vector2i(0, 0));

        //
        divider = temp[3];
        bin[0] = (int) (3*floor(temp[0]));
        histogramRaw[bin[0]] += temp[3];
        bin[1] = (int) (3*floor(temp[1])+1);
        histogramRaw[bin[1]] += temp[3];
        bin[2] = (int) (3*floor(temp[2])+2);
        histogramRaw[bin[2]] += temp[3];

        for (int j = 1; j < lineSize; ++j)
        {
            temp = lineFbo->readPixel(0, Eigen::Vector2i(lineSize, 0));

            divider += temp[3];
            bin[0] = (int) (3*floor(temp[0]));
            histogramRaw[bin[0]] += temp[3];
            bin[1] = (int) (3*floor(temp[1])+1);
            histogramRaw[bin[1]] += temp[3];
            bin[2] = (int) (3*floor(temp[2])+2);
            histogramRaw[bin[2]] += temp[3];
        }

        for (int k = 0; k < NBINS*3; ++k)
        {
            histogramRaw[k] /= divider;
        }

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

    Eigen::Vector2f regionDimensions;
    Eigen::Vector2f center;
    Eigen::Vector2i frameViewport;

    int lineSize;

    Tucano::Mesh quad;
};

}



#endif