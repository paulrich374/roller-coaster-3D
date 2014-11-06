/*
  CSCI 480
  Assignment 2
  Wei-Hung Liu
  weihungl@usc.edu
  
  
  
 */

#include <stdio.h>
#include <stdlib.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#include <math.h>
#include "pic.h"
#include <math.h>


const int WIDTH    			= 640;		/* initial window width */
const int HEIGHT   			= 480;		/* initial window height */
const GLdouble FOV 			= 20.0;	  	/* perspective field of view */
bool SAVE_SCREENSHOT 		= false;
bool START_ANIMATION  		= true;



/* represents one control point along the spline */
struct point {
   double x;
   double y;
   double z;
};

/* spline struct which contains how many control points, and an array of control points */
struct spline {
   int numControlPoints;
   struct point *points;
};

/* the spline array */
struct spline *g_Splines;

/* total number of splines */
int g_iNumOfSplines;

int g_iMenuId;

/* - Mouse State Variables - */
int g_vMousePos[2] 			= {0, 0};
int g_iLeftMouseButton 		= 0;    /* 1 if pressed, 0 if not */
int g_iMiddleMouseButton 	= 0;
int g_iRightMouseButton 	= 0;


/* state of the world */
float g_vLandRotate[3]    	= {0.0, 0.0, 0.0};
float g_vLandTranslate[3] 	= {0.0, 0.0, 0.0};
float g_vLandScale[3]     	= {1.0, 1.0, 1.0};

/* control state derived from mouse button state */
typedef enum { ROTATE, TRANSLATE, SCALE } CONTROLSTATE;
CONTROLSTATE g_ControlState = ROTATE;

/* for splines */
float delta=0.0f;
int count_control_points = -1;
float moveunit = 0.009;

/* arbitrary point for nomral vector */
point arbitrary_point;
/* point for spline tangent vector, position, normal vector, binormal vector */
point t,p,n,b;
point t1,p1,n1,b1;
/* point for camera eye and focus */
point eye, focus;
float up_step_size_Factor = 1.5;    	

/* storage for textures (only one for now)*/
GLuint texIndexSky;
GLuint texIndexGround;

/* screenshot number */
int frame_Num 				= 0;      



/*----------------------- Utility function ------------------------------*/
int loadSplines(char *argv) {
  char *cName = (char *)malloc(128 * sizeof(char));
  FILE *fileList;
  FILE *fileSpline;
  int iType, i = 0, j, iLength;


  /* load the track file */
  fileList = fopen(argv, "r");
  if (fileList == NULL) {
    printf ("can't open file\n");
    exit(1);
  }
  
  /* stores the number of splines in a global variable */
  fscanf(fileList, "%d", &g_iNumOfSplines);

  g_Splines = (struct spline *)malloc(g_iNumOfSplines * sizeof(struct spline));

  /* reads through the spline files */
  for (j = 0; j < g_iNumOfSplines; j++) {
    i = 0;
    fscanf(fileList, "%s", cName);
    fileSpline = fopen(cName, "r");

    if (fileSpline == NULL) {
      printf ("can't open file\n");
      exit(1);
    }

    /* gets length for spline file */
    fscanf(fileSpline, "%d %d", &iLength, &iType);

    /* allocate memory for all the points */
    g_Splines[j].points = (struct point *)malloc(iLength * sizeof(struct point));
    g_Splines[j].numControlPoints = iLength;

    /* saves the data to the struct */
    while (fscanf(fileSpline, "%lf %lf %lf", 
	   &g_Splines[j].points[i].x, 
	   &g_Splines[j].points[i].y, 
	   &g_Splines[j].points[i].z) != EOF) {
      i++;
    }
  }

  free(cName);

  return 0;
}


/* Write a screenshot to the specified filename */
void saveScreenshot (char *filename){   

int i, j;   

Pic *in = NULL;   

Pic *out = NULL;

if (filename == NULL)       

return;

in = pic_alloc(640, 480, 3, NULL);   

out = pic_alloc(640, 480, 3, NULL);

printf("File to save to: %s\n", filename);


glReadPixels(0, 0, 640, 480, GL_RGB, GL_UNSIGNED_BYTE, &in->pix[0]);       

for ( int j=0; j<480; j++ ) { 

for ( int i=0; i<640; i++ ) { 

PIC_PIXEL(out, i, j, 0) = PIC_PIXEL(in, i, 480-1-j, 0); 

PIC_PIXEL(out, i, j, 1) = PIC_PIXEL(in, i, 480-1-j, 1);             

PIC_PIXEL(out, i, j, 2) = PIC_PIXEL(in, i, 480-1-j, 2); 

} 

}

if (jpeg_write(filename, out))       

printf("File saved Successfully\n");   

else       

printf("Error in Saving\n");

pic_free(in);    

pic_free(out);

}



/* call screenshot function and create filename */
void triggerSaveScreenshot(){
	char str[2048];
	if (SAVE_SCREENSHOT && frame_Num < 1000) {
		frame_Num++;
		sprintf(str, "%03d.jpg", frame_Num); 
		saveScreenshot(str);		
	} else {
		SAVE_SCREENSHOT = false;
	}	
}

void drawText(char *string, GLfloat z)
{
    glColor3f(1, 1, 0);
    glRasterPos3f( eye.x, eye.y,eye.z);
    for(int i = 0; string[i] != '\0'; i++)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, string[i]);
}

/*
	loadTexture - Loads a texture from a JPEG file to memory and returns the handle
		Note: pWidth and pHeight are pointers to return imageWidth and imageHeight
*/
//GLuint loadTexture ( char * filename, int width, int height ){
void loadTexture (){

	Pic *textureSky = jpeg_read((char*)"sky6.jpg", NULL);
      
    glGenTextures( 1, &texIndexSky );
    glBindTexture( GL_TEXTURE_2D, texIndexSky );
    // select modulate to mix texture with color for shading
    glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
   
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureSky->nx, textureSky->ny, 0, GL_RGB, GL_UNSIGNED_BYTE, textureSky->pix);
    
    
	Pic *textureGround = jpeg_read((char*)"ground1.jpg", NULL);
   
    glGenTextures( 1, &texIndexGround );
    glBindTexture( GL_TEXTURE_2D, texIndexGround );
    
    // select modulate to mix texture with color for shading
    glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
   	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    //glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureGround->nx, textureGround->ny, 0, GL_RGB, GL_UNSIGNED_BYTE, textureGround->pix);
 
    pic_free(textureSky);
    pic_free(textureGround);    
   
    
    
}

/* implement Catmull-Rom Spline equation: f(x) = [1, x, x^2, x^3] * M * [v0, v1, v2, v3]
Coefficients for Matrix M 
M11	 0.0 M12	 1.0 M13	 0.0 M14	 0.0
M21	-0.5 M22	 0.0 M23	 0.5 M24	 0.0
M31	 1.0 M32	-2.5 M33	 2.0 M34	-0.5
M41	-0.5 M42	 1.5 M43	-1.5 M44	 0.5 
*/
float catmullRomSpline(float x, float v0,float v1,
				float v2,float v3) {

	float c1,c2,c3,c4;

	c1 =  0.0*v0 +   1.0*v1  +    0.0*v2 +    0.0*v3;
	c2 = -0.5*v0 +   0.0*v1  +    0.5*v2 +    0.0*v3;
	c3 =  1.0*v0 +(-2.5)*v1  +    2.0*v2 + (-0.5)*v3;
	c4 = -0.5*v0 +   1.5*v1  + (-1.5)*v2 +    0.5*v3;

	return(((c4*x + c3)*x +c2)*x + c1);
}

/*implement first derivative of Catmull-Rom Spline equation: f(x) = [0, 1, 2*x, 3*x^2] * M * [v0, v1, v2, v3] */
float tangent(float x, float v0,float v1,
				float v2,float v3) {
	
	
	float c1,c2,c3,c4;

	c1 =  0.0*v0 +   1.0*v1  +    0.0*v2 +    0.0*v3;
	c2 = -0.5*v0 +   0.0*v1  +    0.5*v2 +    0.0*v3;
	c3 =  1.0*v0 +(-2.5)*v1  +    2.0*v2 + (-0.5)*v3;
	c4 = -0.5*v0 +   1.5*v1  + (-1.5)*v2 +    0.5*v3;

	return((3*c4*x + 2*c3)*x + 1*c2);	
}

/*implement cross product 
  s_1 = u_2v_3 - u_3v_2
  s_2 = u_3v_1 - u_1v_3
  s_3 = u_1v_2 - u_2v_1

*/ 
point crossProduct(point a, point b) {
	point result_point;
	result_point.x = a.y*b.z - a.z*b.y;
	result_point.y = a.z*b.x - a.x*b.z;
	result_point.z = a.x*b.y - a.y*b.x;	
    return result_point;	    
}

/* implement u / ||u|| */
point unit(point unit){
	float length;
	point normalized_point;
	length = sqrt( (unit.x*unit.x) + (unit.y*unit.y) + (unit.z*unit.z) );
    normalized_point.x=(float) unit.x/length;
    normalized_point.y=(float) unit.y/length;
    normalized_point.z=(float) unit.z/length;
    return normalized_point;
  	
} 

/* implement dynamic gluLookAt(ex, ey, ez, fx, fy, fz, ux, uy, uz) = gluLookAt(P, T, B) */
void camera() {
	
    int spline_index = 0;
    int control_point_index = count_control_points;
    float cross_step_size = 0.08;
    float up_step_size_camera = 0.1;
    
    p.x = catmullRomSpline(delta, g_Splines[spline_index].points[control_point_index].x, g_Splines[spline_index].points[control_point_index+1].x, g_Splines[spline_index].points[control_point_index+2].x, g_Splines[spline_index].points[control_point_index+3].x);
    p.y = catmullRomSpline(delta, g_Splines[spline_index].points[control_point_index].y, g_Splines[spline_index].points[control_point_index+1].y, g_Splines[spline_index].points[control_point_index+2].y, g_Splines[spline_index].points[control_point_index+3].y);
    p.z = catmullRomSpline(delta, g_Splines[spline_index].points[control_point_index].z, g_Splines[spline_index].points[control_point_index+1].z, g_Splines[spline_index].points[control_point_index+2].z, g_Splines[spline_index].points[control_point_index+3].z);
    t.x = tangent(delta, g_Splines[spline_index].points[control_point_index].x, g_Splines[spline_index].points[control_point_index+1].x, g_Splines[spline_index].points[control_point_index+2].x, g_Splines[spline_index].points[control_point_index+3].x);
    t.y = tangent(delta, g_Splines[spline_index].points[control_point_index].y, g_Splines[spline_index].points[control_point_index+1].y, g_Splines[spline_index].points[control_point_index+2].y, g_Splines[spline_index].points[control_point_index+3].y);
    t.z = tangent(delta, g_Splines[spline_index].points[control_point_index].z, g_Splines[spline_index].points[control_point_index+1].z, g_Splines[spline_index].points[control_point_index+2].z, g_Splines[spline_index].points[control_point_index+3].z);
   
    t   = unit(t);
    n   = crossProduct(t, arbitrary_point);
    n   = unit(n);
    b   = crossProduct(t, n);
    b   = unit(b);
    
    eye.x 	= p.x + (cross_step_size/2.0)*n.x + up_step_size_camera*b.x;
    eye.y 	= p.y + (cross_step_size/2.0)*n.y + up_step_size_camera*b.y;
    eye.z 	= p.z + (cross_step_size/2.0)*n.z + up_step_size_camera*b.z;
    focus.x = p.x + (cross_step_size/2.0)*n.x + t.x+ up_step_size_camera*b.x*1.3;
    focus.y = p.y + (cross_step_size/2.0)*n.y + t.y+ up_step_size_camera*b.y*1.3;
    focus.z = p.z + (cross_step_size/2.0)*n.z + t.z+ up_step_size_camera*b.z*1.3;
    
    gluLookAt(eye.x, eye.y, eye.z, focus.x, focus.y, focus.z, b.x, b.y, b.z);
 			
}

void animation () {

   	delta+=moveunit; /* Increase   */
   	/* loop back from zero because delta range 0 ~ 1*/
   	if ( delta >= 1.0 ) {
   		count_control_points++;
   		delta = 0.0;
		/* loop back from the start */
    	if(count_control_points >= g_Splines[0].numControlPoints - 4)
        	count_control_points = 0;	   		
	}
}

/*----------------------- Draw function ------------------------------*/


void drawSkyAndGround()
{
  	/* NOTE: necessary to clean all color */
  	glColor3f(1.0, 1.0, 1.0);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texIndexSky);
    
    glBegin(GL_QUADS);
    /*xz plane*/
    glTexCoord2d(0.0,0.0); glVertex3d(-128,-128.0,-128);
    glTexCoord2d(1.0,0.0); glVertex3d(128.0,-128.0,-128);
    glTexCoord2d(1.0,1.0); glVertex3d(128.0,-128.0,128);
    glTexCoord2d(0.0,1.0); glVertex3d(-128.0,-128.0,128);
    /*zy plane*/
    glTexCoord2d(0.0,0.0); glVertex3d(-128,-128,-128);
    glTexCoord2d(1.0,0.0); glVertex3d(-128,128.0,-128);
    glTexCoord2d(1.0,1.0); glVertex3d(-128,128.0,128);
    glTexCoord2d(0.0,1.0); glVertex3d(-128,-128.0,128);
    /*xz plane*/
    glTexCoord2d(0.0,0.0); glVertex3d(-128,128.0,-128);
    glTexCoord2d(1.0,0.0); glVertex3d(128.0,128.0,-128);
    glTexCoord2d(1.0,1.0); glVertex3d(128.0,128.0,128);
    glTexCoord2d(0.0,1.0); glVertex3d(-128.0,128.0,128);
    /*zy plane*/
    glTexCoord2d(0.0,0.0); glVertex3d(128,-128,-128);
    glTexCoord2d(1.0,0.0); glVertex3d(128,128.0,-128);
    glTexCoord2d(1.0,1.0); glVertex3d(128,128.0,128);
    glTexCoord2d(0.0,1.0); glVertex3d(128,-128.0,128);
    /*xy plane*/
    glTexCoord2d(0.0,0.0); glVertex3d(-128,-128.0,128);
    glTexCoord2d(1.0,0.0); glVertex3d(128.0,-128.0,128);
    glTexCoord2d(1.0,1.0); glVertex3d(128.0,128.0,128);
    glTexCoord2d(0.0,1.0); glVertex3d(-128.0,128.0,128);
    glEnd();
    glDisable(GL_TEXTURE_2D);


    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texIndexGround);
    
    glBegin(GL_QUADS);

    glTexCoord2d(0.0,0.0); glVertex3d(-128,-128.0,-2);
    glTexCoord2d(1.0,0.0); glVertex3d(128.0,-128.0,-2);
    glTexCoord2d(1.0,1.0); glVertex3d(128.0,128.0,-2);
    glTexCoord2d(0.0,1.0); glVertex3d(-128.0,128.0,-2);
    glEnd();
    glDisable(GL_TEXTURE_2D);
}

/* implement 3 rails like 'V' shape */
/* .     . side rails

      . bottom rail
*/
void drawSplines()
{
	
	/* horizontal scale factor */
	float cross_step_size=0.08;
	/* vertical scale factor */
	float up_step_size=0.04;
	/* tube scale factor */
	float tube_step_size=0.006;
	
	/* parallel rail number (implement V shape so we have 3 parallel spline path and therefore for loop 3 times ) */
	for(int index=1;index<4;index++){

     	glBegin(GL_QUADS);
    	/* red */
      	glColor3f(1 , 0, 0); 	

    
    	for (int spline_index = 0; spline_index < g_iNumOfSplines; spline_index++) {
    		
    	    for(int i=-2;i<g_Splines[spline_index].numControlPoints-1;i++) {
    	    	               	
    	        for(float u=0.0;u<1.0;u+=0.04) {
    	            
    	         	/*Point*/
    	            p.x= catmullRomSpline(u, g_Splines[spline_index].points[i].x, g_Splines[spline_index].points[i+1].x, g_Splines[spline_index].points[i+2].x, g_Splines[spline_index].points[i+3].x);
    	            p.y= catmullRomSpline(u, g_Splines[spline_index].points[i].y, g_Splines[spline_index].points[i+1].y, g_Splines[spline_index].points[i+2].y, g_Splines[spline_index].points[i+3].y);
    	            p.z= catmullRomSpline(u, g_Splines[spline_index].points[i].z, g_Splines[spline_index].points[i+1].z, g_Splines[spline_index].points[i+2].z, g_Splines[spline_index].points[i+3].z);
    	            /*Tangent*/
    	            t.x= tangent(u, g_Splines[spline_index].points[i].x, g_Splines[spline_index].points[i+1].x, g_Splines[spline_index].points[i+2].x, g_Splines[spline_index].points[i+3].x);
    	            t.y= tangent(u, g_Splines[spline_index].points[i].y, g_Splines[spline_index].points[i+1].y, g_Splines[spline_index].points[i+2].y, g_Splines[spline_index].points[i+3].y);
    	            t.z= tangent(u, g_Splines[spline_index].points[i].z, g_Splines[spline_index].points[i+1].z, g_Splines[spline_index].points[i+2].z, g_Splines[spline_index].points[i+3].z);
    	            t = unit(t);
    	            /*Normal*/
    	            n = crossProduct(t, arbitrary_point);
    	            n = unit(n);
    	            /*Binormal*/
    	            b = crossProduct(t, n);
    	            b = unit(b);
					/* use u+0.02 to generate dense square cross-section */
    	         	/*Point*/
    	            p1.x= catmullRomSpline(u+0.04, g_Splines[spline_index].points[i].x, g_Splines[spline_index].points[i+1].x, g_Splines[spline_index].points[i+2].x, g_Splines[spline_index].points[i+3].x);
    	            p1.y= catmullRomSpline(u+0.04, g_Splines[spline_index].points[i].y, g_Splines[spline_index].points[i+1].y, g_Splines[spline_index].points[i+2].y, g_Splines[spline_index].points[i+3].y);
    	            p1.z= catmullRomSpline(u+0.04, g_Splines[spline_index].points[i].z, g_Splines[spline_index].points[i+1].z, g_Splines[spline_index].points[i+2].z, g_Splines[spline_index].points[i+3].z);
    	            /*Tangent*/
    	            t1.x= tangent(u+0.04, g_Splines[spline_index].points[i].x, g_Splines[spline_index].points[i+1].x, g_Splines[spline_index].points[i+2].x, g_Splines[spline_index].points[i+3].x);
    	            t1.y= tangent(u+0.04, g_Splines[spline_index].points[i].y, g_Splines[spline_index].points[i+1].y, g_Splines[spline_index].points[i+2].y, g_Splines[spline_index].points[i+3].y);
    	            t1.z= tangent(u+0.04, g_Splines[spline_index].points[i].z, g_Splines[spline_index].points[i+1].z, g_Splines[spline_index].points[i+2].z, g_Splines[spline_index].points[i+3].z);
    	            t1 = unit(t1);
    	            /*Normal*/
    	            n1 = crossProduct(t1, arbitrary_point);
    	            n1 = unit(n1);
    	            /*Binormal*/
    	            b1 = crossProduct(t1, n1);
    	            b1 = unit(b1);
    	
    	            /* implement V shape roller coaster rail */
    	            /* main bottom rail - Center (V bottom vetex)*/  
    	            if(index==1)
    	            {
    	                p.x+=cross_step_size*n.x/2;
    	                p.y+=cross_step_size*n.y/2;
    	                p.z+=cross_step_size*n.z/2;
    	
    	                p1.x+=cross_step_size*n1.x/2;
    	                p1.y+=cross_step_size*n1.y/2;
    	                p1.z+=cross_step_size*n1.z/2;    	
    	            }
    	            /* side rail - Left-Top(V left vetex)*/
    	            if(index==2)
    	            {
    	                p.x+=up_step_size*b.x*up_step_size_Factor;
    	                p.y+=up_step_size*b.y*up_step_size_Factor;
    	                p.z+=up_step_size*b.z*up_step_size_Factor;

    	                p1.x+=up_step_size*b1.x*up_step_size_Factor;
    	                p1.y+=up_step_size*b1.y*up_step_size_Factor;
    	                p1.z+=up_step_size*b1.z*up_step_size_Factor;    	
    	                
    	            }
    	            /* side rail - Right-Top(V right vetex)*/
    	            if(index==3)
    	            {
    	                p.x+=cross_step_size*n.x+ up_step_size*b.x*up_step_size_Factor;
    	                p.y+=cross_step_size*n.y+ up_step_size*b.y*up_step_size_Factor;
    	                p.z+=cross_step_size*n.z+ up_step_size*b.z*up_step_size_Factor;       
    	                
    	                p1.x+=cross_step_size*n1.x+ up_step_size*b1.x*up_step_size_Factor;
    	                p1.y+=cross_step_size*n1.y+ up_step_size*b1.y*up_step_size_Factor;
    	                p1.z+=cross_step_size*n1.z+ up_step_size*b1.z*up_step_size_Factor;
    	
    	            }
	/*  SQUARE TUBE RAIL   	                   
	
	
	
											      TOP FACE
	   
											 (P1)v2 _____ v1(P1)
												   /    /|
											(P1)v3/|___/_| v0(P1)
												 / /  /  /
			      LEFT FACE --->				/_/__/  /            <--- RIGHT FACE
										  (P0)v2|/   | /v1(P0) 
												|____|/
										  (P0)v3      v0(P0)  
										  
										  
										  	BOTTOM FACE
										     	               
									P0: current point, P1: next point 
									.v2(-n+b)   .v1(+n+b)
									
									.v3(-n-b)   .v0(+n-b)  	  
									
									RIGHT FACE : v0(P0) - v0(P1) - v1(P1) - v1(P0)
									LEFT FACE  : (P0)v3 - (P1)v3 - (P1)v2 -(P0)v2
									TOP FACE   : (P0)v2 - v1(P0) - v1(P1) - (P1)v2
									BOTTOM FACE: (P0)v3 - v0(P0) - v0(P1) - (P1)v3  
	*/
	/*  V-SHAPED SQUARE TUBE RAIL   	                   

			
				                             	   
						 (P1)v2 _____ v1(P1) 			 (P1)v2 _____ v1(P1)
							   /    /|       				   /    /|
						(P1)v3/|___/_| v0(P1)			(P1)v3/|___/_| v0(P1)
							 / /  /  /       				 / /  /  /
TOP_LEFT RAIL -->			/_/__/  /        				/_/__/  /              <-- TOP-RIGHT RAIL
					  (P0)v2|/   | /v1(P0)   		  (P0)v2|/   | /v1(P0) 
							|____|/          				|____|/
					  (P0)v3      v0(P0)     		  (P0)v3      v0(P0) 
			 
				                              
									 (P1)v2 _____ v1(P1)  
										   /    /|              
									(P1)v3/|___/_| v0(P1)       
										 / /  /  /         <-- BOTTOM RAIL     
										/_/__/  /               
								  (P0)v2|/   | /v1(P0)          
										|____|/                 
								  (P0)v3      v0(P0)            
			       
							In drawSplines()
							render tripe rail as a V-shpaed by using different start point of P0 
							BOTTOM RAIL:     like index==1 render the first rail which lies in the bottom of V-shaped
							TOP_LEFT RAIL:   like index==2 render the second rail which lies in the top-left of V-shaped
							TOP-RIGHT RAIL:  like index==3 render the third rail which lies in the top-right of V-shaped

	*/
					/* p0 v0 */
    	            glVertex3f(p.x + tube_step_size*( n.x - b.x), p.y + tube_step_size*( n.y - b.y), p.z + tube_step_size*( n.z - b.z));
					/* p1 v0 */
    	            glVertex3f(p1.x + tube_step_size*( n1.x - b1.x), p1.y + tube_step_size*( n1.y - b1.y), p1.z + tube_step_size*( n1.z - b1.z));
					/* p1 v1 */
					glVertex3f(p1.x + tube_step_size*(+n1.x + b.x), p1.y + tube_step_size*( n1.y + b1.y), p1.z + tube_step_size*( n1.z + b1.z));
					/* p0 v1 */
					glVertex3f(p.x + tube_step_size*( n.x + b.x), p.y + tube_step_size*( n.y + b.y), p.z + tube_step_size*( n.z + b.z));

					/* p0 v3 */
					glVertex3f(p.x + tube_step_size*(-n.x - b.x), p.y + tube_step_size*(-n.y - b.y), p.z + tube_step_size*(-n.z - b.z));
					/* p1 v3 */
					glVertex3f(p1.x + tube_step_size*(-n1.x - b1.x), p1.y + tube_step_size*(-n1.y - b1.y), p1.z + tube_step_size*(-n1.z - b1.z));
					/* p1 v2 */
					glVertex3f(p1.x + tube_step_size*(-n1.x + b1.x), p1.y + tube_step_size*(-n1.y + b1.y), p1.z + tube_step_size*(-n1.z + b1.z));
					/* p0 v2 */
    	            glVertex3f(p.x + tube_step_size*(-n.x + b.x), p.y + tube_step_size*(-n.y + b.y), p.z + tube_step_size*(-n.z + b.z));							
    	            
    	            /* left side rail form bottom quad */		
					/* p0 v2 */
    	            glVertex3f(p.x + tube_step_size*(-n.x + b.x), p.y + tube_step_size*(-n.y + b.y), p.z + tube_step_size*(-n.z + b.z));							
					/* p0 v1 */
					glVertex3f(p1.x + tube_step_size*(-n1.x - b1.x), p1.y + tube_step_size*(-n1.y - b1.y), p1.z + tube_step_size*(-n1.z - b1.z));
					/* p1 v1 */
					glVertex3f(p1.x + tube_step_size*(+n1.x + b.x), p1.y + tube_step_size*( n1.y + b1.y), p1.z + tube_step_size*( n1.z + b1.z));
					/* p1 v2 */
					glVertex3f(p1.x + tube_step_size*(-n1.x + b1.x), p1.y + tube_step_size*(-n1.y + b1.y), p1.z + tube_step_size*(-n1.z + b1.z));
    	  							
    	            /* form top quad */		
					/* p0 v3 */
					glVertex3f(p.x + tube_step_size*(-n.x - b.x), p.y + tube_step_size*(-n.y - b.y), p.z + tube_step_size*(-n.z - b.z));
					/* p0 v0 */
    	            glVertex3f(p.x + tube_step_size*( n.x - b.x), p.y + tube_step_size*( n.y - b.y), p.z + tube_step_size*( n.z - b.z));
					/* p1 v0 */
    	            glVertex3f(p1.x + tube_step_size*( n1.x - b1.x), p1.y + tube_step_size*( n1.y - b1.y), p1.z + tube_step_size*( n1.z - b1.z));
					/* p1 v3 */
					glVertex3f(p1.x + tube_step_size*(-n1.x - b1.x), p1.y + tube_step_size*(-n1.y - b1.y), p1.z + tube_step_size*(-n1.z - b1.z));
            							
		        }/* end of for loop*/
    	    }/* end of for loop*/
    	}/* end of for loop*/
    glEnd();
	}/* end of for loop*/
}

/* implement cross rail (45 degrss '/' and 135 degree '\') between two side rails and bottom rail respeactively */
void drawCrossRail()
{
	int factor=0;
	float cross_step_size=0.08;
	float up_step_size=0.035;
	
	/* parallel rail number */
	for(int index=2;index<4;index++){
		
		glLineWidth(7);
    	glBegin(GL_LINES);
    	/* white */
		glColor3f(1, 1, 1);
		      
        for (int spline_index = 0; spline_index < g_iNumOfSplines; spline_index++) {
        	
            for(int i=-2;i<g_Splines[spline_index].numControlPoints-1;i++) {     
            	          	
                for(float u=0.0;u<1.0;u+=0.1) {
                    
    	         	/*Point*/
    	            p.x= catmullRomSpline(u, g_Splines[spline_index].points[i].x, g_Splines[spline_index].points[i+1].x, g_Splines[spline_index].points[i+2].x, g_Splines[spline_index].points[i+3].x);
    	            p.y= catmullRomSpline(u, g_Splines[spline_index].points[i].y, g_Splines[spline_index].points[i+1].y, g_Splines[spline_index].points[i+2].y, g_Splines[spline_index].points[i+3].y);
    	            p.z= catmullRomSpline(u, g_Splines[spline_index].points[i].z, g_Splines[spline_index].points[i+1].z, g_Splines[spline_index].points[i+2].z, g_Splines[spline_index].points[i+3].z);
    	            /*Tangent*/
    	            t.x= tangent(u, g_Splines[spline_index].points[i].x, g_Splines[spline_index].points[i+1].x, g_Splines[spline_index].points[i+2].x, g_Splines[spline_index].points[i+3].x);
    	            t.y= tangent(u, g_Splines[spline_index].points[i].y, g_Splines[spline_index].points[i+1].y, g_Splines[spline_index].points[i+2].y, g_Splines[spline_index].points[i+3].y);
    	            t.z= tangent(u, g_Splines[spline_index].points[i].z, g_Splines[spline_index].points[i+1].z, g_Splines[spline_index].points[i+2].z, g_Splines[spline_index].points[i+3].z);
    	            t = unit(t);
    	            /*Normal*/
    	            n = crossProduct(t, arbitrary_point);
    	            n = unit(n);
    	            /*Binormal*/
    	            b = crossProduct(t, n);
    	            b = unit(b);
               
					/*
											 (P1)v2 _____ v1(P1) 			 (P1)v2 _____ v1(P1)
												   /    /|       				   /    /|
											(P1)v3/|___/_| v0(P1)			(P1)v3/|___/_| v0(P1)
												 / /  /  /       				 / /  /  /
												/_/__/  /        				/_/__/  /              
										  (P0)v2|/   | /v1(P0)   		  (P0)v2|/   | /v1(P0) 
												|____|/          				|____|/
										  (P0)v3      v0(P0)     		  (P0)v3      v0(P0) 
								                 \ \                              / /
									              \ \                            / /
												   \ \	 (P1)v2 _____ v1(P1)    / /
					CROSS RAIL SUPPORT-->			\ \		   /    /|         / /    <--- CROSS RAIL SUPPORT
													 \ \(P1)v3/|___/_| v0(P1) / /       
													  \ \	 / /  /  /       / /      
															/_/__/  /       / /        
													  (P0)v2|/   | /v1(P0)          
															|____|/                 
													  (P0)v3      v0(P0)   
					*/
                    /* top*/
                    if(index==2)
                    {
                        p.x+=up_step_size*b.x*up_step_size_Factor;
                        p.y+=up_step_size*b.y*up_step_size_Factor;
                        p.z+=up_step_size*b.z*up_step_size_Factor;
						factor = 1;
                        
                    }
                    /* top-right*/
                    if(index==3)
                    {
                        p.x+=cross_step_size*n.x+ up_step_size*b.x*up_step_size_Factor;
                        p.y+=cross_step_size*n.y+ up_step_size*b.y*up_step_size_Factor;
                        p.z+=cross_step_size*n.z+ up_step_size*b.z*up_step_size_Factor;
						factor = -1;
                    }
                   	/* side rail */
                    glVertex3f(p.x, p.y, p.z);
                    /* main bottom rail*/
                    glVertex3f(p.x-up_step_size*b.x*up_step_size_Factor+factor*cross_step_size*n.x/2, p.y-up_step_size*b.y*up_step_size_Factor+factor*cross_step_size*n.y/2, p.z-up_step_size*b.z*up_step_size_Factor+factor*cross_step_size*n.z/2);
	
	            }/* end of for loop*/
            }/* end of for loop*/
        }/* end of for loop*/
        
        glEnd();
	}/* end of for loop*/
}
/* implement touch ground pillar ( 4 faces ) to support roller coaster */	
void drawSupportedPillar()
{
	/* horizontal scale factor */
	float cross_step_size=0.08;
	/* vertical scale factor */
	float up_step_size=0.04;
	/* tube scale factor */
	float tube_step_size=0.006;
	
	/* parallel rail number (implement V shape so we have 3 parallel spline path and therefore for loop 3 times ) */
	for(int index=1;index<4;index++){

     	glBegin(GL_QUADS);
     	/* blue */
	   	glColor3f(0.1,0.1, 0.8); 

    	for (int spline_index = 0; spline_index < g_iNumOfSplines; spline_index++) {
    		
    	    for(int i=-2;i<g_Splines[spline_index].numControlPoints-1;i++) {
    	    	               	
    	        for(float u=0.0;u<1.0;u+=0.4) {
                      
    	         	/*Point*/
    	            p.x= catmullRomSpline(u, g_Splines[spline_index].points[i].x, g_Splines[spline_index].points[i+1].x, g_Splines[spline_index].points[i+2].x, g_Splines[spline_index].points[i+3].x);
    	            p.y= catmullRomSpline(u, g_Splines[spline_index].points[i].y, g_Splines[spline_index].points[i+1].y, g_Splines[spline_index].points[i+2].y, g_Splines[spline_index].points[i+3].y);
    	            p.z= catmullRomSpline(u, g_Splines[spline_index].points[i].z, g_Splines[spline_index].points[i+1].z, g_Splines[spline_index].points[i+2].z, g_Splines[spline_index].points[i+3].z);
    	            /*Tangent*/
    	            t.x= tangent(u, g_Splines[spline_index].points[i].x, g_Splines[spline_index].points[i+1].x, g_Splines[spline_index].points[i+2].x, g_Splines[spline_index].points[i+3].x);
    	            t.y= tangent(u, g_Splines[spline_index].points[i].y, g_Splines[spline_index].points[i+1].y, g_Splines[spline_index].points[i+2].y, g_Splines[spline_index].points[i+3].y);
    	            t.z= tangent(u, g_Splines[spline_index].points[i].z, g_Splines[spline_index].points[i+1].z, g_Splines[spline_index].points[i+2].z, g_Splines[spline_index].points[i+3].z);
    	            t = unit(t);
    	            /*Normal*/
    	            n = crossProduct(t, n);
    	            n = unit(n);
    	            /*Binormal*/
    	            b = crossProduct(t, n);
    	            b = unit(b);
               
					/* use u+0.02 to generate dense square cross-section 
					PILAR(SQUARE TUBE) UNDER V-SHAPED SQUARE TUBE RAIL   	                   

			
				                             	                     
						 (P1)v2 _____ v1(P1) 						  		 (P1)v2 _____ v1(P1)
							   /    /|       						  			   /    /|
						(P1)v3/|___/_| v0(P1)						  		(P1)v3/|___/_| v0(P1)
							 / /  /  /       						  			 / /  /  /
							/_/__/  /        						  			/_/__/  /              
					  (P0)v2|/   | /v1(P0)   						  	  (P0)v2|/   | /v1(P0) 
							|____|/          						  			|____|/
					  (P0)v3      v0(P0)     						  	  (P0)v3      v0(P0) 
			                                                		  	
                 				                              
                 	    p1 v3__ p1 v0            (P1)v2 _____ v1(P1)  
                 	        /_/|                	   /    /|      	    p1 v3__ p1 v0                              
                 	  p0 v3 ||||p0 v0           (P1)v3/|___/_| v0(P1	        /_/|                                   
 PILAR SUPPORT-->           | ||                	 / /  /  /      	  p0 v3 ||||p0 v0                            
                 	        ||||                	/_/__/  /       	        | ||              <--- PILAR SUPPORT   
                 p1 v3 ground|.|| p1 v0 ground(P0)v2|/   | /v1(P0)  	        ||||                                   
                 	        |||/                	|____|/         p1 v3 ground|.|| p1 v0 ground                      
                 p0 v3 ground  p0 v0 ground       (P0)v3  v0(P0)    	        |||/                                
		                                                            p0 v3 ground  p0 v0 ground
											    p1 v3__ p1 v0
											        /_/|
											  p0 v3 ||||p0 v0
											        | ||              <--- PILAR SUPPORT
											        ||||
									    p1 v3 ground|.|| p1 v0 ground 
											        |||/ 
				                        p0 v3 ground  p0 v0 ground
	                        
	                        
	                        
	                        PILAR SUPPORT right face:     p0 v0 - p1 v0 - p1 v0 ground - p0 v0 ground
	                        PILAR SUPPORT back face:      p0 v0 - p0 v3 - p0 v3 ground - p0 v0 ground
	                        PILAR SUPPORT left face:      p0 v3 - p1 v3 - p1 v3 ground - p0 v3 ground
	                        PILAR SUPPORT front face:     p1 v0 - p1 v3 - p1 v3 ground - p1 v0 ground	                                    
					*/	
     	         	/*Point*/
    	            p1.x= catmullRomSpline(u+0.02, g_Splines[spline_index].points[i].x, g_Splines[spline_index].points[i+1].x, g_Splines[spline_index].points[i+2].x, g_Splines[spline_index].points[i+3].x);
    	            p1.y= catmullRomSpline(u+0.02, g_Splines[spline_index].points[i].y, g_Splines[spline_index].points[i+1].y, g_Splines[spline_index].points[i+2].y, g_Splines[spline_index].points[i+3].y);
    	            p1.z= catmullRomSpline(u+0.02, g_Splines[spline_index].points[i].z, g_Splines[spline_index].points[i+1].z, g_Splines[spline_index].points[i+2].z, g_Splines[spline_index].points[i+3].z);
    	            /*Tangent*/
    	            t1.x= tangent(u+0.02, g_Splines[spline_index].points[i].x, g_Splines[spline_index].points[i+1].x, g_Splines[spline_index].points[i+2].x, g_Splines[spline_index].points[i+3].x);
    	            t1.y= tangent(u+0.02, g_Splines[spline_index].points[i].y, g_Splines[spline_index].points[i+1].y, g_Splines[spline_index].points[i+2].y, g_Splines[spline_index].points[i+3].y);
    	            t1.z= tangent(u+0.02, g_Splines[spline_index].points[i].z, g_Splines[spline_index].points[i+1].z, g_Splines[spline_index].points[i+2].z, g_Splines[spline_index].points[i+3].z);
    	            t1 = unit(t1);
    	            /*Normal*/
    	            n1 = crossProduct(t1, arbitrary_point);
    	            n1 = unit(n1);
    	            /*Binormal*/
    	            b1 = crossProduct(t1, n1);
    	            b1 = unit(b1);
    	
    	            /* implement V shape roller coaster rail */
    	            /* main bottom rail - Center (V bottom vetex)*/  
    	            if(index==1)
    	            {
    	                p.x+=cross_step_size*n.x/2;
    	                p.y+=cross_step_size*n.y/2;
    	                p.z+=cross_step_size*n.z/2;
    	
    	                p1.x+=cross_step_size*n1.x/2;
    	                p1.y+=cross_step_size*n1.y/2;
    	                p1.z+=cross_step_size*n1.z/2;    	
    	            }
    	            /* side rail - Left-Top(V left vetex)*/
    	            if(index==2)
    	            {
    	                p.x+=up_step_size*b.x*up_step_size_Factor;
    	                p.y+=up_step_size*b.y*up_step_size_Factor;
    	                p.z+=up_step_size*b.z*up_step_size_Factor;

    	                p1.x+=up_step_size*b1.x*up_step_size_Factor;
    	                p1.y+=up_step_size*b1.y*up_step_size_Factor;
    	                p1.z+=up_step_size*b1.z*up_step_size_Factor;    	
    	                
    	            }
    	            /* side rail - Right-Top(V right vetex)*/
    	            if(index==3)
    	            {
    	                p.x+=cross_step_size*n.x+ up_step_size*b.x*up_step_size_Factor;
    	                p.y+=cross_step_size*n.y+ up_step_size*b.y*up_step_size_Factor;
    	                p.z+=cross_step_size*n.z+ up_step_size*b.z*up_step_size_Factor;       
    	                
    	                p1.x+=cross_step_size*n1.x+ up_step_size*b1.x*up_step_size_Factor;
    	                p1.y+=cross_step_size*n1.y+ up_step_size*b1.y*up_step_size_Factor;
    	                p1.z+=cross_step_size*n1.z+ up_step_size*b1.z*up_step_size_Factor;
    	
    	            }
    	            /* implement 4 face of a pillar: right, backawrd, left, forward*/
    	            /* .v2(-n+b)   .v1(+n+b)
    	            
    	               .v3(-n-b)   .v0(+n-b)  
    	            */
    	            /* pillar right face */		
					/* p0 v0 */
    	            glVertex3f(p.x + tube_step_size*( n.x - b.x), p.y + tube_step_size*( n.y - b.y), p.z + tube_step_size*( n.z - b.z));
					/* p1 v0 */
    	            glVertex3f(p1.x + tube_step_size*( n1.x - b1.x), p.y + tube_step_size*( n1.y - b1.y), p1.z + tube_step_size*( n1.z - b1.z));
					/* p1 v0 ground */
					glVertex3f(p1.x + tube_step_size*( n1.x - b1.x), p.y + tube_step_size*( n1.y - b1.y), -3);
					/* p0 v0 ground */
					glVertex3f(p.x + tube_step_size*( n.x - b.x), p.y + tube_step_size*( n.y - b.y), -3);
    	            
    	            /* pillar back face */		
					/* p0 v0 */
    	            glVertex3f(p.x + tube_step_size*( n.x - b.x), p.y + tube_step_size*( n.y - b.y), p.z + tube_step_size*( n.z - b.z));
					/* p0 v3 */
    	            glVertex3f(p.x + tube_step_size*( -n.x - b.x), p.y + tube_step_size*(- n.y - b.y), p.z + tube_step_size*( -n.z - b.z));
					/* p0 v3 ground */
					glVertex3f(p.x + tube_step_size*(-n.x - b.x), p.y + tube_step_size*(-n.y - b.y), -3);
					/* p0 v0 ground */
					glVertex3f(p.x + tube_step_size*(n.x - b.x), p.y + tube_step_size*(n.y - b.y), -3);
    	            
    	            /* pillar left face */		
					/* p0 v3 */
    	            glVertex3f(p.x + tube_step_size*(-n.x - b.x), p.y + tube_step_size*(-n.y - b.y), p.z + tube_step_size*(-n.z - b.z));		
					/* p1 v3 */
    	            glVertex3f(p1.x + tube_step_size*(-n1.x - b1.x), p1.y + tube_step_size*(-n1.y - b1.y), p1.z + tube_step_size*(-n1.z - b1.z));
					/* p1 v3 ground */
					glVertex3f(p1.x + tube_step_size*(-n1.x - b1.x), p1.y + tube_step_size*(-n1.y - b1.y), -3);
					/* p0 v3 ground */
					glVertex3f(p.x + tube_step_size*(-n.x - b.x), p.y + tube_step_size*(-n.y - b.y), -3);
    	  							
    	            /* pillar front face */		
					/* p1 v0 */
    	            glVertex3f(p1.x + tube_step_size*( n1.x - b1.x), p1.y + tube_step_size*( n1.y - b1.y), p1.z + tube_step_size*( n1.z - b1.z));							
					/* p1 v3 */
    	            glVertex3f(p1.x + tube_step_size*(-n1.x - b1.x), p1.y + tube_step_size*(-n1.y - b1.y), p1.z + tube_step_size*(-n1.z - b1.z));
					/* p1 v3 ground */
					glVertex3f(p1.x + tube_step_size*(-n1.x - b1.x), p1.y + tube_step_size*(-n1.y - b1.y), -3);
					/* p1 v0 ground */
					glVertex3f(p1.x + tube_step_size*( n1.x - b1.x), p1.y + tube_step_size*( n1.y - b1.y), -3);
    	            							
		        }/* end of for loop*/
    	    }/* end of for loop*/
    	}/* end of for loop*/
    glEnd();
	}/* end of for loop*/
}
/*----------------------- Opengl Default function ------------------------------*/
void myinit()
{	
	
	
  	glClearColor(0.0, 0.0, 0.0, 0.0);/* setup gl view here */
  	glEnable(GL_DEPTH_TEST);         /* enable depth buffering */
  	glShadeModel(GL_SMOOTH);         /* interpolate colors during rasterization */  
  
    // set up lighting 
    GLfloat light_ambient[] 	= {0.3, 0.3, 0.3, 1.0};
    GLfloat light_diffuse[] 	= {0.8, 0.8, 0.8, 1.0};
    GLfloat light_specular[] 	= {0.8, 0.8, 0.8, 1.0};
    GLfloat light_position[] 	= {eye.x, eye.y, eye.z, 0.0};
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    
    // spotlight
    GLfloat sd[] = {focus.x, focus.y, focus.z};
    glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, sd);
    glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, 90.0);
    glLightf(GL_LIGHT0, GL_SPOT_EXPONENT, 5.0);
  	
    // setup lighting
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);    	
    glEnable(GL_COLOR_MATERIAL);
     
}

/* reshape callback */
/* set projection to aspect ratio of window */
void reshape(int w, int h)
{
    GLfloat aspect = (GLfloat) w / (GLfloat) h;
    glViewport(0, 0, w, h);    /* scale viewport with window */

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    //gluPerspective(FOV, aspect, 1.0, 100.0);
    gluPerspective(FOV, aspect, 0.01, 1000.0);
    glMatrixMode(GL_MODELVIEW);

}

void doIdle()
{
  	/* do some stuff... */
	triggerSaveScreenshot();
  	
	if(START_ANIMATION)
		animation();  	/* make the screen update */
  	glutPostRedisplay();
}

/* draw 1x1 cube about origin */
/* replace this code with your height field implementation */
/* you may also want to precede it with your rotation/translation/scaling */
void display()
{
	/* clear buffers */
  	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  	/* reset transformation - identity matrix*/
  	glLoadIdentity();                                  /*get the (modelview) matrix into the original state*/
  	

  


	/* Set current matrix on the stack */
    glPushMatrix();				



  	/* transformation */
  	glTranslatef(g_vLandTranslate[0], g_vLandTranslate[1], g_vLandTranslate[2]);
  	glRotatef(g_vLandRotate[0], 1.0, 0.0, 0.0);
  	glRotatef(g_vLandRotate[1], 0.0, 1.0, 0.0);
  	glRotatef(g_vLandRotate[2], 0.0, 0.0, 1.0);
  	glScalef(g_vLandScale[0], g_vLandScale[1], g_vLandScale[2]);   	

 	arbitrary_point.x = 0.0;arbitrary_point.y = 0.0;arbitrary_point.z = -1.0; 	
 	camera();


  	
	/*  background */
	drawSkyAndGround(); 
	/* 3 main rails */
	drawSplines();
	/* cross rail between mail running rails */
	drawCrossRail();
	/* ground pillars */
  	drawSupportedPillar();


  	glFlush();
  	/* Pop the old matrix without the transformations */
    glPopMatrix();              
  	glutSwapBuffers();	  	
}

/*-----------------------User Input function ------------------------------*/
void menufunc(int value)
{
  switch (value)
  {
    case 0:
      exit(0);
      break;
  }
}

/* converts mouse drags into information about rotation/translation/scaling */
void mousedrag(int x, int y)
{
  int vMouseDelta[2] = {x-g_vMousePos[0], y-g_vMousePos[1]};
  
  switch (g_ControlState)
  {
    case TRANSLATE:  
      if (g_iLeftMouseButton)
      {
        g_vLandTranslate[0] += vMouseDelta[0]*0.01;
        g_vLandTranslate[1] -= vMouseDelta[1]*0.01;
      }
      if (g_iMiddleMouseButton)
      {
        g_vLandTranslate[2] += vMouseDelta[1]*0.01;
      }
      break;
    case ROTATE:
      if (g_iLeftMouseButton)
      {
        g_vLandRotate[0] += vMouseDelta[1];/* drag up = clockwise around x-axis */
        g_vLandRotate[1] += vMouseDelta[0];/* drag right = counter-clockwise around y-axis */
      }
      if (g_iMiddleMouseButton)
      {
        g_vLandRotate[2] += vMouseDelta[1];/* drag right = counter-clockwise around z-axis */
      }
      break;
    case SCALE:
      if (g_iLeftMouseButton)
      {
        g_vLandScale[0] *= 1.0+vMouseDelta[0]*0.01;/* drag right = scale up in x-direction */
        g_vLandScale[1] *= 1.0-vMouseDelta[1]*0.01;/* drag up = scale up in y-direction */
      }
      if (g_iMiddleMouseButton)
      {
        g_vLandScale[2] *= 1.0-vMouseDelta[1]*0.01;/* drag up = scale up in z-direction */
      }
      break;
  }
  g_vMousePos[0] = x;
  g_vMousePos[1] = y;
}

/*
	mouseidle - Idle mouse movement callback function
*/
void mouseidle(int x, int y)
{
	g_vMousePos[0] = x;
	g_vMousePos[1] = y;
}

/*
	mousebutton - Sets the global mouse states according to the actions
*/
/* mouse callback */
/* set state based on modifier key */
/* ctrl = translate, shift = scale, otherwise rotate */
void mousebutton(int button, int state, int x, int y)
{

  switch (button)
  {
    case GLUT_LEFT_BUTTON:
      g_iLeftMouseButton = (state==GLUT_DOWN);
      break;
    case GLUT_MIDDLE_BUTTON:
      g_iMiddleMouseButton = (state==GLUT_DOWN);
      break;
    case GLUT_RIGHT_BUTTON:
      g_iRightMouseButton = (state==GLUT_DOWN);
      break;
  }
 
  switch(glutGetModifiers())
  {
    case GLUT_ACTIVE_CTRL:
      g_ControlState = TRANSLATE;
      break;
    case GLUT_ACTIVE_SHIFT:
      g_ControlState = SCALE;
      break;
    default:
      g_ControlState = ROTATE;
      break;
  }

  g_vMousePos[0] = x;
  g_vMousePos[1] = y;
}

/* keyboard callback */
void keyboard(unsigned char key, int x, int y)
{
  switch (key) {
  case 27:
    exit(0);
    break;
	case '1'	:
		SAVE_SCREENSHOT = true;
		break;  
	case '2'	:
		SAVE_SCREENSHOT = false;
		break;	
	case '5'	:
		START_ANIMATION = true;
		break;		
	case '6'	:
		START_ANIMATION = false;
		break;	
	case 'a': case 'A':
		g_vLandRotate[2] += 2;  /*along z*/ 
		break;		 
	case 'd': case 'D':
		g_vLandRotate[2] -=2; 
		break;			
	case 'w': case 'W':
		g_vLandTranslate[0] -=0.01;g_vLandTranslate[1] -=0.01;/*along x,y*/
		break;
	case 's': case 'S':
		g_vLandTranslate[0] +=0.01;g_vLandTranslate[1] +=0.01;		 
		break;					
	case 'e': case 'E':
		g_vLandRotate[1] += 2; /*along y*/
		break;		 
	case 'c': case 'C':
		g_vLandRotate[1] -=2; /*along y*/
		break;		
	case 'z': case 'Z':
		g_vLandScale[0] *= 1.2;g_vLandScale[1] *= 1.2;g_vLandScale[2] *= 1.2;
		break;
	case 'q': case 'Q':
		g_vLandScale[0] *= 0.8;g_vLandScale[1] *= 0.8;g_vLandScale[2] *= 0.8;
		break; 	
	case 'o':
    	delta+=0.02; /* 38(up arrow)*/
        if(delta >= 1.0){
        	count_control_points++;
            delta = 0.0;
            if( count_control_points >= g_Splines[0].numControlPoints - 4)
              	count_control_points = 0;
        }
        break;	
	case 'l':
    	delta-=0.02; /* 38(up arrow)*/
        if(delta < 0.0){
            if( count_control_points <= 0)
              	count_control_points = g_Splines[0].numControlPoints - 4;
              	
        	count_control_points--;
            delta = 0.0;              	
              	
        }
        break;	      		
  }
  glutPostRedisplay();
}


int main (int argc, char ** argv)
{
  	if (argc<2)
  	{  
  	printf ("usage: %s <trackfile>\n", argv[0]);
  	exit(0);
  	}


  	
  	loadSplines(argv[1]);
  	
	
  	
		/* create window */
  	glutInit(&argc,argv);
  	glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGBA);  
  	glutInitWindowSize(WIDTH, HEIGHT);
  	glutInitWindowPosition(0, 0);  
  	glutCreateWindow("Assignment 2 - Roller Coaster");   
  	
  	/* allow the user to quit using the right mouse button menu */
  	g_iMenuId = glutCreateMenu(menufunc); /* menu triggered function */
  	glutSetMenu(g_iMenuId);
  	glutAddMenuEntry("Quit",0);   		/* Menu item = (menu title, value) */
  	glutAttachMenu(GLUT_RIGHT_BUTTON);	/* which button trigger menu show up*/
  	
  	/* replace with any animate code */
  	glutIdleFunc(doIdle);
  	
  	/*  set GLUT callbacks */
  	glutDisplayFunc(display);        /* display function to redraw */	
  	glutMotionFunc(mousedrag);       /* callback for mouse drags */
  	glutPassiveMotionFunc(mouseidle);/* callback for idle mouse */
  	glutMouseFunc(mousebutton);      /* callback for mouse button */
  	glutReshapeFunc(reshape);	       /* reshape callback */
  	glutKeyboardFunc(keyboard);	     /* keyboard callback */  
  	  
  	/* do initialization */
  	myinit();
  	
  	/* cannot put before diaply*/
  	loadTexture();  	
  	
	/* start GLUT program - affect window showing */
  	glutMainLoop();    
  	  
  	  
  	return 0;
}
