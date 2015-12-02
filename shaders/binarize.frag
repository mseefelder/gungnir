#version 430

uniform sampler2D frameTexture;
uniform ivec2 dimensions;
uniform ivec2 lowerCorner;

layout (binding = 0) buffer trackInfo
{
	int nPixel;
	ivec3 avgPixel;
	int descriptor[];
};

out vec4 out_Color;

void main()
{
	ivec2 texCoord = ivec2(gl_FragCoord.xy);
	
	if(texCoord.x>=lowerCorner.x && texCoord.x<=(lowerCorner.x+dimensions.x) && texCoord.y>=lowerCorner.y && texCoord.y<=(lowerCorner.y+dimensions.y))
	{
		vec3 result = texelFetch(frameTexture, texCoord, 0).rgb;
		ivec3 resultInt = ivec3(255.*result);
		atomicAdd(avgPixel.x, resultInt.x);
		atomicAdd(avgPixel.y, resultInt.y);
		atomicAdd(avgPixel.z, resultInt.z);
		atomicAdd(nPixel, 1);
	}

    discard;
}
