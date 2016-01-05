#ifndef __WISARD__
#define __WISARD___

#include <tucano.hpp>

#define RAMBITS 4 //work with 4 bit RAM
//#define DEBUGVIEW

/**
 * @brief Weightless nearural network based tracker
 * 
 */
class Wisard : public Tucano::Effect
{

public:

	Wisard() 
	{
		dummyFbo = NULL;
		//Load all shaders
		loadShader(constantShader, "constantShader");
		loadShader(binarizeShader, "binarizeShader");
		loadShader(classifierShader, "classifierShader");
		loadShader(discriminatorShader, "discriminatorShader");
	}

	~Wisard() 
	{}

	/**
	*@brief Must be called for the first frame of tracking, instead of track()
	*/
	virtual void startTracking(Eigen::Vector2i &viewportDimensions, Eigen::Vector2i &targetLowerCorner, Eigen::Vector2i &targetDimensions, int numberSearchRegions)
	{
		setup(viewportDimensions, targetLowerCorner, targetDimensions, numberSearchRegions);
		generateBinarizationConstant();
		createDiscriminator();
	}

	/**
	*@brief Track previous frame's target in current frame
	*/
	virtual track(Eigen::Vector2i &targetLocation)
	{
		binarizeFrame();
		classifyCandidates();
		findTarget(targetLocation);
	}


private:

	Eigen::Vector2i targetCorner;
	Eigen::Vector2i targetSpread;
	Eigen::Vector2i viewportSize;

	Tucano::Shader constantShader;
	Tucano::Shader binarizeShader;
	Tucano::Shader discriminatorShader;
	Tucano::Shader classifierShader;

	ShaderStorageBufferInt* constantTrackBuffer;
	/*contains:
		binarization constant
		discriminator mask
		inverse discriminator mask
		discriminator
	*/
	ShaderStorageBufferInt* changingTrackBuffer;
	/*contains:
		binarized frame
		candidate score array
	*/

	Tucano::Framebuffer* dummyFbo;

	int* discriminatorMask;
	int* inverseDiscriminatorMask;

	int numberPixels;

	/**
	*@ brief Sets tracker up
	*@param viewportDimensions << viewport width, viewport height
	*@param targetLowercorner << coordinates from the target that are the closest to 0,0
	*@param targetDimensions << target width, target height
	*@param numberSearchRegions defines how many times the search region will be bigger than the target.
	*	Example: 4 implies that the search region will be equivalent to 4 targets in width and in height
	*/
	virtual void setup (Eigen::Vector2i &viewportDimensions, Eigen::Vector2i &targetLowerCorner, Eigen::Vector2i &targetDimensions, int numberSearchRegions) 
	{
		//For how many pixels does the target spread
		targetSpread = targetDimensions;
		//Where is the lower valued corner located (nearest to 0,0)
		targetCorner = targetLowerCorner;
		//Size of camera frame in pixels
		viewportSize = viewportDimensions;
		//Number of pixels in target
		numberPixels = targetDimensions[0]*targetDimensions[1];

		//Fbo used in parallel classification
		//Delete if already exists
		if (dummyFbo)
		{
			dummyFbo->destroy();
			delete dummyFbo;
		}
		//Create new
		dummyFbo = new Tucano::Framebuffer(tragetSpread[0]*numberSearchRegions, targetSpread[1]*numberSearchRegions, 1, GL_TEXTURE_2D, GL_R8, GL_RED, GL_UNSIGNED_BYTE);

		//Create masks
		try
		{
			createDiscriminatorMask();
		}
		catch (exception& e)
        {
        	throw;
        }

        //create arrays for shader storage buffers
        int constantSize = 1 + 3*numberPixels;
        int changingSize = (viewportSize[0]*viewportSize[1]) + (numberSearchRegions*numberSearchRegions);
        int constantTrackBufferArray[constantSize];
        int changingTrackBufferArray[changingSize];

        //zero out the arrays
        for (int i = 0; i < constantSize; ++i)
        {
        	constantTrackBufferArray[i] = 0;
        }
        for (int i = 0; i < changingSize; ++i)
        {
        	changingTrackBufferArray[i] = 0;
        }

        //place masks into constantTrackBufferArray
        for (int i = 0; i < numberPixels; ++i)
        {
        	constantTrackBufferArray[1+i] = discriminatorMask[i];
        	constantTrackBufferArray[1+(numberPixels)+i] = inverseDiscriminatorMask[i];
        }

        //create buffers
        constantTrackBuffer = new ShaderStorageBufferInt(constantSize, constantTrackBufferArray);
        changingTrackBuffer = new ShaderStorageBufferInt(changingSize, changingTrackBufferArray);

	}

	/**
	*@brief Creates a pseudo-random, repetition-free, mask of integers (from 0 to the number of pixels in the target -1) to map RAM bits to pixels
	*Also creates the inverted mask to map from the pixels to RAM bits
	*@param debug If true, function prints the masks
	*/
	virtual void createDiscriminatorMask( bool debug ) 
	{
		if (numberPixels > 0)
		{
			discriminatorMask = new int[numberPixels];
			inverseDiscriminatorMask = new int[numberPixels];
		}
		else
		{
			targetException targetEx;
			throw targetEx;
		}

		//create array with all indexes
		int indexes[numberPixels];
		for (int i = 0; i < numberPixels; ++i)
		{
			indexes[i] = i;
		}

        std::random_device rd; // obtain a random number from hardware
        std::mt19937 eng(rd()); // seed the generator
        std::uniform_int_distribution<> distr(0, numberPixels-1); // define the range
        int currIndex, mistery;
        for (int i = 0; i < numberPixels; ++i)
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
            discriminatorMask[i] = currIndex;
            inverseDiscriminatorMask[currIndex] = i;
            indexes[mistery] = -1;
        }

        if (debug)
        {
        	std::cout<<" discriminatorMask: ["
        	for (int i = 0; i < numberPixels; ++i)
        	{
        		std::cout<<discriminatorMask[i]<<", "
        	}
        	std::cout<<"] \n inverseDiscriminatorMask: [";
        	for (int i = 0; i < numberPixels; ++i)
        	{
        		std::cout<<inverseDiscriminatorMask[i]<<", "
        	}
        	std::cout<<"]"<<std::endl;
        }

        return;
	}

	/**
	*@brief Binarization constant = target's mean luminance
	*Does it on shader, final result is storing a binarization constant on a shader storage buffer
	*/
	virtual void generateBinarizationConstant()
	{}

	/**
	*@brief For each pixel, if luminance > binarization contstant: white; else: black
	*Does it on shader, final result is filling the shader storage buffer with the binarized frame
	*/
	virtual void binarizeFrame()
	{}

	/**
	*@brief Create descriptor for target to be followed
	*Does it on shader, uses the descriptor mask to create a descriptor based on the binarized target
	*/
	virtual void createDiscriminator()
	{}

	/**
	*@brief Attributes a score to each candidate in the search region
	*Does it on shader
	*/
	virtual void classifyCandidates()
	{}

	/**
	*@brief After classifying the candidates, returns the new target location
	*/
	virtual void findTarget (Eigen::Vector2i &targetLocation)
	{}

};

#endif