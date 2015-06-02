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
    for (i; i<dimensions.x; i++)
    {
        for (j; j<dimensions.y; j++)
        {
            //bin = texelFetch(pValues, ivec2(i, j), 0);
            bin = texture(pValues, vec2(1.0*i/dimensions.x, 1.0*j/dimensions.y));

            if(int(bin.r) == int(gl_FragCoord.y))//bin.r > gl_FragCoord.y && bin.r > 1.0+gl_FragCoord.y)//bin.r == 1.0*gl_FragCoord.y)//
            {
                value.r += bin.a;
                //value.r+=1.0;
            }
            if(int(bin.g) == int(gl_FragCoord.y))//bin.g > gl_FragCoord.y && bin.r > 1.0+gl_FragCoord.y)//
            {
                value.g += bin.a;
                //value.g+=1.0;
            }
            if(int(bin.b) == int(gl_FragCoord.y))//bin.b > gl_FragCoord.y && bin.r > 1.0+gl_FragCoord.y)//
            {
                value.b += bin.a;
                //value.b+=1.0;
            }
        }
    }

	out_Color = value;
    //out_Color = vec4(32.2, 42, 42, 1.0);
}
