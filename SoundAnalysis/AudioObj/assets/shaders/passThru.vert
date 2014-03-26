

void main()
{
	gl_TexCoord[0]  = gl_MultiTexCoord0;
//	gl_FrontColor	= vec4(1.0,0.0,0.0,1.0);       
	gl_FrontColor 	= gl_Color;				// doesn't work?!
    gl_Position     = ftransform();
}
