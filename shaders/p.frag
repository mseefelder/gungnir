#version 430

in vec4 gl_FragCoord;

out vec4 out_Color;
uniform sampler2D frameTexture;
uniform vec2 center;
uniform ivec2 dimensions;
uniform ivec2 viewport;

#define NBINS 16

void main()
{	
	//Pixel position on region of interest, mapped to whole frame:
	vec2 position = vec2(0.0);
	//position.x = gl_FragCoord.x/viewport.x + (center.x-((dimensions.x/viewport.x)/2));
	position.x = (gl_FragCoord.x + center.x*viewport.x - dimensions.x/2.0)/(viewport.x*1.0);
	//position.y = gl_FragCoord.y/viewport.y + (center.y-((dimensions.y/viewport.y)/2));
	position.y = (gl_FragCoord.y + center.y*viewport.y - dimensions.y/2.0)/(viewport.y*1.0);

	//Pixel values:
	vec4 pixel = texture(frameTexture, position);

	//In wich histogram bin each color is:
	vec3 bin = vec3(1.0*ivec3(255*pixel.xyz/(256/NBINS)));

	//Kernel value:
	float value = exp( 0.5*distance(center, position)/(dimensions.x*dimensions.x/16.0) );

	//Output
	//out_Color = vec4(bin.xyz, value);
	out_Color = 255*pixel;
	//out_Color = vec4(position, center);
	//out_Color = vec4(vec3(2.0), 0.5);
}
