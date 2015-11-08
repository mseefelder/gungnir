#version 430

uniform ivec2 SWsize; //in pixel
uniform ivec2 SWcorner; //in frame coords
uniform ivec2 totalSize; //of SWsize.x*ROIsize.x x ...
uniform ivec2 frameSize;//in pixels
uniform ivec2 ROIsize;// in pixels
uniform int rambits;

layout (binding = 0) buffer trackInfo
{
	int nRam;
	int nPixel;
	int nope;
	ivec3 avgPixel;
	int descriptor[];
};

layout (binding = 1) buffer maskBuffer
{
	int mask[];
};

layout (binding = 2) buffer scoreBuffer
{
	int score[];
};

out vec4 out_Color;

ivec2 decodeAddress(int coded)
{
	int a = coded%ROIsize.x;
	return ivec2(
		a,
		(coded-a)/ROIsize.x
		);
}

// http://lolengine.net/blog/2013/07/27/rgb-to-hsv-in-glsl
void main()
{
	ivec2 texCoord = ivec2(gl_FragCoord.xy);
	int index = texCoord.x + (texCoord.y * totalSize.x);
	
	int a, b, ram, total;
	total = SWsize.x*SWsize.y;
	a = (index%total)%SWsize.x; //mapped to search window  coordinates
	b = ((index%total)-a)/SWsize.x; //mapped to search window coordinates
	ram = ((index-a)-(b*SWsize.x))/total;
	
	ivec2 thisRegionCornerInSW = ivec2(a,b); 
	ivec2 thisRegionCorner = thisRegionCornerInSW + SWcorner;//mapped to frame coordinates
	
	bool matches = true;
	ivec2 bitSearchCoord = ivec2(0);
	int bitSearchIndex = 0;
	int i = 0;

	for(i=0;i<rambits;i++)
	{
		ivec2 bitSearchCoord = decodeAddress(mask[(ram*rambits)+i]) + thisRegionCorner; //mapped to frame coordinates
																					//moves corner to searched bit
		int bitSearchIndex = bitSearchCoord.x + bitSearchCoord.y*frameSize.x; //mapped to binarized frame array
		matches = matches && (mask[(2*nPixel)+bitSearchIndex]==descriptor[(ram*rambits)+i]);		
	}

	if(matches){
		int addIndex = thisRegionCornerInSW.x + thisRegionCornerInSW.y*SWsize.x;
		atomicAdd(score[addIndex],1);
	}
	
	memoryBarrier();

	discard;
}
	/**
	int normalizedAddress = texCoord.x + texCoord.y*(viewport.x);

	int codedAddress;
	bool matches = true;

	codedAddress = mask[rambits*ram];
	ivec2 getCoord = ivec2(
		codedAddress%dimensions.x,
		int(floor(codedAddress/float(dimensions.y)))
		)
		+ texCoord;
	int getAddress = getCoord.x + getCoord.y*viewport.x;
	matches = matches && (mask[nPixel+getAddress]==descriptor[rambits*ram]);

	codedAddress = mask[rambits*ram+1];
	getCoord = ivec2(
		codedAddress%dimensions.x,
		int(floor(codedAddress/float(dimensions.y)))
		)
		+ texCoord;
	getAddress = getCoord.x + getCoord.y*viewport.x;
	matches = matches && (mask[nPixel+getAddress]==descriptor[rambits*ram+1]);

	codedAddress = mask[rambits*ram+2];
	getCoord = ivec2(
		codedAddress%dimensions.x,
		int(floor(codedAddress/float(dimensions.y)))
		)
		+ texCoord;
	getAddress = getCoord.x + getCoord.y*viewport.x;
	matches = matches && (mask[nPixel+getAddress]==descriptor[rambits*ram+2]);

	if(matches)
		atomicAdd(score[normalizedAddress],1);
		//atomicAdd(mask[nPixel+(viewport.x*viewport.y)+normalizedAddress],1);

	discard;
	/**/

	/**
	int i = 0;
	int counter = 0;
	int score = 0;
	bool matches = true;
	int codedAddress;
	for(i = 0; i<nPixel; i++)
	{
		//compare
		codedAddress = mask[i];
		matches = matches && (mask[thisAddress+codedAddress] == descriptor[i]);
		//increment counter
		counter += 1;
		//check if ram is complete
		if(counter==rambits)
		{
			score += matches?1:0;
			matches = true;
			counter = 0;
		}
	}
	//if number of pixels not multiple of rambits, last RAM is incomplete
	if(counter > 0)
	{
		score += matches?1:0;
	}
	atomicExchange(mask[thisAddress+2*nPixel],score);

	discard;
	/**/
