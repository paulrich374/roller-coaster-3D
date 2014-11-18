AUTHOR/E-MAIL: Wei-Hung Liu, weihungl@usc.edu

![alt tag](https://raw.githubusercontent.com/weihungliu/roller-coaster-3D/master/animation_jpeg_files/030.jpg)

INSTALL:
--------------------------------
> cd pic
> make
> cd ..
> cd assign2
> make
> ./assign2 track.txt


FILES INCLUDED: 
--------------------------------
1) animation_jpeg_files/001.jpg-1000.jpg
2) assign2.cpp
3) ground1.jpg
4) Makefile
5) README.txt
6) sky6.jpg
7) splines/rollerCoaster.sp
8) track.txt



MANDATORY FEATURES:
--------------------------------
1) Properly render Catmull-Rom splines to represent your track.
	drawSplines()
	CatmullRom Equation: catmullRomSpline()
	float catmullRomSpline(float x, float v0,float v1,float v2,float v3)
	float tangent(float x, float v0,float v1,float v2,float v3)
	point crossProduct(point a, point b)
	point unit(point unit)


2) Render a texture-mapped ground and sky.
	drawSkyAndGround()
	Sky texture: sky6.jpg
	Ground texture: ground1.jpg

3) Render a rail cross-section.
	drawSplines()
	cross-section requires 4 vertices v0,v1,v2,v3, which are composed of n(normal vector) and b(binormal vector)
	where n and b are calculated from P0 and t(tangent vector)
	In other words, every generated spline points like P0 will derive out 4 other vertices which form a cross-section of rail
	However, we draw rail as square tubes which concatenated to each other
	Each square tube have 4 faces as described below (right, left, top, bottom)
	
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

4) Move the camera at a reasonable speed in a continuous path and orientation along the coaster.
	camera()

5) Render the coaster in an interesting manner (good visibility, realism).
	drawSplines()

6) Run at interactive frame rates (>15fps at 640x480)
	animation()

7) Be reasonably commented and written in an understandable manner--we will read your code.
	comment denoted within /* */

8) Be submitted along with JPEG frames for the required animation (see below).
	001.jpg-1000.jpg

9) Be submitted along with a readme file documenting your program's features and describing the approaches you took to each of the open-ended problems we posed here (rendering the sky and determining coaster normals in particular). This is especially crucial if you have done something spectacular for which you wish to receive extra credit!
	README.txt


OPTIONAL FEATURES:
--------------------------------
1) Render a T-shaped rail cross-section.
	drawSplines()
	as described in above item (3)

2) Render double rail (like in real railroad tracks).

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
3) Add OpenGL lighting to make your coaster look more realistic.
   
   Within myinit()   
   // set up lighting position: camera eye porition and using point source of light 
   GLfloat light_position[] 	= {eye.x, eye.y, eye.z, 0.0};
   // spotlight position: camera tangent vector    
   GLfloat sd[] = {focus.x, focus.y, focus.z};                                                            
                                         
4) Draw additional scene elements: texture-mapped wooden crossbars, a support structure that looks realistic, decorations on your track, something interesting in the world nearby, etc.
	
	a support structure: drawCrossRail()

	/*  CROSS SUPPORT BETWEEN V-SHAPED SQUARE TUBE RAIL   	                   

			
				                             	   
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
								  
					CROSS RAIL SUPPORT(RIGHT):	connect the point locates in index==3 the third rail which lies in the top-right of V-shaped 
								                to the point locates in	index==1 render the first rail which lies in the bottom of V-shapeds
					CROSS RAIL SUPPORT(LEFT):	connect the point locates in index==2 the second rail which lies in the top-left of V-shaped 
								                to the point locates in	index==1 render the first rail which lies in the bottom of V-shaped	       
	*/	
	
	
	a support structure: drawSupportedPillar()

	/*  PILAR(SQUARE TUBE) UNDER V-SHAPED SQUARE TUBE RAIL   	                   

			
				                             	                     
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
	
??) Make your track circular and close it with C1 continuity (small credit, i.e., 1 point).

??) Create tracks that mimic real world roller coasters such as [Magic Mountain] (close to Los Angeles; check out the Green Lantern in the [video]), or [Gardaland] in Italy.

??) Generate your track from several different sequences of splines (multiple track files), so you can shuffle these around and create a random track.

??) Draw splines using recursive subdivision (vary step size to draw short lines) instead of using brute force (vary u with fixed step size).

??) Render the environment in a better (prettier, more efficient, more interesting, etc?) manner than described here.

Decide coaster normals in a better manner than described here.

??) Modify the velocity with which your camera moves to make it physically realistic in terms of gravity. Please see the [equation] on how to update u for every time step.

??) Derive the steps that lead to the physically realistic equation of updating the u (i.e. u_new = u_old + (dt)(sqrt(2gh)/mag(dp/du)), see [here]).


INTERACTIVE CONTROL:
------------------------
// Exit  	
  	"esc: quit the program       																	"  
// Transformation 	 	
 	"a  : left rotate            																	"
 	"d  : right rotate           																	"
 	"w  : move forward           																	"
 	"s  : move backward          																	"
 	"q  : zoom out               																	"
 	"z  : zoom in                																	"
 	"e  : rotate right-handed along y             													"
 	"c  : rotate left-handed along y              													"
// Animation 	 	
 	"o  : move forward on the track   																 "
 	"l  : move back on the track              													    "
 	"1  : save screenshot        																	"
 	"2  : stop screenshot      																	    "		 		
 	"5  : start animation                                       									" 		
 	"6  : stop animation                     														"
// Transformation 	
 	"ctrl + mouse : translate                														"
 	"shift + mouse: scale                    														"
 	"mouse        : rotate                   														"

OTHER KNOWN BUGS/ISSUES:
------------------------





