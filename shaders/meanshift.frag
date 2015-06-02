#version 430

in vec4 gl_FragCoord;
out vec4 out_Color;
uniform sampler2D pvalues;
uniform vec2 center;
uniform ivec2 dimensions;
uniform sampler2D pHistogram;
uniform sampler2D qHistogram;
uniform ivec2 viewport;
uniform float dividerP;
uniform float dividerQ;

float dist2(vec2 a, vec2 b)
{
	return (a.x-b.x)*(a.x-b.x) + (a.y-b.y)*(a.y-b.y);
}

void main()
{
	//Pixel position on region of interest, mapped to whole frame:
	vec2 position = vec2(0.0);
	position.x = (gl_FragCoord.x + ((center.x+1.0)/2.0) * viewport.x - dimensions.x/2.0)/(viewport.x*1.0);
	position.y = (gl_FragCoord.y + ((center.y+1.0)/2.0) *viewport.y - dimensions.y/2.0)/(viewport.y*1.0);

	vec4 pvalue = texture(pvalues, vec2(gl_FragCoord.xy), 0);

	float weight = 0.0;
	weight += sqrt((texelFetch(pHistogram, ivec2(1, int(pvalue.r)), 0).a/dividerP)/(texelFetch(qHistogram, ivec2(1, int(pvalue.r)), 0).a/dividerQ));
	weight += sqrt((texelFetch(pHistogram, ivec2(1, int(pvalue.g)), 0).a/dividerP)/(texelFetch(qHistogram, ivec2(1, int(pvalue.g)), 0).a/dividerQ));
	weight += sqrt((texelFetch(pHistogram, ivec2(1, int(pvalue.b)), 0).a/dividerP)/(texelFetch(qHistogram, ivec2(1, int(pvalue.b)), 0).a/dividerQ));

	float normalizer = weight*exp(-0.5*dist2(center, position)/(dimensions.x*dimensions.x/16.0));
	float xComp = position.x*normalizer;
	float yComp = position.y*normalizer;

  	out_Color = vec4(xComp, yComp, normalizer, 1.0);
  	//out_Color = vec4(weight);
}
