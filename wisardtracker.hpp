#ifndef __WISARD__
#define __WISARD___

#include <tucano.hpp>

#define RAMBITS 4
//#define DEBUGVIEW

/**
 * @brief Picks a 3D position from screen position
 */
class Wisard : public Tucano::Effect
{

public:

	Wisard() {}

	~Wisard() {}

	/**
	*@ brief Sets tracker up
	*@param viewportDimensions << viewport width, viewport height
	*@param targetLowercorner << coordinates from the target that are the closest to 0,0
	*@param targetDimensions << target width, target height
	*@param numberSearchRegions defines how many times the search region will be bigger than the target
	*	Example: 4 implies that the search region will be equivalent to 4 targets in width and in height
	*/
	virtual void setup (Eigen::Vector2i &viewportDimensions, Eigen::Vector2i &targetLowerCorner, Eigen::Vector2i &targetDimensions, int numberSearchRegions) {

	}

	/**
	*@brief Creates a pseudo-random, repetition-free, mask of integers (from 0 to the number of pixels in the target -1) to map RAM bits to pixels
	*Also creates the inverted mask to map from the pixels to RAM bits
	*/
	virtual void createDiscriminatorMask() {}

	/**
	*@brief Track previous frame's target in current frame
	*/
	virtual void track (){}

	/**
	*@brief Binarization constant = target's mean luminance
	*Does it on shader, final result is storing a binarization constant on a shader storage buffer
	*/
	virtual void generateBinarizationConstant(){}

	/**
	*@brief For each pixel, if luminance > binarization contstant: white; else: black
	*Does it on shader, final result is filling the shader storage buffer with the binarized frame
	*/
	virtual void binarizeFrame(){}

	/**
	*@brief Create descriptor for target to be followed
	*Does it on shader, uses the descriptor mask to create a descriptor based on the binarized target
	*/
	virtual void createDiscriminator(){}

	/**
	*@brief Attributes a score to each candidate in the search region
	*Does it on shader
	*/
	virtual void classifyCandidates(){}


private:

	Eigen::Vector2i *targetCorner;
	Eigen::Vector2i *targetSpread;

};

#endif