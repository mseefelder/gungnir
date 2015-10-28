#version 430

uniform sampler2D imageTexture;
uniform ivec2 viewportSize;
uniform float frameNorm;

out vec4 out_Color;

void main()
{
	vec2 texCoord = vec2(gl_FragCoord.xy/vec2(viewportSize.xy));
	vec3 result = texture(imageTexture, texCoord).rgb;
	float resNorm = (max(result.z,max(result.x,result.y))+min(result.z,min(result.x,result.y)))/2.0;//length(result);
	out_Color = vec4(0.,0.,0.,1.0);
	if(resNorm > (frameNorm)/255.)
		out_Color = vec4(1.0);
  //out_Color = vec4(result, 1.0);
}
