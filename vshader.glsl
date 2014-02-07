//#version 1.20
attribute vec4 vertex_position;//in
attribute vec4 vertex_color;
varying vec4 overtex_color;//out

uniform mat4 M;//modeling
uniform mat4 V;//viewing
uniform mat4 P;//projection

void main()  {
    //can't have M=mat4() here
    
	gl_Position = P*V*M*vertex_position;  // pass on the vertex position unchanged
    
    //gl_Position (built-in): holds the ouput of vetex shader.
    overtex_color=vertex_color;
}



//apply the modeling,viewing, and projection transformations in the vertex shader. !
