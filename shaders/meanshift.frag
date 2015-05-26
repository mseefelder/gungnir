#version 430

out vec4 out_Color;
uniform sampler2D pvalues;
uniform vec2 center;
uniform vec2 dimensions;
uniform sampler2D pHistogram;
uniform sampler2D qHistogram;
uniform ivec2 viewport;

void main()
{
  out_Color = vec4(1.0,1.0,1.0,1.0);
}
