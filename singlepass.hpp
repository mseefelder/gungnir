#ifndef __SINGLEPASS__
#define __SINGLEPASS__

#include <tucano.hpp>

using namespace std;

using namespace Tucano;

/**
 * @brief A simple effect to render a texture.
 **/
class SinglePass : public Effect
{
public:
    /**
     * @brief Default Constructor.
     */
    SinglePass (void)
    {
    }

    /**
     * @brief Deafult empty destructor.
     */
    ~SinglePass (void) {}

    /**
     * @brief Initializes the effect, creating and loading the shader.
     */
    virtual void initialize()
    {
		loadShader(shader, "singlepass");
        quad.createQuad();
    }

    /**
     * @brief Renders the given texture.
     *
     * Renders the given texture using a proxy geometry, a quad the size of the viewport
     * to hold the texture.
     */
    void render (Texture& tex, Eigen::Vector2i viewport, double &frameNorm)
    {
        glViewport(0, 0, viewport[0], viewport[1]);

        shader.bind();
        shader.setUniform("imageTexture", tex.bind());
        shader.setUniform("viewportSize", viewport);
        shader.setUniform("frameNorm", (float)frameNorm);
        quad.render();

        shader.unbind();
        tex.unbind();
    }

private:

    /// The mean filter shader.
    Shader shader;

    /// A quad to be rendered forcing one call of the fragment shader per image pixel (its just a proxy geometry)
    Mesh quad;
};

#endif
