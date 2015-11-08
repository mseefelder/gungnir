#version 430

uniform sampler2D frameTexture;
uniform ivec2 dimensions;
uniform ivec2 viewport;
uniform ivec2 SWsize;

layout (binding = 0) buffer trackInfo
{
	int nRam;
	int nPixel;
	int avgLuminance;
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
vec3 rgb2hsv(vec3 c)
{
    vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    vec4 p = c.g < c.b ? vec4(c.bg, K.wz) : vec4(c.gb, K.xy);
    vec4 q = c.r < p.x ? vec4(p.xyw, c.r) : vec4(c.r, p.yzx);

    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;
    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}
vec3 hsv2rgb(vec3 c)
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

void main()
{
	ivec2 texCoord = ivec2(gl_FragCoord.xy);
	int thisAddress = texCoord.x + texCoord.y*viewport.x;
	
	/**
	vec3 result = texelFetch(frameTexture, texCoord, 0).rgb;
	vec3 resultHsv = rgb2hsv(result);
	
	vec3 avgHsv = rgb2hsv(vec3(avgPixel/(float(nPixel)*255.)));

	float isok = (resultHsv.z<avgHsv.z)?1.0:0.0;
	float score = (score[thisAddress]==0)?1.0:0.0;//float(mask[thisAddress+2*nPixel])/(float(nPixel)/3.0);
	vec3 colore = vec3(isok, score, 0.0);
	**/

	//out_Color = vec4(vec3(avgPixel/(float(nPixel)*255.)),1.0);
	//out_Color = vec4(colore,1.0);
	//out_Color = (thisAddress<(dimensions.x*dimensions.y))?vec4(1.0):vec4(vec3(0.0),1.0);
	
	//render descriptor
	/**
	int a = (thisAddress<(dimensions.x*dimensions.y))?thisAddress:0;
	out_Color = (descriptor[a]==1)?vec4(1.0):vec4(vec3(0.0),1.0);
	**/

	//render mask
	/**
	int a = (thisAddress<(nPixel))?thisAddress:0;
	float value = float(mask[a])/float(nPixel);
	out_Color = vec4(vec3(value),1.0);
	**/

	//render binarized
	/**
	float value = float(mask[nPixel+thisAddress]);
	out_Color = vec4(vec3(value),1.0);
	**/

	//render score
	/**/
	//out_Color = vec4(vec3(1.0,0.0,0.0),1.0);
	//if(texCoord.x<=SWsize.x && texCoord.y<=SWsize.y)
	//{
		thisAddress = texCoord.x + texCoord.y*(viewport.x);
		float value = score[thisAddress]/float(nPixel/3);
		out_Color = vec4(vec3(value),1.0);
		//out_Color = (score[thisAddress]>0)?vec4(1.0):vec4(vec3(0.0),1.0);
	//}
	/**/

	//out_Color = (resultHsv.z<avgHsv.z)?vec4(1.0):vec4(0.,0.,0.,1.);
}
