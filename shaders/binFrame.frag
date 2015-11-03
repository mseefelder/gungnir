#version 430

uniform sampler2D frameTexture;
uniform int height;

layout (binding = 0) buffer trackInfo
{
	int nRam;
	int nPixel;
	int nope;
	ivec3 avgPixel;
	int maskDescriptor[];
};

layout (binding = 1) buffer maskBuffer
{
	int mask[];
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
	vec3 result = texelFetch(frameTexture, texCoord, 0).rgb;
	
	vec3 resultHsv = rgb2hsv(result);
	vec3 avgHsv = rgb2hsv(vec3(avgPixel/(float(nPixel)*255.)));

	int address = (texCoord.x+(height*texCoord.y));
	(resultHsv.z<avgHsv.z)?atomicExchange(mask[address], 1):atomicExchange(mask[address], 1);

}
