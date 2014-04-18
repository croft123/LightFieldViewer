/*
 *  LightSlab.h
 *  LightFieldViewer
 *
 *  Created by Sean Arietta on 4/9/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#pragma once
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
#include <GL/glut.h>
#include <GL/gl.h>
#include <math.h>

using namespace std;
using namespace cv;

class LightSlab
{
private:
	unsigned char* data;
	GLuint _texST;
	GLuint _texUV;
	
public:
	//unsigned char* data;
	int t_res;
	int s_res;
	int v_res;
	int u_res;
	
	Vec4f uv_corners[4];
	Vec4f st_corners[4];
	
	// Constructors
	LightSlab();
	LightSlab(string);
	LightSlab(string, int);
	
	// Sample the light field
	void sample(float s, float t, float u, float v, unsigned char*);
	void samplef(float s, float t, float u, float v, float*);
	void samplefST(float s, float t, float u, float v, float*);
	void samplefUV(float s, float t, float u, float v, float*);
	void samplefUVST(float s, float t, float u, float v, float*);
	
	// Slice the light field in either (u,v) or (s,t)
	void sliceUV(float u, float v, unsigned char* slice);
	void sliceST(float s, float t, unsigned char* slice);
	
	// Load a light slab
	void load(string filename);
	
	// Special functions just to display a slice
	void displaySliceUV(float u, float v);
	void displaySliceST(float s, float t);
	
	// Special functions to draw st and uv planes
	void drawSTPlane();
	void drawUVPlane();
};