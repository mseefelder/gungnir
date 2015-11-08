#version 430

uniform ivec2 dimensions;
uniform ivec2 thisSize;
uniform ivec2 viewport;
uniform int rambits;
//uniform ivec2 cornerSR; //for fixed size

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
	int a = coded%dimensions.x;
	return ivec2(
		a,
		(coded-a)/dimensions.x
		);
}

// http://lolengine.net/blog/2013/07/27/rgb-to-hsv-in-glsl
void main()
{
	ivec2 texCoord = ivec2(gl_FragCoord.xy);
	int index = texCoord.x + (texCoord.y * thisSize.x);
	
	int a, b, ram, total;
	total = thisSize.x*thisSize.y;
	a = (index%total)%thisSize.x;
	b = ((index%total)-a)/thisSize.x;
	ram = ((index-a)-(b*thisSize.x))/total;
	
	ivec2 pointZero = ivec2(a,b);
	//pointZero = pointZero + cornerSR; //for fixed size 
	
	bool matches = true;

	ivec2 getCoord = pointZero + decodeAddress(mask[rambits*ram]);
	int getAddress = getCoord.x + getCoord.y*viewport.x;
	matches = matches && (mask[(2*nPixel)+getAddress] == descriptor[rambits*ram]);

	getCoord = pointZero + decodeAddress(mask[(rambits*ram)+1]);
	getAddress = getCoord.x + getCoord.y*viewport.x;
	matches = matches && (mask[(2*nPixel)+getAddress] == descriptor[(rambits*ram)+1]);

	getCoord = pointZero + decodeAddress(mask[(rambits*ram)+2]);
	getAddress = getCoord.x + getCoord.y*viewport.x;
	matches = matches && (mask[(2*nPixel)+getAddress] == descriptor[(rambits*ram)+2]);

	getCoord = pointZero + decodeAddress(mask[(rambits*ram)+2]);
	getAddress = getCoord.x + getCoord.y*viewport.x;
	matches = matches && (mask[(2*nPixel)+getAddress] == descriptor[(rambits*ram)+2]);

	if(matches){
		int fullAddress = pointZero.x + (pointZero.y*(viewport.x-dimensions.x));
		//int fullAddress = pointZero.x + (pointZero.y*viewport.x);
		atomicAdd(score[fullAddress],1);
	}

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
