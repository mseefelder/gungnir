#ifndef __TRACKER__
#define __TRACKER___

#include <tucano.hpp>

/**
 * @brief Picks a 3D position from screen position
 */
class Tracker : public Tucano::Effect
{

public:
    virtual void initialize (void) = 0;

    virtual void firstFrame (Eigen::Vector2i viewport, Eigen::Vector2i firstCorner, Eigen::Vector2i spread, Tucano::Texture* frame) = 0;

    virtual void track (Tucano::Texture* frame, Eigen::Vector2i* corner, Eigen::Vector2i* spread, int itermax) = 0;

private:

};

#endif