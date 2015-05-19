#version 430

uniform sampler2D imageTexture;
uniform ivec2 viewportSize;

out vec4 out_Color;

void main()
{
  vec2 texCoord = vec2(gl_FragCoord.xy/vec2(viewportSize.xy));
  float rR = texture(imageTexture, texCoord+vec2(0.01,0)).r;
  float gG = texture(imageTexture, texCoord+vec2(0,0.01)).g;
  float bB = texture(imageTexture, texCoord-vec2(0.01,0)).b;
  out_Color = vec4(rR, gG, bB, 1.0);
}
