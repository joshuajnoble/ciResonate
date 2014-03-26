
void main()
{
	gl_TexCoord[0]  = gl_MultiTexCoord0;
//	gl_FrontColor 	= gl_Color[0];				// doesn't work?!
    gl_Position     = ftransform();
}
