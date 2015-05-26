#version 430

out vec4 out_Color;
uniform sampler2D pvalues;

void main()
{
  out_Color = vec4(0.00001, 0.00001, 1.0, 1.0);
}
