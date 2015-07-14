#version 430

in vec4 gl_FragCoord;
out vec4 out_Color;
uniform sampler2D pvalues;
uniform ivec2 center;
uniform ivec2 dimensions;
uniform sampler2D pHistogram;
uniform sampler2D qHistogram;
uniform ivec2 viewport;
//uniform float dividerP;
//uniform float dividerQ;
uniform ivec2 lowerCorner;

float dist2(vec2 a, vec2 b)
{
	return (a.x-b.x)*(a.x-b.x) + (a.y-b.y)*(a.y-b.y);
}

void main()
{
	//Pixel position on region of interest, mapped to whole frame:
	vec2 position = vec2(lowerCorner)+gl_FragCoord.xy;//vec2 position = vec2(ceil(gl_FragCoord.xy))+vec2(center)-vec2(dimensions)/2;
	ivec2 iposition = lowerCorner+ivec2(gl_FragCoord.xy);
	vec2 normPosition = vec2(position.x/float(viewport.x), position.y/float(viewport.y)) ;

	//Pixel values:
	//vec4 proof = 
	vec4 pvalue = texelFetch(pvalues, ivec2(gl_FragCoord.xy), 0);//texture(pvalues, normPosition);

	float weight = 0.0;
	// weight += sqrt((texelFetch(qHistogram, ivec2(0, int(pvalue.r)), 0).r/dividerQ)/(texelFetch(pHistogram, ivec2(0, int(pvalue.r)), 0).r/dividerP));
	// weight += sqrt((texelFetch(qHistogram, ivec2(0, int(pvalue.g)), 0).g/dividerQ)/(texelFetch(pHistogram, ivec2(0, int(pvalue.g)), 0).g/dividerP));
	// weight += sqrt((texelFetch(qHistogram, ivec2(0, int(pvalue.b)), 0).b/dividerQ)/(texelFetch(pHistogram, ivec2(0, int(pvalue.b)), 0).b/dividerP));

	weight += sqrt((texelFetch(qHistogram, ivec2(0, int(pvalue.r)), 0).r)/(texelFetch(pHistogram, ivec2(0, int(pvalue.r)), 0).r));
	weight += sqrt((texelFetch(qHistogram, ivec2(0, int(pvalue.g)), 0).g)/(texelFetch(pHistogram, ivec2(0, int(pvalue.g)), 0).g));
	weight += sqrt((texelFetch(qHistogram, ivec2(0, int(pvalue.b)), 0).b)/(texelFetch(pHistogram, ivec2(0, int(pvalue.b)), 0).b));

	//float normalizer = weight*exp( -0.5* (dist2(vec2(center), vec2(iposition))) / ( (float(dimensions.x)*float(dimensions.x)) /16.0) );
	float normalizer = weight*exp( -0.5 * (
		 ( (float(center.x)-float(iposition.x))*(float(center.x)-float(iposition.x))/((float(dimensions.x)*float(dimensions.x))/16.0) ) 
		 + 
		 ( (float(center.y)-float(iposition.y))*(float(center.y)-float(iposition.y))/((float(dimensions.y)*float(dimensions.y))/16.0) ) 
		 ) );
	float xComp = iposition.x*normalizer;
	float yComp = iposition.y*normalizer;

  	out_Color = vec4(xComp, yComp, normalizer, 1.0);
  	//out_Color = vec4(-0.5* (dist2(vec2(center), vec2(iposition))),( (float(dimensions.x)*float(dimensions.x)) /16.0), weight , normalizer);
  	//out_Color = texelFetch(pHistogram, ivec2(0, 0), 0);//vec4(weight);
}
