/**
 * Tucano - A library for rapid prototying with Modern OpenGL and GLSL
 * Copyright (C) 2014
 * LCG - Laboratório de Computação Gráfica (Computer Graphics Lab) - COPPE
 * UFRJ - Federal University of Rio de Janeiro
 *
 * This file is part of Tucano Library.
 *
 * Tucano Library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Tucano Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Tucano Library.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __DRAWRECTANGLE__
#define __DRAWRECTANGLE__

#include <tucano.hpp>

using namespace std;

using namespace Tucano;

namespace Effects
{

/**
 * @brief A simple effect to render a texture.
 **/
class drawRectangle : public Effect
{
public:
    /**
     * @brief Default Constructor.
     */
    drawRectangle (void)
    {
    }

    /**
     * @brief Deafult empty destructor.
     */
    ~drawRectangle (void) {}

    /**
     * @brief Initializes the effect, creating and loading the shader.
     */
    virtual void initialize()
    {
		loadShader(shader, "rectangle");
        quad.createQuad();
    }

    /**
     * @brief Renders the given texture.
     *
     * Renders the given texture using a proxy geometry, a quad the size of the viewport
     * to hold the texture.
     */
    void renderTexture (Eigen::Vector2i &viewport, Eigen::Vector2i &firstCorner, Eigen::Vector2i &spread)
    {
        /**/
        internalFirstCorner = Eigen::Vector2f(
       ((2*((float)firstCorner[0]/(float)viewport[0]))-1.0)
       , 
       ((2*((float)firstCorner[1]/(float)viewport[1]))-1.0)
       );

       internalSpread = Eigen::Vector2f(
       ((2*((float)spread[0]/(float)viewport[0]))-1.0)
       , 
       ((2*((float)spread[1]/(float)viewport[1]))-1.0)
       );

        setWarpMatrix(&internalFirstCorner, &internalSpread);
        /**/
        
        //setWarpMatrix(viewport, firstCorner, spread);
        
        glViewport(0, 0, viewport[0], viewport[1]);

        shader.bind();
        shader.setUniform("warpMatrix", warpQuad);
        GLint previousMode;
        glGetIntegerv(GL_POLYGON_MODE, &previousMode);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        quad.render();
        glPolygonMode(GL_FRONT_AND_BACK, previousMode);

        shader.unbind();
    }

    /**/
    void setWarpMatrix (Eigen::Vector2f* firstCorner, Eigen::Vector2f* spread)
    {
        warpQuad = Eigen::Matrix4f::Zero();

        //Scale
        scale << abs((*spread)[0]-(*firstCorner)[0])/2.0, abs((*spread)[1]-(*firstCorner)[1])/2.0;
        warpQuad(0,0) = scale[0];//-(*firstCorner)[0]; //x
        warpQuad(1,1) = scale[1];//-(*firstCorner)[1]; //y
        warpQuad(2,2) = 1;
        //Translation
        warpQuad(0,3) = (*firstCorner)[0] + (scale[0]);//(2*(*firstCorner)[0]-1.0) + (scale[0]);//(0.25 + (*firstCorner)[0])-0.5; //x OK
        warpQuad(1,3) = (*firstCorner)[1] - (scale[1]);//(-2*(*firstCorner)[1]+1.0) - (scale[1]);//((1.0-(*firstCorner)[1])-0.25)-0.5; //y
        //uniform matrix value
        warpQuad(3,3) = 1;
    }
    /**/

    /*
    void setWarpMatrix (Eigen::Vector2i &viewport, Eigen::Vector2i &firstCorner, Eigen::Vector2i &spread)
    {
        warpQuad = Eigen::Matrix4f::Zero();

        //Scale
        scale << abs((spread)[0]-(firstCorner)[0])/(viewport[0]*1.0), abs((spread)[1]-(firstCorner)[1])/(viewport[1]*1.0);
        warpQuad(0,0) = scale[0];//-(*firstCorner)[0]; //x
        warpQuad(1,1) = scale[1];//-(*firstCorner)[1]; //y
        warpQuad(2,2) = 1;
        //Translation
        warpQuad(0,3) = firstCorner[0]/(0.5*viewport[0])-1.0;//(*firstCorner)[0] + (scale[0]);//(2*(*firstCorner)[0]-1.0) + (scale[0]);//(0.25 + (*firstCorner)[0])-0.5; //x OK
        warpQuad(1,3) = firstCorner[1]/(0.5*viewport[1])-1.0;//(*firstCorner)[1] - (scale[1]);//(-2*(*firstCorner)[1]+1.0) - (scale[1]);//((1.0-(*firstCorner)[1])-0.25)-0.5; //y
        //uniform matrix value
        warpQuad(3,3) = 1;
    }
    */

private:

    /// The square rendering shader.
    Shader shader;

    /// A quad to be rendered forcing one call of the fragment shader per image pixel (its just a proxy geometry)
    Mesh quad;

    /// Matrix to scale and translate the quad on screen coordinates
    Eigen::Matrix4f warpQuad;
    Eigen::Vector2f scale;
    Eigen::Vector2f internalSpread;
    Eigen::Vector2f internalFirstCorner;
};

}

#endif
