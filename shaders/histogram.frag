#version 430

in vec4 gl_FragCoord;

out vec4 out_Color;
uniform sampler2D pValues;
uniform ivec2 dimensions;

#define NBINS 32

void main()
{	
	
    vec4 value = vec4(0.0, 0.0, 0.0, 1.0);
    vec4 bin;

    int thisBin = int(gl_FragCoord.y);

    int i = 0;
    int j = 0;
    
    
    for (i = 0; i<dimensions.x; i++)
    {
        for (j  = 0; j<dimensions.y; j++)
        {
            bin = texelFetch(pValues, ivec2(i, j), 0);
            //bin = texture(pValues, vec2(1.0*i/dimensions.x, 1.0*j/dimensions.y));

            if(int(bin.r) == thisBin)//bin.r > gl_FragCoord.y && bin.r > 1.0+gl_FragCoord.y)//bin.r == 1.0*gl_FragCoord.y)//
            {
                value.r += bin.a;
                //value.r+=1.0;
            }
            if(int(bin.g) == thisBin)//bin.g > gl_FragCoord.y && bin.r > 1.0+gl_FragCoord.y)//
            {
                value.g += bin.a;
                //value.g+=1.0;
            }
            if(int(bin.b) == thisBin)//bin.b > gl_FragCoord.y && bin.r > 1.0+gl_FragCoord.y)//
            {
                value.b += bin.a;
                //value.b+=1.0;
            }
        }
    }
    
    
    //vec4 pix = texelFetch(pValues, ivec2( thisBin/4, mod(thisBin,4)), 0);
    //value = pix;
    //value = vec4(float(thisBin/4), float(mod(thisBin,4)), pix, 0.0);
    //value = vec4(vec2(dimensions), pix, 0.0);
    

	out_Color = value;
    //out_Color = vec4(32.2, 42, 42, 1.0);
}
