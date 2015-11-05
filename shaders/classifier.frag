#version 430

uniform ivec2 dimensions;
uniform ivec2 viewport;

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

// http://lolengine.net/blog/2013/07/27/rgb-to-hsv-in-glsl
void main()
{
	ivec2 texCoord = ivec2(gl_FragCoord.xy);
	int thisAddress = texCoord.x + texCoord.y*(viewport.x-dimensions.x);
	int ram = int(floor(float(thisAddress)/float((viewport.x-dimensions.x)*(viewport.y-dimensions.y))));
	ivec2 normalizedTexCoord = ivec2(texCoord.x, texCoord.y-(ram*(viewport.y-dimensions.y)));
	int normalizedAddress = normalizedTexCoord.x + normalizedTexCoord.y*viewport.x;

	/**/

	int codedAddress;
	bool matches = true;

	codedAddress = mask[3*ram];
	ivec2 getCoord = ivec2(
		codedAddress%dimensions.x,
		int(floor(codedAddress/float(dimensions.y)))
		)
		+ normalizedTexCoord;
	int getAddress = getCoord.x + getCoord.y*viewport.x;
	matches = matches && mask[nPixel+getAddress]==descriptor[3*ram];

	codedAddress = mask[3*ram+1];
	getCoord = ivec2(
		codedAddress%dimensions.x,
		int(floor(codedAddress/float(dimensions.y)))
		)
		+ normalizedTexCoord;
	getAddress = getCoord.x + getCoord.y*viewport.x;
	matches = matches && mask[nPixel+getAddress]==descriptor[3*ram+1];

	codedAddress = mask[3*ram+2];
	getCoord = ivec2(
		codedAddress%dimensions.x,
		int(floor(codedAddress/float(dimensions.y)))
		)
		+ normalizedTexCoord;
	getAddress = getCoord.x + getCoord.y*viewport.x;
	matches = matches && mask[nPixel+getAddress]==descriptor[3*ram+2];

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
		if(counter==3)
		{
			score += matches?1:0;
			matches = true;
			counter = 0;
		}
	}
	//if number of pixels not multiple of 3, last RAM is incomplete
	if(counter > 0)
	{
		score += matches?1:0;
	}
	atomicExchange(mask[thisAddress+2*nPixel],score);

	discard;
	/**/

}
