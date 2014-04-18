
#include <stdio.h>
#include <iostream>
#include "opencv2/core/core.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <opencv2/calib3d/calib3d.hpp>
#include "opencv2/nonfree/features2d.hpp"
#include <opencv2/core/core.hpp>




#include <stdlib.h>
//#include <windows.h>
#include <GL/glew.h>
#include <GL/glut.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include <math.h>

#include "XForm.h"
#include "GLCamera.h"
#include "LightSlab.h"
#include "ReadInDataSet.h"
#include "dfsFolder.h"

using namespace std;
using namespace cv;

#define DEBUG 0
#define WIDTH 768
#define HEIGHT 512

typedef struct
{
	Vec3f top_left;
	Vec3f top_right;
	Vec3f btm_right;
	Vec3f btm_left;
} MyRectangle;

// Globals
GLCamera camera;
xform xf;
xform xf_0;
vector<LightSlab> ls;
MyRectangle R;

Vec3f center(0,0,0);
double size = 5.0;
float select_u = 0;
float select_v = 0;
float select_s = 0;
float select_t = 0;

bool draw_edges = false;
bool show_frame = false;
bool show_st = false;
bool save_image = false;
short lerp_mode = 0;
GLuint texture = 0;
GLuint fb[2] = {0};
GLuint fb_texture = 0;
GLuint rb[2];

void setup_lighting()
{
	glEnable(GL_LIGHTING);
	GLfloat mat_front[4] = { 1.0, 0.95, 0.9, 1 };
	GLfloat mat_back[4] = { 0.7, 0.6, 0.6, 1 };
	GLfloat mat_specular[4] = { 0.18, 0.18, 0.18, 0.18 };
	GLfloat mat_shininess[] = { 64 };
	GLfloat global_ambient[] = { 0.02, 0.02, 0.05, 0.05 };
	GLfloat light0_ambient[] = { 0, 0, 0, 0 };
	GLfloat light0_diffuse[] = { 0.85, 0.85, 0.8, 0.85 };
	GLfloat light0_specular[] = { 0.85, 0.85, 0.85, 0.85 };
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, mat_front);
	glMaterialfv(GL_BACK, GL_AMBIENT_AND_DIFFUSE, mat_back);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, mat_shininess);
	glLightfv(GL_LIGHT0, GL_AMBIENT, light0_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light0_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light0_specular);
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, global_ambient);
	glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_FALSE);
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
	glEnable(GL_LIGHT0);
}

void draw_lightfield()
{
	float* slice = (float*) calloc(WIDTH * HEIGHT * 3, sizeof(float));
	
	for (int s = 0; s < ls.size(); s++)
	{
		float* data0 = (float*) malloc(sizeof(float) * WIDTH * HEIGHT * 4);
		float* data1 = (float*) malloc(sizeof(float) * WIDTH * HEIGHT * 4);
		
		/*
		 FIRST SQUARE IS RENDERED INTO THE FIRST FRAME BUFFER OBJECT
		 */
		
		// Make our back frame buffer current
		glBindFramebuffer(GL_FRAMEBUFFER_EXT, fb[0]);
		
		// Setup
		glClearColor(1, 1, 1, 1);
		glClearDepth(1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		// Matrix Projections
		xf_0 = xform::trans(0, 0, -5.0f * size) * xform::trans(-center);
		glPushMatrix();
		glMultMatrixd(xf_0);
		// Settings
		glDisable(GL_CULL_FACE);
		glDisable(GL_LIGHTING);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_BLEND);
		// Draw
		//ls[s].drawUVPlane();
		ls[s].drawSTPlane();
		//draw_frame();
		//ls[s].displaySliceST(1,0);

		// Bind the renderbuffer
		glBindRenderbuffer(GL_RENDERBUFFER_EXT, rb[0]);
		// Copy down the data
		glReadPixels(0, 0, WIDTH, HEIGHT, GL_RGBA, GL_FLOAT, data0);
		glPopMatrix();
		
		/*
		 SECOND SQUARE IS RENDERED INTO THE SECOND FRAME BUFFER OBJECT
		 */
		glPushMatrix();
		glMultMatrixd(xf);
				// Settings
		glDisable(GL_CULL_FACE);
		glDisable(GL_LIGHTING);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_BLEND);
		// Make our back frame buffer current
		glBindFramebuffer(GL_FRAMEBUFFER_EXT, fb[1]);
		
		// Setup
		glClearColor(1, 1, 1, 1);
		glClearDepth(1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		// Draw
		//ls[s].drawSTPlane();
		ls[s].drawUVPlane();
		//ls[s].displaySliceUV(0,0);
		// Bind the renderbuffer
		glBindRenderbuffer(GL_RENDERBUFFER_EXT, rb[1]);
		// Copy down the data
		glReadPixels(0, 0, WIDTH, HEIGHT, GL_RGBA, GL_FLOAT, data1);	
	// Set the framebuffer back to the window
		glBindFramebuffer(GL_FRAMEBUFFER_EXT, 0);
		
		glPopMatrix();
		glEnable(GL_LIGHTING);

		
		// Now all of the data we need is in data0 and data1, but we need to combine them
		float* data = (float*) malloc(sizeof(float) * WIDTH * HEIGHT * 4);
		for (int i = 0; i < WIDTH*HEIGHT*4; i+= 4)
		{
			data[i] = data0[i];
			data[i+1] = data0[i+1];
			data[i+2] = data1[i];
			data[i+3] = data1[i+1];
			//data[i+2] = data1[i];
			//data[i+3] = data1[i+1];
		}
		
		// Cleanup memory
		free(data0);
		free(data1);
		
		// Save for debugging
		if (0)
		{
			FILE* file = fopen("image.txt", "w");
			for (int i = 0; i < WIDTH * HEIGHT * 4; i+=4)
			{
				if (i % (WIDTH*4) == 0)
					fprintf(file, "\n");
				
				fprintf(file, "(%2.2f, %2.2f, %2.2f, %2.2f)\t", data[i], data[i + 1], data[i + 2], data[i + 3]);
			}
			fclose(file);
		}

		
		// Should have data = (s,t,u,v) at each pixel in the image
		// Data is laid out with the first element being the bottom left corner of the 
		// screen and inreasing fastest in the x direction
		
		int count = 0;
		for (int i = 0; i < WIDTH*HEIGHT*4; i+=4)
		{
			if (data[i] != 1.0 && data[i+1] != 1.0 && data[i+2] != 1.0 && data[i+3] != 1.0)
			{
				if(data[i+2] != 0)
					int test = 1;
				if (lerp_mode == 0)
					ls[s].samplefUV(data[i], data[i+1], data[i+2], data[i+3], &slice[count]);
				else if (lerp_mode == 1)
					ls[s].samplefST(data[i], data[i+1], data[i+2], data[i+3], &slice[count]);
				else if (lerp_mode == 2)
					ls[s].samplefUVST(data[i], data[i+1], data[i+2], data[i+3], &slice[count]);
				else
					ls[s].samplef(data[i], data[i+1], data[i+2], data[i+3], &slice[count]);
			}
			else
			{
				slice[count] =1.0;
				slice[count+1] =1.0;
				slice[count+2] =1.0;

			}
			count += 3;
		}
		
		// Cleanup memory
		free(data);
	}

	int debug = 0;
	if(debug == 1)
	{
		CvSize mSize1;
		mSize1.height = HEIGHT;
		mSize1.width = WIDTH;
		IplImage* image1 = cvCreateImage(mSize1, 8, 3);
		//memcpy( image1->imageData, input, 3*mSize.height*mSize.width);
		for(int s=0;s<mSize1.height;s++)
		{
			for(int t=0;t<mSize1.width;t++)
			{
				*(image1->imageData + s*image1->widthStep+3*t) = slice[mSize1.width*3*s+3*t]*255;
				*(image1->imageData + image1->widthStep*s+3*t+1) = slice[mSize1.width*3*s+3*t+1]*255;
				*(image1->imageData + image1->widthStep*s+3*t+2) = slice[mSize1.width*3*s+3*t+2]*255;
			
			}
		}
		cvNamedWindow( "Display Window6", WINDOW_AUTOSIZE);
		cvShowImage( "Display Window6", image1 );
		waitKey(0);
		cvDestroyWindow("Display Window6");
	}
	
	// Save for debugging
	if (DEBUG)
	{
		FILE* file = fopen("slice.txt", "w");
		for (int i = 0; i < WIDTH * HEIGHT * 3; i+=3)
		{
			if (i % (WIDTH*3) == 0)
				fprintf(file, "\n");
			
			fprintf(file, "(%4.4f, %4.4f, %4.4f)\t", slice[i], slice[i + 1], slice[i + 2]);
		}
		fclose(file);
	}
	
	// Draw to screen
	
	glWindowPos2i(0,0);

	/*glClearColor(0, 0, 0, 0);
	glClearDepth(1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	*/
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(-1, 1, 1, -1);
	
	glDisable(GL_LIGHTING);
	
	glDrawPixels(WIDTH, HEIGHT, GL_RGB, GL_FLOAT, slice);
	free(slice);
	
	/*glBegin (GL_QUADS);
	glColor3f(1.0, 0.0, 0.0);
	glVertex3f (-1,-1,0);
	glVertex3f (1,-1,0);
	glVertex3f (1,1,0);
	glVertex3f (-1,1,0);
	glEnd ();
	*/
	
	glPopMatrix();
	glEnable(GL_LIGHTING);
	glMatrixMode(GL_MODELVIEW);
}

void check_dp()
{
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(-1, 1, 1, -1);
	GLsizei w = 512;
	
	unsigned char* img = (unsigned char*) malloc(sizeof(unsigned char) * w*w*3);
	int j;
	int count = 0;
	for (j = 0; j < w*w*3; j++) 
	{
		img[j] = 256 * j / (w*w*3);
	}
	glDrawPixels(w, w, GL_RGB, GL_UNSIGNED_BYTE, img);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	
	free(img);
}

void draw_rect(bool st)
{
	glPushMatrix();
	glMultMatrixd(xf);
	
	glDepthFunc(GL_LESS);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	glDisable(GL_CULL_FACE);
	glDisable(GL_LIGHTING);
	
	for (int i = 0; i < ls.size(); i++)
		ls[i].drawSTPlane();
	//glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	for (int i = 0; i < ls.size(); i++)
		ls[i].drawUVPlane();
		
	glPopMatrix();
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
}

void draw_frame()
{
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	//glMultMatrixd(xf);
	glLoadIdentity();
	gluOrtho2D(-1, 1, 1, -1);
	
	glDisable(GL_CULL_FACE);
	glDisable(GL_LIGHTING);
	glEnable( GL_BLEND );
	glBlendFunc( GL_ONE, GL_ONE_MINUS_SRC_ALPHA );
	glColor3f( 1.0f, 1.0f, 1.0f );
	
	glBindTexture(GL_TEXTURE_2D, fb_texture);
	glBegin (GL_QUADS);
		glTexCoord2f (0, 1);
		glVertex3f (-1,-1,0);
	
		glTexCoord2f (1, 1);
		glVertex3f (1,-1,0);
	
		glTexCoord2f (1, 0);
		glVertex3f (1,1,0);
	
		glTexCoord2f (0, 0);
		glVertex3f (-1,1,0);
	glEnd ();
	
	glBindTexture(GL_TEXTURE_2D, 0);
	
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glEnable(GL_LIGHTING);
	glDisable(GL_BLEND);
}

void create_fbo()
{
	GLenum status;	


	glGenFramebuffers(2, fb);
	
	// Set up the FBO with one texture attachment
	glBindFramebuffer(GL_FRAMEBUFFER_EXT, fb[0]);
	
	// Create the RBO and attach
	glGenRenderbuffers(2, rb);
	glBindRenderbuffer(GL_RENDERBUFFER_EXT, rb[0]);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, WIDTH, HEIGHT);

	//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureId, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_RENDERBUFFER_EXT, rb[0]);
	
	// Set up the FBO with one texture attachment
	glBindFramebuffer(GL_FRAMEBUFFER, fb[1]);
	
	// Create the RBO and attach
	glBindRenderbuffer(GL_RENDERBUFFER_EXT, rb[1]);
	glRenderbufferStorage(GL_RENDERBUFFER_EXT, GL_RGBA8, WIDTH, HEIGHT);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rb[1]);

	status = glCheckFramebufferStatus(GL_FRAMEBUFFER_EXT);
	if (status != GL_FRAMEBUFFER_COMPLETE_EXT)
		printf("Error!\n");
	
	
	/*glGenTextures(1, &fb_texture);
	glBindTexture(GL_TEXTURE_2D, fb_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, WIDTH, HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	
	status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	if (status != GL_FRAMEBUFFER_COMPLETE_EXT)
		printf("Error!\n");*/
	 

	
	glBindFramebuffer(GL_FRAMEBUFFER_EXT, 0);
}
	
void delete_fbo()
{
	glDeleteTextures(1, &fb_texture);
	glDeleteFramebuffers(1, fb);
}

void capture_frame()
{	
	
	// Make our back frame buffer current
	glBindFramebuffer(GL_FRAMEBUFFER_EXT, fb[0]);
	
	// Render the square
	glDisable(GL_TEXTURE_2D);
	glClearColor(0, 0, 0, 0);
	glClearDepth(1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	draw_rect(show_st);
	glEnable(GL_TEXTURE_2D);
	
	// Set the framebuffer back to the window
	glBindFramebuffer(GL_FRAMEBUFFER_EXT, 0);
	
	// Bind the renderbuffer
	glBindRenderbuffer(GL_RENDERBUFFER_EXT, rb[0]);
	
	// Copy down the data
	float* data = (float*) malloc(sizeof(float) * WIDTH * HEIGHT * 4);
	glReadPixels(0, 0, WIDTH, HEIGHT, GL_RGBA, GL_FLOAT, data);
	
	// Copy into texture
	glBindTexture(GL_TEXTURE_2D, fb_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, WIDTH, HEIGHT, 0, GL_RGBA, GL_FLOAT, data);
	//glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, WIDTH, HEIGHT, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	
	// Save for debugging
	if (DEBUG)
	{
		FILE* file = fopen("image.txt", "w");
		for (int i = 0; i < WIDTH * HEIGHT * 4; i+=4)
		{
			if (i % (WIDTH*4) == 0)
				fprintf(file, "\n");
			
			fprintf(file, "(%2.2f, %2.2f, %2.2f, %2.2f)\t", data[i], data[i + 1], data[i + 2], data[i + 3]);
		}
		fclose(file);
	}
	
	free(data);
}

// Draw the complete scene
void redraw()
{
	camera.setupGL(xf * center, size);
	glClearColor(1, 1, 1, 1);
	glClearDepth(1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDisable(GL_CULL_FACE);
	setup_lighting();

	if (draw_edges && show_st)
		ls[0].displaySliceUV(select_u, select_v);
	else 
		if (draw_edges && !show_st)
			ls[0].displaySliceST(select_s, select_t);
		else 
			if (show_frame)
				draw_lightfield();
			else
				draw_rect(show_st);
	
	/*if (save_image)  /// my
	{
		glWindowPos2i(0,0);
		unsigned char* image = (unsigned char*) malloc(sizeof(unsigned char) * WIDTH * HEIGHT * 3);
		glReadPixels(0, 0, WIDTH, HEIGHT, GL_RGB, GL_UNSIGNED_BYTE, image);
		char* err;
		if (!dm_write_pngfile("image.png", image, WIDTH, HEIGHT, err))
			printf("ERROR: Couldn't save image!\n");
		free(image);
		//free(err);
		
		save_image = false;
	}*/

	glutSwapBuffers();
}

// Handle mouse button and motion events
static unsigned buttonstate = 0;

// Reset the view...
void resetview()
{
	xf = xform::trans(0, 0, -5.0f * size) * xform::trans(-center);
	camera.stopspin();
	buttonstate = 0;
}

void mousemotionfunc(int x, int y)
{
	static const Mouse::button physical_to_logical_map[] = {
		Mouse::NONE, Mouse::ROTATE, Mouse::MOVEXY, Mouse::MOVEZ,
		Mouse::MOVEZ, Mouse::MOVEXY, Mouse::MOVEXY, Mouse::MOVEXY,
	};
	
	Mouse::button b = Mouse::NONE;
	
	if (buttonstate & (1 << 4))
		b = Mouse::WHEELUP;
	else if (buttonstate & (1 << 3))
		b = Mouse::WHEELDOWN;
	else
		b = physical_to_logical_map[buttonstate & 7];
	b = Mouse::MOVEXY;
	camera.mouse(x, y, b, xf * center, size, xf);
	if (b != Mouse::NONE)
		glutPostRedisplay();
}

void mousebuttonfunc(int button, int state, int x, int y)
{
	if (state == GLUT_DOWN)
		buttonstate |= (1 << button);
	else
		buttonstate = 0;//buttonstate &= ~(1 << button);
	
	mousemotionfunc(x, y);
}

// Handle keyboard events
void keyboardfunc(unsigned char key, int x, int y)
{
	switch (key) {
		case ' ':
			resetview(); break;
		case 'e':
			draw_edges = !draw_edges; break;
		case '\033': // Esc
		case 'Q':
		case 'q':
			exit(0);
		case 'x':
			show_st = !show_st; break;
		case 'f':
			capture_frame(); show_frame = !show_frame; break;
		case 'k':
			select_v = select_v < ls[0].v_res-1 ? select_v+1 : select_v; break;
		case 'h':
			select_v = select_v > 0 ? select_v-1 : select_v; break;
		case 'u':
			select_u = select_u < ls[0].u_res-1 ? select_u+1 : select_u; break;
		case 'j':
			select_u = select_u > 0 ? select_u-1 : select_u; break;
		case 'd':
			select_t = select_t < ls[0].t_res-1 ? select_t+1 : select_t; break;
		case 'a':
			select_t = select_t > 0 ? select_t-1 : select_t; break;
		case 'w':
			select_s = select_s < ls[0].s_res-1 ? select_s+1 : select_s; break;
		case 's':
			select_s = select_s > 0 ? select_s-1 : select_s; break;
		case 'l':
			lerp_mode = lerp_mode == 3 ? 0 : lerp_mode+1; break;
		case 'z':
			save_image = true; break;
	}
	glutPostRedisplay();
}

void usage(const char *myname)
{
	fprintf(stderr, "usage: %s\n", myname);
	exit(1);
}

void init(const char* in, char c)
{
	create_fbo();
	if (c == 'm')
	{
		for (int i = 1; i <= 4; i++)
			ls.push_back(LightSlab(string(in), i));
	} else
	{
		ls.push_back(LightSlab(string(in)));
	}
	
	// Initialize the texture
	glGenTextures(1, &texture);
	
	// select our current texture
	glBindTexture( GL_TEXTURE_2D, texture );

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	
	//glTexImage2D (GL_TEXTURE_2D, 0, GL_RGB, ls[0].t_res, ls[0].s_res, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexImage2D (GL_TEXTURE_2D, 0, GL_RGB, ls[0].t_res, ls[0].s_res, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
	
	R.top_right = Vec3f(-1, 1, 0);
	R.top_right = Vec3f(1, 1, 0);
	R.btm_right = Vec3f(1, -1, 0);
	R.btm_left = Vec3f(-1, -1, 0);
}

int main( int argc, char** argv )
{
	glutInitWindowPosition(100, 0);
	glutInitWindowSize(WIDTH,HEIGHT);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInit(&argc, argv);
	glutCreateWindow("Light Field Viewer");
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		fprintf(stderr,"Error: %s\n to initialize glew",glewGetErrorString(err));
	}
	glEnable(GL_TEXTURE_2D);
	
	// Register callback functions for drawing, mouse, and keyboard events
	glutDisplayFunc(redraw);
	glutMouseFunc(mousebuttonfunc);
	glutMotionFunc(mousemotionfunc);
	glutKeyboardFunc(keyboardfunc);
	
	// Reset camera parameters
	resetview();
	
	if (argc < 1)
		usage("Viewer.app filename [s|m]");
	string filepath = "C:\\HomeWork&Project\\CS684\\LightFields_MALTAB\\LightFields-master\\DragonAndBunnies\\DragonsAndBunnies_5x5_ap5\\";
	string data = "Dragons";
	int WriteTXT = 0;
	if(WriteTXT == 1)
	{
		string dataTXT = "C:\\" + data + ".txt";
		ofstream fout( dataTXT, ios::app);
		dfsFolder(filepath, fout);
	}
	ReadInImages(filepath,data);
	
	
	string filename = filepath + data;
	init(filename.c_str(),0);
	// Enter event loop
	//glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glutMainLoop();
}
