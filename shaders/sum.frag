#version 430

in vec4 gl_FragCoord;

out vec4 out_Color;
uniform sampler2D pvalues;
uniform ivec2 dimensions;
uniform bool widthIsMax;

void main()
{
	
	int k = 0;
	vec3 components = vec3(0.0);
	if(widthIsMax)
	{
		for(k = 0; k<dimensions.y; k++)
		{
			components += texture(pvalues, vec2(gl_FragCoord.y/dimensions.y, k/dimensions.y)).rgb;//texelFetch(pvalues, ivec2(gl_FragCoord.y, k), 0).rgb;
		}
	}
	else
	{
		for(k = 0; k<dimensions.x; k++)
		{
			components += texture(pvalues, vec2(k/dimensions.y, gl_FragCoord.y/dimensions.y)).rgb;//texelFetch(pvalues, ivec2(k, gl_FragCoord.y), 0).rgb;
		}
	}
	
  	out_Color = vec4(components, 1.0);

  	//out_Color = vec4(4.0);

  	/*
  	if(widthIsMax)
  		out_Color = vec4(2.0);
  	else
  		out_Color = vec4(3.0);
	*/
}
