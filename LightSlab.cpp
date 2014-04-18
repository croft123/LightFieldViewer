/*
 *  LightSlab.cpp
 *  LightFieldViewer
 *
 *  Created by Sean Arietta on 4/9/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#include "LightSlab.h"

// Taken from: http://www.lighthouse3d.com/opengl/glut/index.php?bmpfont
void renderBitmapString(
						float x, 
						float y, 
						float z, 
						void *font, 
						char *string) 
{  
	char *c;
	glRasterPos3f(x, y,z);
	for (c=string; *c != '\0'; c++) {
		glutBitmapCharacter(font, *c);
	}
}

// Constructors
LightSlab::LightSlab()
{
}

LightSlab::LightSlab(string filename)
{
	//string param = filename + "_parameters";
	//string data = filename + "_data";
	
	string param = filename + "_parameters.txt";
	string data = filename + "_data";

	// First read in parameters
	
	FILE* lfparamfile = fopen(param.c_str(), "r");
	
	if (lfparamfile == NULL) {
		printf("Could not open file %s", param.c_str()); 
		return;
	}
	
	int t_res, s_res, v_res, u_res;
	fscanf(lfparamfile, "%d %d %d %d", &u_res, &v_res, &s_res, &t_res);
	for (int i = 0; i < 4; i++)
	{
		float p1,p2,p3,p4;
		fscanf(lfparamfile, "%f %f %f %f %*f %*f", &p1, &p2, &p3, &p4);
		//this->uv_corners[i] = Vec<4,float>(p1,p2,p3,p4);
		this->uv_corners[i] = Vec4f(p1,p2,p3,p4);

	}
	for (int i = 0; i < 4; i++)
	{
		float p1,p2,p3,p4;
		fscanf(lfparamfile, "%f %f %f %f %*f %*f", &p1, &p2, &p3, &p4);
		this->st_corners[i] = Vec4f(p1,p2,p3,p4);
	}
	fclose(lfparamfile);
	
	this->t_res = t_res;
	this->s_res = s_res;
	this->v_res = v_res;
	this->u_res = u_res;
	
	this->data = (unsigned char*) malloc(sizeof(unsigned char) * t_res*s_res*v_res*u_res*3);
	
	// Then grab the data
	FILE *lfdatafile = fopen(data.c_str(), "rb");
	if (lfdatafile == NULL) {
		printf("Could not open file %s", data.c_str()); 
		return;
	}
	fread( this->data, t_res*s_res*v_res*u_res*3, 1, lfdatafile );
	fclose(lfdatafile);

	int debug =0;
	if(debug == 1)
	{
		CvSize mSize1;
		mSize1.height = 235;
		mSize1.width = 314;
		IplImage* image1 = cvCreateImage(mSize1, 8, 3);
		memcpy( image1->imageData, this->data, 3*mSize1.height*mSize1.width);
		cvNamedWindow( "Display Window4", WINDOW_AUTOSIZE);
		cvShowImage( "Display Window4", image1 );
		waitKey(0);
	}

	
	// Initialize the texture
	glGenTextures(1, &_texST);
	
	// select our current texture
	glBindTexture( GL_TEXTURE_2D, _texST );
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	
	glTexImage2D (GL_TEXTURE_2D, 0, GL_RGB, t_res, s_res, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glBindTexture(GL_TEXTURE_2D, 0);
	
	// Initialize the texture
	glGenTextures(1, &_texUV);
	
	// select our current texture
	glBindTexture( GL_TEXTURE_2D, _texUV );
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	
	glTexImage2D (GL_TEXTURE_2D, 0, GL_RGB, u_res, v_res, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glBindTexture(GL_TEXTURE_2D, 0);
	
	printf("Read %s:\nResolution: (%d,%d,%d,%d)\n", filename.c_str(), s_res, t_res, u_res, v_res);
}

LightSlab::LightSlab(string filename, int num)
{
	string param, data;
	char buf[255];
	sprintf(buf, "%d", num);
	param = filename + "_slab" + string(buf) + "_parameters";
	data = filename + "_slab" + string(buf) + "_data";
	
	// First read in parameters
	FILE* lfparamfile = fopen(param.c_str(), "r");
	
	if (lfparamfile == NULL) {
		printf("Could not open file %s", param.c_str()); 
		return;
	}
	
	int t_res, s_res, v_res, u_res;
	fscanf(lfparamfile, "%d %d %d %d", &u_res, &v_res, &s_res, &t_res);
	for (int i = 0; i < 4; i++)
	{
		float p1,p2,p3,p4;
		fscanf(lfparamfile, "%f %f %f %f %*f %*f", &p1, &p2, &p3, &p4);
		this->uv_corners[i] = Vec4f(p1,p2,p3,p4);
	}
	for (int i = 0; i < 4; i++)
	{
		float p1,p2,p3,p4;
		fscanf(lfparamfile, "%f %f %f %f %*f %*f", &p1, &p2, &p3, &p4);
		this->st_corners[i] = Vec4f(p1,p2,p3,p4);
	}
	fclose(lfparamfile);
		
	this->t_res = t_res;
	this->s_res = s_res;
	this->v_res = v_res;
	this->u_res = u_res;
	
	this->data = (unsigned char*) malloc(sizeof(unsigned char) * t_res*s_res*v_res*u_res*3);
	
	// Then grab the data
	FILE *lfdatafile = fopen(data.c_str(), "rb");
	fread( this->data, t_res*s_res*v_res*u_res*3, 1, lfdatafile );
	fclose(lfdatafile);
	
	// Initialize the texture
	glGenTextures(1, &_texST);
	
	// select our current texture
	glBindTexture( GL_TEXTURE_2D, _texST );
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	
	glTexImage2D (GL_TEXTURE_2D, 0, GL_RGB, s_res, t_res, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glBindTexture(GL_TEXTURE_2D, 0);
	
	// Initialize the texture
	glGenTextures(1, &_texUV);
	
	// select our current texture
	glBindTexture( GL_TEXTURE_2D, _texUV );
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	
	glTexImage2D (GL_TEXTURE_2D, 0, GL_RGB, u_res, v_res, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glBindTexture(GL_TEXTURE_2D, 0);
}

// Sample the light field
// Assume that the parameters are in the range (0,1)
void LightSlab::sample(float s, float t, float u, float v, unsigned char* trip)
{	
	// Map to discretized points in light slab
	int ti = (int) (t_res * t);
	int si = (int) (s_res * s);
	int ui = (int) (u_res * u);
	int vi = (int) (v_res * v);
	
	int mytest = ti*3 + si*t_res*3 + vi*t_res*s_res*3 + ui*t_res*s_res*v_res*3;
	if(mytest > 3*5*5*593*840)
		mytest = 3*5*5*593*840;

	
	trip[0] = data[ti*3 + si*t_res*3 + vi*t_res*s_res*3 + ui*t_res*s_res*v_res*3 + 2];
	trip[1] = data[ti*3 + si*t_res*3 + vi*t_res*s_res*3 + ui*t_res*s_res*v_res*3 + 1];
	trip[2] = data[ti*3 + si*t_res*3 + vi*t_res*s_res*3 + ui*t_res*s_res*v_res*3];
}

void LightSlab::samplef(float s, float t, float u, float v, float* trip)
{	
	// Map to discretized points in light slab
	int ti = (int) (t_res * t);
	int si = (int) (s_res * s);
	int ui = (int) (u_res * u);
	int vi = (int) (v_res * v);
	
	trip[0] = (data[ti*3 + si*t_res*3 + vi*t_res*s_res*3 + ui*t_res*s_res*v_res*3 + 2]) / 255.0;
	trip[1] = (data[ti*3 + si*t_res*3 + vi*t_res*s_res*3 + ui*t_res*s_res*v_res*3 + 1]) / 255.0;
	trip[2] = (data[ti*3 + si*t_res*3 + vi*t_res*s_res*3 + ui*t_res*s_res*v_res*3]) / 255.0;
}

void LightSlab::samplefST(float s, float t, float u, float v, float* trip)
{	
	// Map to discretized points in light slab
	float tf = ((float) t_res) * t;
	float sf = ((float) s_res) * s;
	int ui = (int) (u_res * u);
	int vi = (int) (v_res * v);
	
	int samples[4];
	int t1 = floor(tf);
	int t2 = ceil(tf);
	int s1 = floor(sf);
	int s2 = ceil(sf);
	
	if (t2 >= t_res)
		t2 = t_res-1;
	if (s2 >= s_res)	
		s2 = s_res-1;
	
	samples[0] = t1*3 + s1*t_res*3 + vi*t_res*s_res*3 + ui*t_res*s_res*v_res*3;
	samples[1] = t1*3 + s2*t_res*3 + vi*t_res*s_res*3 + ui*t_res*s_res*v_res*3;
	samples[2] = t2*3 + s1*t_res*3 + vi*t_res*s_res*3 + ui*t_res*s_res*v_res*3;
	samples[3] = t2*3 + s2*t_res*3 + vi*t_res*s_res*3 + ui*t_res*s_res*v_res*3;
	
	float c[4];
	c[0] = ((float) s2 - sf) * ((float) t2 - tf) / ((float) (s2-s1) * (t2-t1));
	c[1] = ((float) sf - s1) * ((float) t2 - tf) / ((float) (s2-s1) * (t2-t1));
	c[2] = ((float) s2 - sf) * ((float) tf - t1) / ((float) (s2-s1) * (t2-t1));
	c[3] = ((float) sf - s1) * ((float) tf - t1) / ((float) (s2-s1) * (t2-t1));
	
	trip[2] += (c[0] * (float) data[samples[0]] + c[1] * (float) data[samples[1]] + c[2] * (float) data[samples[2]] + c[3] * (float) data[samples[3]]) / 255.0;
	trip[1] += (c[0] * (float) data[samples[0]+1] + c[1] * (float) data[samples[1]+1] + c[2] * (float) data[samples[2]+1] + c[3] * (float) data[samples[3]+1]) / 255.0;
	trip[0] += (c[0] * (float) data[samples[0]+2] + c[1] * (float) data[samples[1]+2] + c[2] * (float) data[samples[2]+2] + c[3] * (float) data[samples[3]+2]) / 255.0;
}

void LightSlab::samplefUV(float s, float t, float u, float v, float* trip)
{	
	// Map to discretized points in light slab
	float vf = ((float) v_res) * v;
	float uf = ((float) u_res) * u;
	int si = (int) (s_res * s);
	int ti = (int) (t_res * t);
	
	int samples[4];
	int v1 = floor(vf);
	int v2 = ceil(vf);
	int u1 = floor(uf);
	int u2 = ceil(uf);
	
	if (v2 >= v_res)
		v2 = v_res-1;
	if (u2 >= u_res)	
		u2 = u_res-1;
	samples[0] = ti*3 + si*t_res*3 + v1*t_res*s_res*3 + u1*t_res*s_res*v_res*3;
	samples[1] = ti*3 + si*t_res*3 + v1*t_res*s_res*3 + u2*t_res*s_res*v_res*3;
	samples[2] = ti*3 + si*t_res*3 + v2*t_res*s_res*3 + u1*t_res*s_res*v_res*3;
	samples[3] = ti*3 + si*t_res*3 + v2*t_res*s_res*3 + u2*t_res*s_res*v_res*3;
	
	float c[4];
	c[0] = ((float) u2 - uf) * ((float) v2 - vf) / ((float) (u2-u1) * (v2-v1));
	c[1] = ((float) uf - u1) * ((float) v2 - vf) / ((float) (u2-u1) * (v2-v1));
	c[2] = ((float) u2 - uf) * ((float) vf - v1) / ((float) (u2-u1) * (v2-v1));
	c[3] = ((float) uf - u1) * ((float) vf - v1) / ((float) (u2-u1) * (v2-v1));
	
	trip[2] += (c[0] * (float) data[samples[0]] + c[1] * (float) data[samples[1]] + c[2] * (float) data[samples[2]] + c[3] * (float) data[samples[3]]) / 255.0;
	trip[1] += (c[0] * (float) data[samples[0]+1] + c[1] * (float) data[samples[1]+1] + c[2] * (float) data[samples[2]+1] + c[3] * (float) data[samples[3]+1]) / 255.0;
	trip[0] += (c[0] * (float) data[samples[0]+2] + c[1] * (float) data[samples[1]+2] + c[2] * (float) data[samples[2]+2] + c[3] * (float) data[samples[3]+2]) / 255.0;
}

void LightSlab::samplefUVST(float s, float t, float u, float v, float* trip)
{	
	// Map to discretized points in light slab
	float tf = ((float) t_res) * t;
	float sf = ((float) s_res) * s;
	float vf = ((float) v_res) * v;
	float uf = ((float) u_res) * u;
	
	float samples[16];
	int t1 = floor(tf);
	int t2 = ceil(tf);
	int s1 = floor(sf);
	int s2 = ceil(sf);
	int v1 = floor(vf);
	int v2 = ceil(vf);
	int u1 = floor(uf);
	int u2 = ceil(uf);
	
	if (v2 >= v_res)
		v2 = v_res-1;
	if (u2 >= u_res)	
		u2 = u_res-1;
	if (t2 >= t_res)
		t2 = t_res-1;
	if (s2 >= s_res)	
		s2 = s_res-1;
	
	for (int j = 0; j < 3; j++)
	{
		float alpha = ((float) s2 - sf) / ((float) (s2-s1));
		float beta = ((float) sf - s1) / ((float) (s2-s1));
		
		// Top of a cube
		samples[0] = (float) data[t1*3 + s1*t_res*3 + v1*t_res*s_res*3 + u1*t_res*s_res*v_res*3 + j];
		samples[1] = (float) data[t1*3 + s2*t_res*3 + v1*t_res*s_res*3 + u1*t_res*s_res*v_res*3 + j];
		samples[2] = (float) data[t2*3 + s1*t_res*3 + v1*t_res*s_res*3 + u1*t_res*s_res*v_res*3 + j];
		samples[3] = (float) data[t2*3 + s2*t_res*3 + v1*t_res*s_res*3 + u1*t_res*s_res*v_res*3 + j];
		// Bottom of a cube
		samples[4] = (float) data[t1*3 + s1*t_res*3 + v1*t_res*s_res*3 + u2*t_res*s_res*v_res*3 + j];
		samples[5] = (float) data[t1*3 + s2*t_res*3 + v1*t_res*s_res*3 + u2*t_res*s_res*v_res*3 + j];
		samples[6] = (float) data[t2*3 + s1*t_res*3 + v1*t_res*s_res*3 + u2*t_res*s_res*v_res*3 + j];
		samples[7] = (float) data[t2*3 + s2*t_res*3 + v1*t_res*s_res*3 + u2*t_res*s_res*v_res*3 + j];
		// Another cube
		samples[8] = (float) data[t1*3 + s1*t_res*3 + v2*t_res*s_res*3 + u1*t_res*s_res*v_res*3 + j];
		samples[9] = (float) data[t1*3 + s2*t_res*3 + v2*t_res*s_res*3 + u1*t_res*s_res*v_res*3 + j];
		samples[10] = (float) data[t2*3 + s1*t_res*3 + v2*t_res*s_res*3 + u1*t_res*s_res*v_res*3 + j];
		samples[11] = (float) data[t2*3 + s2*t_res*3 + v2*t_res*s_res*3 + u1*t_res*s_res*v_res*3 + j];
		// Bottom agajn
		samples[12] = (float) data[t1*3 + s1*t_res*3 + v2*t_res*s_res*3 + u2*t_res*s_res*v_res*3 + j];
		samples[13] = (float) data[t1*3 + s2*t_res*3 + v2*t_res*s_res*3 + u2*t_res*s_res*v_res*3 + j];
		samples[14] = (float) data[t2*3 + s1*t_res*3 + v2*t_res*s_res*3 + u2*t_res*s_res*v_res*3 + j];
		samples[15] = (float) data[t2*3 + s2*t_res*3 + v2*t_res*s_res*3 + u2*t_res*s_res*v_res*3 + j];
		
		// lerp in s
		float ss[8];
		float ts[4];
		float us[2];
		for (int i = 0; i < 16; i+=2)
			ss[i/2] = alpha*samples[i] + beta*samples[i+1];
		
		// bilerp in t
		alpha = ((float) t2 - tf) / ((float) (t2-t1));
		beta = ((float) tf - t1) / ((float) (t2-t1));
		for (int i = 0; i < 8; i+= 2)
			ts[i/2] = alpha*ss[i] + beta*ss[i+1];
		
		// trilerp in u
		alpha = ((float) u2 - uf) / ((float) (u2-u1));
		beta = ((float) uf - u1) / ((float) (u2-u1));
		for (int i = 0; i < 4; i+= 2)
			us[i/2] = alpha*ts[i] + beta*ts[i+1];
		
		// quadlerp in v
		alpha = ((float) v2 - vf) / ((float) (v2-v1));
		beta = ((float) vf - v1) / ((float) (v2-v1)); 
		
		trip[j] = (alpha*us[0] + beta*us[1]) / 255.0;
	}
}

// Slice the light field in either (u,v) or (s,t)
void LightSlab::sliceUV(float u, float v, unsigned char* slice)
{
	int index = 0;
	for (float s = 0; s < 0.99999; s += 1./((float) s_res))
	{
		for (float t = 0; t < 1.0; t += 1./((float) t_res))
		{
			this->sample(s, t, u, v, &slice[index]);
			index += 3;
		}
	}
	int test = 1;
}

void LightSlab::sliceST(float s, float t, unsigned char* slice)
{
	int index = 0;
	for (float u = 0; u < 1.0; u += 1./((float) u_res))
	{
		for (float v = 0; v < 1.0; v += 1./((float) v_res))
		{
			this->sample(s, t, u, v, &slice[index]);
			index += 3;
		}
	}
}

// Special functions just to display a slice
void LightSlab::displaySliceUV(float u, float v)
{
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(-1, 1, 1, -1);
	
	glDisable(GL_LIGHTING);
	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	glColor3f( 1.0f, 1.0f, 1.0f );
	
	int vi = (int) v;
	int ui = (int) u;
	// This is because we pas u and v in as (0, u_res) and (0, v_res)
	u = u / ((float) u_res);
	v = v / ((float) v_res);
	
	glBindTexture(GL_TEXTURE_2D, _texST);
	unsigned char* slice = (unsigned char*) malloc(sizeof(unsigned char) * s_res * t_res * 3);
	this->sliceUV(u,v,slice);
	
	int debug = 0;
	if(debug == 1)
	{
		CvSize mSize1;
		mSize1.height = s_res;
		mSize1.width = t_res;
		IplImage* image1 = cvCreateImage(mSize1, 8, 3);
		//memcpy( image1->imageData, input, 3*mSize.height*mSize.width);
		for(int s=0;s<s_res;s++)
		{
			for(int t=0;t<t_res;t++)
			{
				*(image1->imageData + s*image1->widthStep+3*t) = slice[t_res*3*s+3*t];
				*(image1->imageData + image1->widthStep*s+3*t+1) = slice[t_res*3*s+3*t+1];
				*(image1->imageData + image1->widthStep*s+3*t+2) = slice[t_res*3*s+3*t+2];
			
			}
		}
		cvNamedWindow( "Display Window5", WINDOW_AUTOSIZE);
		cvShowImage( "Display Window5", image1 );
		waitKey(0);
	}

	//glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	//glPixelStorei(GL_UNPACK_ROW_LENGTH, s_res);
	//glTexSubImage2D (GL_TEXTURE_2D, 0, 0, 0, t_res,s_res, GL_RGB, GL_UNSIGNED_BYTE,slice);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, t_res,s_res, 0, GL_RGB, GL_UNSIGNED_BYTE, slice);
	//glTexImage2D(GL_TEXTURE_2D,3,0,s_res,t_res,0,GL_RGB,GL_UNSIGNED_BYTE,slice);

	//&data[s_res * t_res * 3 * vi + s_res * t_res * v_res * 3 * ui]
	glBegin (GL_QUADS);
		glTexCoord2f (0, 0);
		glVertex3f (-1,-1,0);
		glTexCoord2f (1, 0);
		glVertex3f (1,-1,0);
		glTexCoord2f (1, 1);
		glVertex3f (1,1,0);
		glTexCoord2f (0, 1);
		glVertex3f (-1,1,0);
	glEnd ();
	
	glBindTexture(GL_TEXTURE_2D, 0);
	glColor3f(0.0, 1.0, 0.0);
	char legend[255];
	sprintf(legend, "(u,v) = (%i,%i)\n", ui, vi);
	renderBitmapString(-0.95, -0.95, 0, GLUT_BITMAP_HELVETICA_10, legend);
	
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glEnable(GL_LIGHTING);
	glDisable(GL_BLEND);
	
	free(slice);
}

void LightSlab::displaySliceST(float s, float t)
{
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(-1, 1, 1, -1);
	
	glDisable(GL_LIGHTING);
	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	glColor3f( 1.0f, 1.0f, 1.0f );
	
	int si = (int) s;
	int ti = (int) t;
	// This is because we pas u and v in as (0, u_res) and (0, v_res)
	s = s / ((float) s_res);
	t = t / ((float) t_res);
	
	glBindTexture(GL_TEXTURE_2D, _texUV);
	unsigned char* slice = (unsigned char*) malloc(sizeof(unsigned char) * u_res * v_res * 3);
	this->sliceST(s,t,slice);
	//glTexSubImage2D (GL_TEXTURE_2D, 0, 0, 0, v_res, u_res, GL_RGB, GL_UNSIGNED_BYTE, slice);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, v_res,u_res, 0, GL_RGB, GL_UNSIGNED_BYTE, slice);

	glBegin (GL_QUADS);
		glTexCoord2f (0, 0);
		glVertex3f (-1,-1,0);
		glTexCoord2f (1, 0);
		glVertex3f (1,-1,0);
		glTexCoord2f (1, 1);
		glVertex3f (1,1,0);
		glTexCoord2f (0, 1);
		glVertex3f (-1,1,0);
	glEnd ();
	
	glBindTexture(GL_TEXTURE_2D, 0);
	glColor3f(0.0, 1.0, 0.0);
	char legend[255];
	sprintf(legend, "(s,t) = (%i,%i)\n", si, ti);
	renderBitmapString(-0.95, -0.95, 0, GLUT_BITMAP_HELVETICA_10, legend);
	
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glEnable(GL_LIGHTING);
	glDisable(GL_BLEND);
}

// Load a light slab
void LightSlab::load(string filename)
{
}

void LightSlab::drawSTPlane()
{
	glBegin(GL_QUADS);
		const Vec4f &v0 = this->st_corners[0];
		const Vec4f &v1 = this->st_corners[1];
		const Vec4f &v2 = this->st_corners[2];
		const Vec4f &v3 = this->st_corners[3];
		float myv0[4],myv1[4],myv2[4],myv3[4];
		for(int i=0;i<4;i++)
		{
			myv0[i] = v0[i];
			myv1[i] = v1[i];
			myv2[i] = v2[i];
			myv3[i] = v3[i];
		}

		glColor4f(0,0,0,0);
		glVertex4fv(myv0);
		glColor4f(1,0,0,0);
		glVertex4fv(myv1);
		glColor4f(1,1,0,0);
		glVertex4fv(myv2);
		glColor4f(0,1,0,0);
		glVertex4fv(myv3);
	glEnd();
}

void LightSlab::drawUVPlane()
{
	//this->displaySliceUV(0,0);
	glBegin(GL_QUADS);
		const Vec4f &v0 = this->uv_corners[0];
		const Vec4f &v1 = this->uv_corners[1];
		const Vec4f &v2 = this->uv_corners[2];
		const Vec4f &v3 = this->uv_corners[3];
		
		float myv0[4],myv1[4],myv2[4],myv3[4];
		for(int i=0;i<4;i++)
		{
			myv0[i] = v0[i];
			myv1[i] = v1[i];
			myv2[i] = v2[i];
			myv3[i] = v3[i];
		}

		glColor4f(0,0,0,0);
		glVertex4fv(myv0);
		glColor4f(1,0,0,0);
		glVertex4fv(myv1);
		glColor4f(1,1,0,0);
		glVertex4fv(myv2);
		glColor4f(0,1,0,0);
		glVertex4fv(myv3);
	glEnd();
}