#version 430

in vec4 gl_FragCoord;

out vec4 out_Color;
uniform sampler2D pValues;
uniform ivec2 dimensions;

#define NBINS 16

void main()
{	

	vec4 value = vec4(0.0, 0.0, 0.0, 1.0);
    vec4 bin;

    int i = 0;
    int j = 0;
    for (i=0; i<dimensions.x; i++)
    {
        for (j=0; j<dimensions.y; j++)
        {
            bin = texelFetch(pValues, ivec2(i, j), 1);

            if(int(bin.r) == gl_FragCoord.y)
            {
                value.r += bin.a;
            }
            if(int(bin.g) == gl_FragCoord.y)
            {
                value.g += bin.a;
            }
            if(int(bin.b) == gl_FragCoord.y)
            {
                value.b += bin.a;
            }
        }
    }

	out_Color = value;
    //out_Color = vec4(32.2, 42, 42, 1.0);
}
