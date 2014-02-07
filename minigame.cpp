// Basic OpenGL program
// Based on example code from Ed Angel's 6th edition textbook

#include <stdio.h>
#include <stdlib.h>
#include <GLUT/glut.h>
//#include <GL/glut.h>
#include <iostream>   //std::cout
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <cmath>     //asin   (arcsin)     abs for float is only in cmath rather than math.h
#include <algorithm>    // std::min
//#include "BasicTest.h"

#include "mat.h" //which include "vec.h" at the beginning.
//#include "Angel.h"

using namespace std;

#define DEBUG_ON 1
#define WINDOW_WIDTH 600
#define WINDOW_HEIGHT 600
#define ROTATE_ANGLE 4
#define BUFFER_OFFSET(bytes) ((GLvoid*) (bytes))
#define ABS(X) ((X)>0?(X):(-(X)))

typedef vec3 color3;
typedef vec3 point3;
typedef vec2 point2;

typedef vec4  color4;
typedef vec4  point4;

typedef struct {
	float x, y;
} FloatType2D;

typedef struct {
    float r,g,b;
} FloatType3D;

GLint m_location, v_location, p_location;

float x_pos, y_pos;
int mod;

double pi=4.0*atan(1.0);
//double theta=ROTATE_ANGLE*pi/180;// float s = M_PI/180.0*xangle; //convert degrees to radians, then get cos and sin.

double theta = pi/60;
double costheta=cos(theta);
double sintheta=sin(theta);

mat4 S,R,T,M;
mat4 R1;

GLfloat lef=-1,righ=1,bottom=-1, top=1, near=4,far=6;
GLfloat original_bottom=bottom,original_top=top,original_lef=lef,original_righ=righ;//used in reshape function

//mat4 P=Ortho(lef,righ,bottom,top,near,far);//orthographic projection
mat4 P;//defined at the end of init()

point4 eye=point4(0,0,5,1.0);
vec4 view_dir(0,0,-1,0);//(0 -1 -1)
vec4 view_dir_copy(view_dir);
vec4 newview_dir;
vec4 up_dir(0,1,0,0);
//-------from view transformation matrix function from mat.h--------
vec4 n = normalize(-view_dir);
vec4 u = normalize(cross(up_dir,n));
vec4 v = normalize(cross(n,u));
vec4 line4 = vec4(0.0, 0.0, 0.0, 1.0);
mat4 c = mat4(u, v, n, line4);
mat4 V=c * Translate( -eye );//V[0][3]=
//-------------------------------------------------------------------

const int nvertices = 36;//(6 faces)(2 triangles/face) (3 vertices/triangle)
point4 points[nvertices];
color4 colors[nvertices];
// Vertices of a unit cube centered at origin, sides aligned with axes
point4 vertices[8] = {
    point4( -0.5, -0.5,  0.5, 1.0 ),
    point4( -0.5,  0.5,  0.5, 1.0 ),
    point4(  0.5,  0.5,  0.5, 1.0 ),
    point4(  0.5, -0.5,  0.5, 1.0 ),
    point4( -0.5, -0.5, -0.5, 1.0 ),
    point4( -0.5,  0.5, -0.5, 1.0 ),
    point4(  0.5,  0.5, -0.5, 1.0 ),
    point4(  0.5, -0.5, -0.5, 1.0 )
};
// RGBA colors
color4 vertex_colors[8] = {
    color4( 0.0, 0.0, 0.0, 1.0 ),  // black
    color4( 1.0, 0.0, 0.0, 1.0 ),  // red
    color4( 1.0, 1.0, 0.0, 1.0 ),  // yellow
    color4( 0.0, 1.0, 0.0, 1.0 ),  // green
    color4( 0.0, 0.0, 1.0, 1.0 ),  // blue
    color4( 1.0, 0.0, 1.0, 1.0 ),  // magenta
    color4( 1.0, 1.0, 1.0, 1.0 ),  // white
    color4( 0.0, 1.0, 1.0, 1.0 )   // cyan
};

//----------------------------------------------------------------------
// quad generates two triangles for each face and assigns colors
//    to the vertices
int Index = 0;
void
quad( int a, int b, int c, int d )
{
    colors[Index] = vertex_colors[a]; points[Index] = vertices[a]; Index++;
    colors[Index] = vertex_colors[b]; points[Index] = vertices[b]; Index++;
    colors[Index] = vertex_colors[c]; points[Index] = vertices[c]; Index++;
    colors[Index] = vertex_colors[a]; points[Index] = vertices[a]; Index++;
    colors[Index] = vertex_colors[c]; points[Index] = vertices[c]; Index++;
    colors[Index] = vertex_colors[d]; points[Index] = vertices[d]; Index++;
}
//----------------------------------------------------------------------
// generate 12 triangles: 36 vertices and 36 colors
void
colorcube( void )
{
    quad( 1, 0, 3, 2 );
    quad( 2, 3, 7, 6 );
    quad( 3, 0, 4, 7 );
    quad( 6, 5, 1, 2 );
    quad( 4, 5, 6, 7 );
    quad( 5, 4, 0, 1 );
}
//----------------------------------------------------------------------





// Create a NULL-terminated string by reading the provided file
static char*
readShaderSource(const char* shaderFile)
{
    FILE *fp;
	long length;
	char *buffer;
	
	// open the file containing the text of the shader code
	fp = fopen(shaderFile, "r");
	
	// check for errors in opening the file
    if ( fp == NULL ) {
		printf("can't open shader source file %s\n", shaderFile);
		return NULL;
	}
	
	// determine the file size
    fseek(fp, 0, SEEK_END); // move position indicator to the end of the file;
    length = ftell(fp);  // return the value of the current position
	
	// allocate a buffer with the indicated number of bytes, plus one
	buffer = new char[length + 1];
	
	// read the appropriate number of bytes from the file
    fseek(fp, 0, SEEK_SET);  // move position indicator to the start of the file
    fread(buffer, 1, length, fp); // read all of the bytes
	
	// append a NULL character to indicate the end of the string
    buffer[length] = '\0';
	
	// close the file
    fclose(fp);
	
	// return the string
    return buffer;
}

// Create a GLSL program object from vertex and fragment shader files
GLuint
InitShader(const char* vShaderFileName, const char* fShaderFileName)
{
	GLuint vertex_shader, fragment_shader;
	GLchar *vs_text, *fs_text;
	GLuint program;
	
	// check GLSL version
	//printf("GLSL version: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));   //1.20
	
	// Create shader handlers
	vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	
	// Read source code from shader files
	vs_text = readShaderSource(vShaderFileName);
	fs_text = readShaderSource(fShaderFileName);
	
	// error check
	if ( vs_text == NULL ) {
		printf("Failed to read from vertex shader file %s\n", vShaderFileName);
		exit( 1 );
	} else if (DEBUG_ON) {
		printf("read shader code:\n%s\n", vs_text);
	}
	if ( fs_text == NULL ) {
		printf("Failed to read from fragent shader file %s\n", fShaderFileName);
		exit( 1 );
	} else if (DEBUG_ON) {
		printf("read shader code:\n%s\n", fs_text);
	}
	
	// Set shader source
	const char *vv = vs_text;
	const char *ff = fs_text;
	glShaderSource(vertex_shader, 1, &vv, NULL);
	glShaderSource(fragment_shader, 1, &ff, NULL);
	
	// Compile shaders
	glCompileShader(vertex_shader);
	glCompileShader(fragment_shader);
	
	// error check
	GLint  compiled;
	glGetShaderiv( vertex_shader, GL_COMPILE_STATUS, &compiled );
	if ( !compiled ) {
		printf("vertex_shader failed to compile\n");
		if (DEBUG_ON) {
			GLint logMaxSize, logLength;
			glGetShaderiv( vertex_shader, GL_INFO_LOG_LENGTH, &logMaxSize );
			printf("printing error message of %d bytes\n", logMaxSize);
			char* logMsg = new char[logMaxSize];
			glGetShaderInfoLog( vertex_shader, logMaxSize, &logLength, logMsg );
			printf("%d bytes retrieved\n", logLength);
			printf("error message: %s\n", logMsg);
			delete[] logMsg;
		}
		exit(1);
	}
	glGetShaderiv( fragment_shader, GL_COMPILE_STATUS, &compiled );
	if ( !compiled ) {
		printf("fragment_shader failed to compile\n");
		if (DEBUG_ON) {
			GLint logMaxSize, logLength;
			glGetShaderiv( fragment_shader, GL_INFO_LOG_LENGTH, &logMaxSize );
			printf("printing error message of %d bytes\n", logMaxSize);
			char* logMsg = new char[logMaxSize];
			glGetShaderInfoLog( fragment_shader, logMaxSize, &logLength, logMsg );
			printf("%d bytes retrieved\n", logLength);
			printf("error message: %s\n", logMsg);
			delete[] logMsg;
		}
		exit(1);
	}
	
	// Create the program
	program = glCreateProgram();
	
	// Attach shaders to program
	glAttachShader(program, vertex_shader);
	glAttachShader(program, fragment_shader);
	
    // Link and set program to use
	glLinkProgram(program);
	glUseProgram(program);
	
    return program;
}

float cubeVert[8][3] = {{-1.0, -1.0, 1.0}, {-1.0, 1.0, 1.0}, {1.0, 1.0, 1.0},
    {1.0, -1.0, 1.0}, {-1.0, -1.0, -1.0}, {-1.0, 1.0, -1.0},
    {1.0, 1.0, -1.0}, {1.0, -1.0, -1.0}};

float cubeColor[8][3] = {{0.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {1.0, 1.0, 0.0},
    {0.0, 1.0, 0.0}, {0.0, 0.0, 1.0}, {1.0, 0.0, 1.0},
    {1.0, 1.0, 1.0}, {0.0, 1.0, 1.0}};

int cubeIndices[24] =  {0, 3, 2, 1,
    2, 3, 7, 6,
    0, 4, 7, 3,
    1, 2, 6, 5,
    4, 5, 6, 7,
    0, 1, 5, 4};



mat4 rotateX_3_degrees=RotateX(7);
mat4 rotateX_minus_3_degrees=RotateX(-7);
mat4 rotateY_3_degrees=RotateY(7);
mat4 rotateY_minus_3_degrees=RotateY(-7);
mat4 rotateZ_3_degrees=RotateZ(7);
mat4 rotateZ_minus_3_degrees=RotateZ(-7);
void
init( void )//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
{
    GLuint vao[1], buffer, vPosition, vColor, program;
    R=mat4();// cumulative rotation matrix
    T=mat4();//cumulative translation matrix M=T*R
   // cout<<rotateX_minus_3_degrees<<endl;

    //cout<<rotateX_3_degrees<<endl;

   // cout<<V;
    
    //  if (DEBUG_ON) cout<<M<<endl;
    
    colorcube();
    program = InitShader("vshader36.glsl", "fshader36.glsl");
    glUseProgram(program);
    
    // Create a vertex array object
    glGenVertexArraysAPPLE( 1, vao );
    glBindVertexArrayAPPLE( vao[0] );
    
    // Create and initialize a buffer object to hold the vertex data
    glGenBuffers( 1, &buffer );
    glBindBuffer( GL_ARRAY_BUFFER, buffer );
	glBufferData( GL_ARRAY_BUFFER, sizeof(points)+sizeof(colors), NULL, GL_STATIC_DRAW );//modified for 2b
	
    glBufferSubData(GL_ARRAY_BUFFER,0,sizeof(points),points);//added for 2c-- from 0 to sizeof(points): first half of buffer
    glBufferSubData(GL_ARRAY_BUFFER,sizeof(points),sizeof(colors),colors);
    

    // Initialize the vertex position attribute from the vertex shader
	vPosition = glGetAttribLocation( program, "vertex_position" );//vertex_position is variable in vshader.glsl
    glEnableVertexAttribArray( vPosition );
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0) );
	
	vColor = glGetAttribLocation( program, "vertex_color" );
    glEnableVertexAttribArray( vColor );
    glVertexAttribPointer( vColor, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(points)) );
    
    glEnable(GL_DEPTH_TEST);
    
  
    m_location=glGetUniformLocation(program,"M");//Returns the index of the uniform variable name associated with the shader program.
    if (m_location==-1) {cout<<"can't get M location in the vertex shader."<<endl;}
    
    v_location=glGetUniformLocation(program,"V");//Global variable V is the view transformation matrix, which is sent to the vertex shader to transform the vertices  from the world coordinates to camera location.
    if (v_location==-1) {cout<<"can't get V location in the vertex shader."<<endl;}
    //pls define V.
     //then use glUniformMatrix4fv(v_location,1,GL_TRUE,V);// to the GPU
    
    p_location=glGetUniformLocation(program,"P");//Returns the index of the uniform variable name associated with the shader program.
    if (p_location==-1) {cout<<"can't get P location in the vertex shader."<<endl;}
 
    
    //--------------------------------
    P= Frustum( lef,  righ,  bottom,  top,  near,  far);

    //--------------------------------
    //P= Ortho(lef,righ,bottom,top,near,far)*pers;     //perspective projection
    
    
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    glClearColor( 1.0, 1.0, 0.7, 1.0 ); // white background
}


void
display_callback( void )//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
{
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
/*
    M[0][0]=R[0][0]*S[0][0];
    M[1][0]=R[1][0]*S[0][0];
    M[0][1]=R[0][1]*S[1][1];
    M[1][1]=R[1][1]*S[1][1];
    M[0][3]=T[0][3];
    M[1][3]=T[1][3];
*/
    M=T*R;//cumulative translation matrix and cumulative rotation matrix
    // if (DEBUG_ON) cout<<M<<endl;
    glUniformMatrix4fv(m_location,1,GL_TRUE,M);
    glUniformMatrix4fv(p_location,1,GL_TRUE,P);
    glUniformMatrix4fv(v_location,1,GL_TRUE,V);
    
    glDrawArrays(GL_TRIANGLES, 0, nvertices);
    glutSwapBuffers();
}


void
keyboard_callback( unsigned char key, int x, int y )//~~~~~~~~~~~~~~~~~~~~
{
    switch ( key ) {
		case 033:  // octal ascii code for ESC
		case 'q':
		case 'Q':
            exit( 0 );
        case 'r':
            S=mat4();
            R=mat4();
            T=mat4();
            M=mat4();
            glutPostRedisplay();
            break;
    }
}

mat4 pers;

void
process_special_keys(int key, int x, int y)//~~~~~~~~~~~~~~~~~~~~~~~~~~
{
    //int mod = glutGetModifiers();
    switch (key){
        case GLUT_KEY_UP:
            cout<<"go forward..."<<endl;
          //  printf("current window width: %d \n",glutGet(GLUT_WINDOW_WIDTH));
          //  printf("current window height: %d  \n",glutGet(GLUT_WINDOW_HEIGHT));
            eye+=view_dir*(0.1);
 /*           near+=0.1;
            far+=0.1;
  P= Frustum( lef,  righ,  bottom,  top,  near,  far);

*/
        //-------from view transformation matrix function from mat.h---update Viewing transformation matrix V-----
            n = normalize(-view_dir);
            u = normalize(cross(up_dir,n));
            v = normalize(cross(n,u));
            c = mat4(u, v, n, line4);
            V=c * Translate( -eye );
        //------------------------
            break;
        case GLUT_KEY_RIGHT:
            cout<<"turning right..."<<endl;
            view_dir.x = costheta*view_dir.x - sintheta*view_dir.z;
            view_dir.z = costheta*view_dir.z + sintheta*view_dir.x;
        //-------from view transformation matrix function from mat.h---update Viewing transformation matrix V-----
             n = normalize(-view_dir);
             u = normalize(cross(up_dir,n));
             v = normalize(cross(n,u));
             c = mat4(u, v, n, line4);
             V=c * Translate( -eye );
        //------------------------
            break;
        case GLUT_KEY_LEFT:
            cout<<"turning left"<<endl;
            view_dir.x = costheta*view_dir.x + sintheta*view_dir.z;
            view_dir.z = costheta*view_dir.z - sintheta*view_dir.x;
            //-------from view transformation matrix function from mat.h---update Viewing transformation matrix V-----
            n = normalize(-view_dir);
            u = normalize(cross(up_dir,n));
            v = normalize(cross(n,u));
            c = mat4(u, v, n, line4);
            V=c * Translate( -eye );//V[0][3]=
            //-------------------------------------------------------
            break;
        case GLUT_KEY_DOWN:
            cout<<"go backward..."<<endl;
            eye-=view_dir*(0.1);
 /*           near-=0.2;
            far-=0.2;
            //----update P--------------------------------------
  P= Frustum( lef,  righ,  bottom,  top,  near,  far);

*/
            //-------from view transformation matrix function from mat.h---update Viewing transformation matrix V-----
            n = normalize(-view_dir);
            u = normalize(cross(up_dir,n));
            v = normalize(cross(n,u));
            c = mat4(u, v, n, line4);
            V=c * Translate( -eye );
            //------------------------            break;
    }
    glutPostRedisplay();
}

//void passivemouse(int x,int y){

//}

void mouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);//when redrawing the display
        
        mod = glutGetModifiers();//can use this in mouse function
        if (mod == GLUT_ACTIVE_ALT){
            cout<<"Following the mouse..."<<endl;//want this to be printed out once every time we drag and  rotate
        }
        else { cout<<"Rotating around cube's x or y axis..."<<endl;}
        x_pos=x;
        y_pos=y;
    }
 /*   else if (button==GLUT_LEFT_BUTTON&&state==GLUT_UP){
    }
    else if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) {
    }*/
}


void mouse_motion(int x, int y) {//x and y max are 1390,856.
    vec2 offset(x-x_pos,y-y_pos);
    //  cout<<x<<" "<<y<<" "<<x_pos<<" "<<y_pos<<endl;
    x_pos=x;
    y_pos=y;
    if (mod == GLUT_ACTIVE_ALT){//mod is modified by mouse() call back function
        //float magnitude=sqrt(offset.x*offset.x+offset.y*offset.y);
        //translating
        offset=offset*0.003;
        T[0][3]+=offset.x;
        T[1][3]-=offset.y;
    }
    else if (mod==GLUT_ACTIVE_CTRL){
        if(offset.x<0){
            R=rotateZ_3_degrees*R;
        }else if (offset.x>0){
            R=rotateZ_minus_3_degrees*R;
        }
    }
    else {//no special key is pressed when pressing the mouse
        cout<<"fu  "<<offset.x<<" "<<offset.y<<endl;

        if(abs(offset.x)>abs(offset.y)){//horizontal movement dominates, rotate around Y
            if (offset.x<0){//move downward
                R=rotateY_minus_3_degrees*R; //3 degrees
            }else if (offset.x>0){
                R=rotateY_3_degrees*R; //-3 degrees
            }
        }
         else if(abs(offset.x)<abs(offset.y)) { //vertical movement dominates, rotate around x
             
             if (offset.y<0){//move downward
                //cout<<"zheng   "<<x<<"  "<<x_pos<<" "<<offset.x<<endl;
                R=rotateX_minus_3_degrees*R; //3 degrees
            }else if (offset.y>0){
                R=rotateX_3_degrees*R; //3 degrees
            }
        }
    }
    glutPostRedisplay();
}


void reshapeWindow(int w, int h){
   // printf("%d %d \n " , w, h);
    GLfloat aspectRatio=w/h;
    
    if (aspectRatio>1.0){
        bottom=-1.0;
        top=1.0;
        lef = original_lef * aspectRatio;
        righ = original_righ * aspectRatio;
    }
    else if(aspectRatio<1.0){
        righ=1.0;
        lef=-1.0;
        bottom = original_bottom / aspectRatio;
        top = original_top / aspectRatio;
    }
    else {
        lef=-1.0;
        bottom=-1.0;
        righ=1.0;
        top=1.0;
    }
    
    //double fovy = 45.0;
    //P=Perspective(fovy, aspectRatio, near, far);
  //  P= Frustum( lef,  righ,  bottom,  top,  near,  far);
    
   if (w == h) glViewport(0, 0, w, h);
    else if (w > h) glViewport(0, 0, h, h);
    else glViewport(0, 0, w, w);
  /***********from the Internet*****sample reshape func for 3D scenes*******/
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    //glViewport(0, 0, w, h);
    gluPerspective(45, aspectRatio, near, far);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
/****************************/
    glutPostRedisplay();
}
int
main( int argc, char **argv )
{
    glutInit( &argc, argv );
    //–  Initializes GLUT and must be called before any OpenGL functions are called!
    //–  Accepts the command line arguments from main() and can use them in an implementation-dependent manner
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA |GLUT_DEPTH);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutCreateWindow( "3D interaction" );
	//–  Creates a window on the screen with the given title!
    //–  Returns an integer that can be used to refer to the window in multi-window situations!
    init();
	
    glutDisplayFunc( display_callback );//tell openGL to use this function as call back function.
    //–  Because the form of the GLUT callback functions is ﬁxed, one must use global variables to pass values into and out of callback functions!
    glutKeyboardFunc( keyboard_callback );
    glutSpecialFunc(process_special_keys);
    glutMouseFunc(mouse);
    glutMotionFunc(mouse_motion);
    glutReshapeFunc(reshapeWindow);
    //  glutPassiveMotionFunc(passivemouse);
    
    glutMainLoop();
    return 0;
}

