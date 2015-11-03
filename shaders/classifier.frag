#version 430

uniform int height;

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

out vec4 out_Color;

// http://lolengine.net/blog/2013/07/27/rgb-to-hsv-in-glsl
void main()
{
	ivec2 texCoord = ivec2(gl_FragCoord.xy);

	int thisAddress = texCoord.x + texCoord.y*height;
	int i = 0;
	int counter = 0;
	int score = 0;
	bool matches = true;
	int codedAddress;
	for(i = 0; i<nPixel; n++)
	{
		//compare
		codedAddress = mask[nPixel+i];
		matches = matches && (mask[thisAddress+codedAddress] == descriptor[thisAddress+codedAddress]);
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

	discard;

}
