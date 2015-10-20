#version 430

in vec4 gl_FragCoord;

out vec4 out_Color;
uniform sampler2D frameTexture;
uniform ivec2 dimensions;
uniform ivec2 lowerCorner;

#define NBINS 32

float dist2(vec2 a, vec2 b)
{
	return (a.x-b.x)*(a.x-b.x) + (a.y-b.y)*(a.y-b.y);
}

void main()
{
	out_Color = vec4(0.0);

	ivec2 position = ivec2(gl_FragCoord.xy);
	if(position.x > lowerCorner.x && position.x < (lowerCorner.x+dimensions.x) && position.y > lowerCorner.y && position.y < (lowerCorner.y+dimensions.y))
	{
		out_Color = texelFetch(frameTexture, position, 0);
	}
}
