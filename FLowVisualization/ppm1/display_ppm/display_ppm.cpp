/*
A program to read and write an image of .ppm format (the image must be a squared image)
Eugene Zhang, March 2005
*/

#include <fstream>

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <glut.h>

#include <string>
#include <iostream>

#include "icVector.H"
#include "icMatrix.H"
#include "ppm.h"

using namespace std;

#define MAX_TEXTURE_SIZE 1024

struct pixel{
    float x;
    float y;
    float scalar;
};

struct color{
    float r;
    float g;
    float b;
};

char texture_name[128];
const int win_width=512;
const int win_height=512;
int pixels_number;
char *progname;
float intensity_max, intensity_min, intensity_range;

int flag_ppm_blurred;
unsigned char *pixels_input, *pixels_output, *scalar_field;
float *scalar_field2;
int saveCount;

pixel v[4];
string notes[] = { "a: Compute the intensity of the image with using Average method", "b: Compute the intensity of the image with using Colorimetric method" };

void initialize_system();
void finalize_system();
void usage(char *progname);
void init(void);
void keyboard_input_window(unsigned char key, int x, int y);
void keyboard_output_window(unsigned char key, int x, int y);

color getRainbowColor(float intensity);
color getHeatColor(float intensity);
color getBlue_WhiteColor(float intensity);

//Display function for the input image
void display_image_input_window(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDrawBuffer(GL_BACK);
	glDrawPixels(win_width, win_height, GL_RGB, GL_UNSIGNED_BYTE, pixels_input);
	glutSwapBuffers();
}

//Display function for the output image
void display_image_output_window(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-1.0, 1.0, -1.0, 1.0, 0.0, 40.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glOrtho(-0.5, 0.5, -0.5, 0.5, 0.0, -40.0);


	glEnable(GL_TEXTURE_2D);
	glShadeModel(GL_FLAT);
	glEnable(GL_DEPTH_TEST);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDisable (GL_BLEND);
	glDisable(GL_LIGHTING);

    //glBegin(GL_LINES);
    //glColor3f(1.0, 0.0, 0.0);
    //glVertex3d(0.0, 0.0, -4.0);
    //glVertex3d(0.4, 0.4, -4.0);
    //glEnd();

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, win_width, win_height, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels_output);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);     // this multiplies texture with lighting color
    
    glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0);	glVertex3d(-0.5, -0.5, -4.0);
	glTexCoord2f(0.0, 1.0);	glVertex3d(-0.5, 0.5, -4.0);
	glTexCoord2f(1.0, 1.0);	glVertex3d(0.5, 0.5, -4.0);
	glTexCoord2f(1.0, 0.0);	glVertex3d(0.5, -0.5, -4.0);
	glEnd();

	glutSwapBuffers();
}

//matching square
void drawLine(float v1_x, float v1_y, float v2_x, float v2_y, color c){
    glLineWidth(1.5);
    glColor3f(c.r, c.g, c.b);
    //glColor3f(1.0, 0.0, 0.0);

    glBegin(GL_LINES);
    //glColor3f(1.0, 0.0, 0.0);
    glVertex3d(v1_x, v1_y,0.0);
    glVertex3d(v2_x, v2_y,0.0);
    glEnd();
    glFlush();
}

void drawLine_height(float v1_x, float v1_y, float v2_x, float v2_y,float w, color c){
    glLineWidth(1.5);
    glColor3f(0.0, 0.0, 0.0);
    //glColor3f(1.0, 0.0, 0.0);

    glBegin(GL_LINES);
    //glColor3f(1.0, 0.0, 0.0);
    glVertex3d(v1_x, w+5, -v1_y);
    glVertex3d(v2_x, w+5, -v2_y);
    glEnd();
    glFlush();
}

//pixels should be counterclock-wise
pixel getInterpolationPoint(pixel p1, pixel p2, float w){
    pixel v1;
        v1.scalar = w;
        v1.x = p2.x + (w - p2.scalar) / (p1.scalar - p2.scalar)*(p1.x - p2.x);
        v1.y = p2.y + (w - p2.scalar) / (p1.scalar - p2.scalar)*(p1.y - p2.y);
        return v1;
}


void drawCountourLine(pixel p1, pixel p2, pixel p3, pixel p4, float w, color c, int flag){
    int n = 0;
    if (p1.scalar >= w && p2.scalar <= w || p1.scalar<=w && p2.scalar>=w){
        v[n] = getInterpolationPoint(p1, p2, w);
        n++;
    }
    if (p2.scalar >= w && p3.scalar <= w || p2.scalar<=w && p3.scalar>=w){
        v[n] = getInterpolationPoint(p2, p3, w);
        n++;
    }
    if (p3.scalar >= w && p4.scalar <= w || p3.scalar<=w && p4.scalar>=w){
        v[n] = getInterpolationPoint(p3, p4, w);
        n++;
    }
    if (p4.scalar >= w && p1.scalar <= w || p4.scalar<=w && p1.scalar>=w){
        v[n] = getInterpolationPoint(p4, p1, w);
        n++;
    }
    if (n == 1)
        //drawLine(v[0].x, v[0].y, v[0].x, v[0].y, c);
    {
        drawLine(p1.x, p1.y, p2.x, p2.y, c);
        drawLine(p2.x, p2.y, p3.x, p3.y, c);
        drawLine(p3.x, p3.y, p4.x, p4.y, c);
        drawLine(p4.x, p4.y, p1.x, p1.y, c);
    }
    if (flag == 1){
        if (n == 2)
            drawLine(v[0].x, v[0].y, v[1].x, v[1].y, c);
        if (n == 4){
            drawLine(v[0].x, v[0].y, v[1].x, v[1].y, c);
            drawLine(v[2].x, v[2].y, v[3].x, v[3].y, c);
        }
    }
    else if(flag ==2){
        if (n == 2)
            drawLine_height(v[0].x, v[0].y, v[1].x, v[1].y, w, c);
        if (n == 4){
            drawLine_height(v[0].x, v[0].y, v[1].x, v[1].y, w, c);
            drawLine_height(v[2].x, v[2].y, v[3].x, v[3].y, w, c);
        }
    }
}

void display_contour(int flag)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    //glOrtho(-1.0, 1.0, -1.0, 1.0, 0.0, 40.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    //glOrtho(0, 512, 0, 512, 0.0, -40.0);

    glOrtho(0, 512, 0, 512, -600.0, 600.0);

    if (flag == 2){
        glScaled(0.5, 0.5, 0.5);
        glRotatef(50, 1.0, 0.0, 0.0);
        glRotatef(-20, 0.0, 1.0, 0.0);
        glTranslated(512 / 2, 500, 0.0);
    }

    glDisable(GL_TEXTURE_2D);
    glShadeModel(GL_FLAT);
    glEnable(GL_DEPTH_TEST);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDisable(GL_BLEND);
    glDisable(GL_LIGHTING);

    int colorsch;
    color c;
    if (flag == 1){
        cout << "select the color scheme" << endl;
        cin >> colorsch;
    }
    else{
        colorsch = 4;
    }

    //float countour_w = 100;
    //color c;
    //c.r = c.g = c.b = 0.0;
    float countour_w = intensity_min;
    while (countour_w < intensity_max){
        //float countour_w = 0.0;
        //while (countour_w < 255){
        float divergence = (countour_w - intensity_min) / intensity_range * 255;
        //float divergence = countour_w;

        switch (colorsch)
        {
        case 1:default:
            c.r = 0.0;
            c.g = divergence / 255.0;
            c.b = 0.0;
            break;
        case 2:
            c.r = (255 - divergence) / 255.0;
            c.g = 255.0 / 255.0;
            c.b = (255 - divergence) / 255.0;
            break;
        case 3:
            c.r = divergence / 255.0;
            c.g = 255.0 / 255.0;
            c.b = 0.0;
            break;
        case 4:
            c = getRainbowColor(countour_w);
            c.r /= 255.0;
            c.g /= 255.0;
            c.b /= 255.0;
            break;
        case 5:
            c = getHeatColor(countour_w);
            c.r /= 255.0;
            c.g /= 255.0;
            c.b /= 255.0;
            break;
        case 6:
            c = getBlue_WhiteColor(countour_w);
            c.r /= 255.0;
            c.g /= 255.0;
            c.b /= 255.0;
            break;
        }
    
        int cell_length = 4;
        int x, y;
        for (y = 0; y < 504; y = y + cell_length){
            for (x = 0; x < 504; x = x + cell_length){
                //int pixel_index = (y*win_width + x) * 3;
                pixel p1, p2, p3, p4;

                p1.x = x;
                p1.y = y;
                //p1.scalar = scalar_field[3 * (p1.y*win_width + p1.x)];
                p1.scalar = scalar_field2[int(3 * (p1.y*win_width + p1.x))];

                p2.x = x + cell_length;
                p2.y = y;
                //p2.scalar = scalar_field[3 * (p2.y*win_width + p2.x)];
                p2.scalar = scalar_field2[int(3 * (p2.y*win_width + p2.x))];

                p3.x = x + cell_length;
                p3.y = y + cell_length;
                //p3.scalar = scalar_field[3 * (p3.y*win_width + p3.x)];
                p3.scalar = scalar_field2[int(3 * (p3.y*win_width + p3.x))];

                p4.x = x;
                p4.y = y + cell_length;
                //p4.scalar = scalar_field[3 * (p4.y*win_width + p4.x)];
                p4.scalar = scalar_field2[int(3 * (p4.y*win_width + p4.x))];

                drawCountourLine(p1, p2, p3, p4, countour_w, c, flag);
            }
        }

        countour_w = countour_w + (intensity_range / 8.0);
    }
    //    //countour_w = countour_w + (255 / 8.0);
    glutSwapBuffers();
}

void display_height()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    //glOrtho(-1.0, 1.0, -1.0, 1.0, 0.0, 40.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    //gluLookAt(0.0, 0.0, 50.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);


    glOrtho(0, 512, 0, 512, -600.0, 600.0);
    //glFrustum(-512.0,512,-512.0,512,-100.0,100);
    //gluPerspective(90, 1.0, 0, 20);

    glScaled(0.5, 0.5, 0.5);
    //glTranslated(512 / 2, 300, 0.0);
    glRotatef(50, 1.0, 0.0, 0.0);
    glRotatef(-20, 0.0, 1.0, 0.0);

    glTranslated(512 / 2, 500, 0.0);

    glDisable(GL_TEXTURE_2D);
    glShadeModel(GL_FLAT);
    glEnable(GL_DEPTH_TEST);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDisable(GL_BLEND);
    glDisable(GL_LIGHTING);

    glLineWidth(1.0);
    glBegin(GL_LINES);
    for (int y = 0; y < win_height; y++){
        for (int x = 0; x < win_width; x++){
            int index = y*win_width + x;
            float intensity = scalar_field[index*3];

            color c = getRainbowColor(intensity);
            //color c = getHeatColor(intensity);
            c.r /= 255.0;
            c.g /= 255.0;
            c.b /= 255.0;

            glColor3f(c.r, c.g, c.b);
            glVertex3d(x, 0.0, -y);
            glVertex3d(x, intensity, -y);
        }
    }
    glEnd();

    glutSwapBuffers();
}

void display_height_with_contour()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    //glOrtho(-1.0, 1.0, -1.0, 1.0, 0.0, 40.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    //gluLookAt(0.0, 0.0, 50.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);


    glOrtho(0, 512, 0, 512, -600.0, 600.0);
    //glFrustum(-512.0,512,-512.0,512,-100.0,100);
    //gluPerspective(90, 1.0, 0, 20);

    glScaled(0.5, 0.5, 0.5);
    //glTranslated(512 / 2, 300, 0.0);
    glRotatef(50, 1.0, 0.0, 0.0);
    glRotatef(-20, 0.0, 1.0, 0.0);

    glTranslated(512 / 2, 500, 0.0);

    glDisable(GL_TEXTURE_2D);
    glShadeModel(GL_FLAT);
    glEnable(GL_DEPTH_TEST);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDisable(GL_BLEND);
    glDisable(GL_LIGHTING);

    glLineWidth(1.0);
    glBegin(GL_LINES);
    for (int y = 0; y < win_height; y++){
        for (int x = 0; x < win_width; x++){
            int index = y*win_width + x;
            float intensity = scalar_field[index*3];

            //color c = getRainbowColor(intensity);
            color c = getHeatColor(intensity);
            c.r /= 255.0;
            c.g /= 255.0;
            c.b /= 255.0;

            glColor3f(c.r, c.g, c.b);
            glVertex3d(x, 0.0, -y);
            glVertex3d(x, intensity, -y);
        }
    }
    glEnd();

    //int colorsch;
    color c;
    c.r = 0;
    c.g = 0;
    c.b = 0;
    //if (flag == 1){
    //    cout << "select the color scheme" << endl;
    //    cin >> colorsch;
    //}
    //else{
    //colorsch = 4;
    /*}*/

    //float countour_w = 100;
    //color c;
    //c.r = c.g = c.b = 0.0;
    float countour_w = intensity_min;
    while (countour_w < intensity_max){
        //float countour_w = 0.0;
        //while (countour_w < 255){
        //float divergence = (countour_w - intensity_min) / intensity_range * 255;
        //float divergence = countour_w;

        //switch (colorsch)
        //{
        //case 1:default:
        //    c.r = 0.0;
        //    c.g = divergence / 255.0;
        //    c.b = 0.0;
        //    break;
        //case 2:
        //    c.r = (255 - divergence) / 255.0;
        //    c.g = 255.0 / 255.0;
        //    c.b = (255 - divergence) / 255.0;
        //    break;
        //case 3:
        //    c.r = divergence / 255.0;
        //    c.g = 255.0 / 255.0;
        //    c.b = 0.0;
        //    break;
        //case 4:
        //    c = getRainbowColor(countour_w);
        //    c.r /= 255.0;
        //    c.g /= 255.0;
        //    c.b /= 255.0;
        //    break;
        //case 5:
        //    c = getHeatColor(countour_w);
        //    c.r /= 255.0;
        //    c.g /= 255.0;
        //    c.b /= 255.0;
        //    break;
        //case 6:
        //    c = getBlue_WhiteColor(countour_w);
        //    c.r /= 255.0;
        //    c.g /= 255.0;
        //    c.b /= 255.0;
        //    break;
        //}

        int cell_length = 8;
        int x, y;
        for (y = 0; y < 504; y = y + cell_length){
            for (x = 0; x < 504; x = x + cell_length){
                //int pixel_index = (y*win_width + x) * 3;
                pixel p1, p2, p3, p4;

                p1.x = x;
                p1.y = y;
                //p1.scalar = scalar_field[3 * (p1.y*win_width + p1.x)];
                p1.scalar = scalar_field2[int(3 * (p1.y*win_width + p1.x))];

                p2.x = x + cell_length;
                p2.y = y;
                //p2.scalar = scalar_field[3 * (p2.y*win_width + p2.x)];
                p2.scalar = scalar_field2[int(3 * (p2.y*win_width + p2.x))];

                p3.x = x + cell_length;
                p3.y = y + cell_length;
                //p3.scalar = scalar_field[3 * (p3.y*win_width + p3.x)];
                p3.scalar = scalar_field2[int(3 * (p3.y*win_width + p3.x))];

                p4.x = x;
                p4.y = y + cell_length;
                //p4.scalar = scalar_field[3 * (p4.y*win_width + p4.x)];
                p4.scalar = scalar_field2[int(3 * (p4.y*win_width + p4.x))];

                drawCountourLine(p1, p2, p3, p4, countour_w, c, 2);
            }
        }

        countour_w = countour_w + (intensity_range / 8.0);
    }

    glutSwapBuffers();
}

//Print out usage information.
void usage(char *progname)
{   
    printf("Input image is %s\n", progname);
	//fprintf (stderr, "usage: %s [flags] <in.ply >out.ply\n", progname);
	//fprintf (stderr, "       -n ppm file name\n");
}

//Initialize display windows
void init(void) {

    /*select clearing color */ 
	glClearColor (0.0, 0.0, 0.0, 0.0);  // background
	glClearDepth(1.0);
 
	glShadeModel (GL_FLAT);
	glPolygonMode(GL_FRONT, GL_FILL);

	/* initialize viewing values */
	glViewport(0, 0, 2.0, 2.0);

	glDisable(GL_DITHER);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	glPixelStorei(GL_PACK_ALIGNMENT,1);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	glClearColor (1.0, 1.0, 1.0, 1.0);  // background for rendering color coding and lighting
 
}

//the process functions
void initialize(){
    pixels_number = win_width*win_height;

    for (int i=0; i<3*win_width*win_height; i++)
    	pixels_output[i] = pixels_input[i];

    saveCount = 0;
}

void computeIntensityA(unsigned char *pixels_input, unsigned char *scalar_field){

    memset(pixels_output, 100, 3 * win_width*win_height*sizeof(unsigned char));

    for (int i = 0; i < 3*pixels_number; i=i+3){
        float intensity = (sqrt(pixels_input[i] * pixels_input[i] + pixels_input[i + 1] * pixels_input[i + 1] + pixels_input[i + 2] * pixels_input[i + 2]) / 3.0);
        scalar_field[i] = scalar_field[i + 1] = scalar_field[i + 2] = intensity;
        scalar_field2[i] = scalar_field2[i + 1] = scalar_field2[i + 2] = intensity;
        pixels_output[i] = pixels_output[i + 1] = pixels_output[i + 2] = intensity;
    }
}

void computeIntensityA2(unsigned char *pixels_input, unsigned char *scalar_field){

    memset(pixels_output, 100, 3 * win_width*win_height*sizeof(unsigned char));

    for (int i = 0; i < 3 * pixels_number; i = i + 3){
        float intensity = (sqrt(pixels_input[i] * pixels_input[i] + pixels_input[i + 1] * pixels_input[i + 1] + pixels_input[i + 2] * pixels_input[i + 2]) / 3.0);
        scalar_field[i] = scalar_field[i + 1] = scalar_field[i + 2] = intensity;
        //pixels_output[i] = pixels_output[i + 1] = pixels_output[i + 2] = intensity;
    }

    float range, max, min;
    for (int i = 0; i < 3 * pixels_number; i = i + 3){
        float intensity = scalar_field[i];
        if (i == 0){
            max = min = intensity;
        }
        else{
            max = intensity > max ? intensity : max;
            min = intensity < min ? intensity : min;
        }

    }
    range = max - min;

    for (int i = 0; i < 3 * pixels_number; i = i + 3){
        float intensity = (sqrt(pixels_input[i] * pixels_input[i] + pixels_input[i + 1] * pixels_input[i + 1] + pixels_input[i + 2] * pixels_input[i + 2]) / 3.0);
        scalar_field[i] = scalar_field[i + 1] = scalar_field[i + 2] = (intensity-min)/range*255;

        //pixels_output[i] = pixels_output[i + 1] = pixels_output[i + 2] = intensity;
    }
}

void computeIntensityB(unsigned char *pixels_input, unsigned char *scalar_field){

    memset(pixels_output, 100, 3 * win_width*win_height*sizeof(unsigned char));

    for (int i = 0; i < 3 * pixels_number; i = i + 3){
        float intensity = pixels_input[i] * 0.30 + pixels_input[i + 1] * 0.59 + pixels_input[i + 2] * 0.11;
        scalar_field[i] = scalar_field[i + 1] = scalar_field[i + 2] = intensity;
        scalar_field2[i] = scalar_field2[i + 1] = scalar_field2[i + 2] = intensity;
        pixels_output[i] = pixels_output[i + 1] = pixels_output[i + 2] = intensity;
    }
}

void computeChannelRed(unsigned char *pixels_input, unsigned char *scalar_field){

    memset(pixels_output, 100, 3 * win_width*win_height*sizeof(unsigned char));

    for (int i = 0; i < 3 * pixels_number; i = i + 3){
        //float intensity = pixels_input[i] * 0.30 + pixels_input[i + 1] * 0.59 + pixels_input[i + 2] * 0.11;
        scalar_field[i] = scalar_field[i + 1] = scalar_field[i + 2] = pixels_input[i];
        pixels_output[i] = pixels_input[i];
        pixels_output[i + 1] = pixels_output[i + 2]=0.0;
        scalar_field2[i] = scalar_field2[i + 1] = scalar_field2[i + 2] = pixels_input[i];
    }
}

void computeChannelGreen(unsigned char *pixels_input, unsigned char *scalar_field){

    memset(pixels_output, 100, 3 * win_width*win_height*sizeof(unsigned char));

    for (int i = 0; i < 3 * pixels_number; i = i + 3){
        //float intensity = pixels_input[i] * 0.30 + pixels_input[i + 1] * 0.59 + pixels_input[i + 2] * 0.11;
        scalar_field[i] = scalar_field[i + 1] = scalar_field[i + 2] = pixels_input[i+1];
        pixels_output[i+1] = pixels_input[i + 1];
        pixels_output[i] = pixels_output[i + 2] = 0.0;
        scalar_field2[i] = scalar_field2[i + 1] = scalar_field2[i + 2] = pixels_input[i + 1];
    }
}

void computeChannelBlue(unsigned char *pixels_input, unsigned char *scalar_field){

    memset(pixels_output, 100, 3 * win_width*win_height*sizeof(unsigned char));

    for (int i = 0; i < 3 * pixels_number; i = i + 3){
        //float intensity = pixels_input[i] * 0.30 + pixels_input[i + 1] * 0.59 + pixels_input[i + 2] * 0.11;
        scalar_field[i] = scalar_field[i + 1] = scalar_field[i + 2] = pixels_input[i+2];
        pixels_output[i + 2] = pixels_input[i + 2];
        pixels_output[i] = pixels_output[i + 1] = 0.0;
        scalar_field2[i] = scalar_field2[i + 1] = scalar_field2[i + 2] = pixels_input[i + 2];
    }
}

void calculateIntensityRange(unsigned char *p){

    for (int i = 0; i < 3 * pixels_number; i = i + 3){
        float intensity = p[i];
        if (i == 0){
            intensity_max = intensity_min = intensity;
        }
        else{
            intensity_max = intensity > intensity_max ? intensity : intensity_max;
            intensity_min = intensity < intensity_min ? intensity : intensity_min;
        }

    }
    intensity_range = intensity_max - intensity_min;
}

void ShowScalar1(unsigned char *scalar_field){

    for (int i = 0; i < 3 * pixels_number; i = i + 3){
        float intensity = scalar_field[i];
        float divergence = (intensity - intensity_min) / intensity_range * 255;

        pixels_output[i] = 0.0;
        pixels_output[i + 1] = divergence;
        pixels_output[i + 2] = 0.0;
    }
}

void ShowScalar2(unsigned char *scalar_field){

    for (int i = 0; i < 3 * pixels_number; i = i + 3){
        float intensity = scalar_field[i];
        float divergence = (intensity - intensity_min) / intensity_range * 255;

        pixels_output[i] = 255- divergence;
        pixels_output[i + 1] = 255;
        pixels_output[i + 2] = 255- divergence;
    }
}

void ShowScalar3(unsigned char *scalar_field){

    //computeIntensityA(pixels_input, pixels_output);
    //calculateIntensityRange(pixels_output);

    for (int i = 0; i < 3 * pixels_number; i = i + 3){
        float intensity = scalar_field[i];
        float divergence = (intensity - intensity_min) / intensity_range * 255;

        pixels_output[i] = divergence;
        pixels_output[i + 1] = 255;
        pixels_output[i + 2] = 0.0;
    }
}

color getRainbowColor(float intensity){

    color c;
    float bound[5];
    float range = 255.0 / 4.0;
    for (int i = 0; i < 5; i++){
        bound[i] = range*i;
    }
    intensity = (intensity - intensity_min) / intensity_range * 255;

    if (intensity < bound[1] && intensity >= bound[0]){
        float divergence = (intensity - bound[0]) / range*255.0;
        c.r = 0.0;
        c.g = divergence;
        c.b = 255;
    }
    else if (intensity < bound[2] && intensity >= bound[1]){
        float divergence = (intensity - bound[1]) / range*255.0;
        c.r = 0.0;
        c.g= 255;
        c.b = 255 - divergence;
    }
    else if (intensity < bound[3] && intensity >= bound[2]){
        float divergence = (intensity - bound[2]) / range*255.0;
        c.r = divergence;
        c.g = 255;
        c.b = 0.0;
    }
    else if (intensity <= bound[4] && intensity >= bound[3]){
        float divergence = (intensity - bound[3]) / range*255.0;
        c.r = 255;
        c.g = 255 - divergence;
        c.b = 0.0;
    }
    return c;

}

//rainbow
void ShowScalar4(unsigned char *scalar_field){

    for (int i = 0; i < 3 * pixels_number; i = i + 3){
        float intensity = scalar_field[i];
        
        color c = getRainbowColor(intensity);
        pixels_output[i] = c.r;
        pixels_output[i + 1] = c.g;
        pixels_output[i + 2] = c.b;

    }
}

color getHeatColor(float intensity){

    color c;
    float bound[4];
    float range = 255.0 / 3;
    for (int i = 0; i < 4; i++){
        bound[i] = range*i;
    }
    intensity = (intensity - intensity_min) / intensity_range * 255;

    if (intensity < bound[1] && intensity >= bound[0]){
        float divergence = (intensity - bound[0]) / range*255.0;
        c.r = divergence;
        c.g = 0.0;
        c.b = 0.0;
    }
    else if (intensity < bound[2] && intensity >= bound[1]){
        float divergence = (intensity - bound[1]) / range*255.0;
        c.r = 255;
        c.g = divergence;
        c.b = 0.0;
    }
    else if (intensity <= bound[3] && intensity >= bound[2]){
        float divergence = (intensity - bound[2]) / range*255.0;
        c.r = 255;
        c.g = 255;
        c.b = divergence;
    }
    return c;

}

//heat
void ShowScalar5(unsigned char *scalar_field){

    for (int i = 0; i < 3 * pixels_number; i = i + 3){
        float intensity = scalar_field[i];

        color c = getHeatColor(intensity);
        pixels_output[i] = c.r;
        pixels_output[i + 1] = c.g;
        pixels_output[i + 2] = c.b;

    }
}

//blue-white-red
color getBlue_WhiteColor(float intensity){

    color c;
    float bound[3];
    float range = 255.0 / 2.0;
    for (int i = 0; i < 3; i++){
        bound[i] = range*i;
    }
    intensity = (intensity - intensity_min) / intensity_range * 255;

    if (intensity < bound[1] && intensity >= bound[0]){
        float divergence = (intensity - bound[0]) / range*255.0;
        c.r = divergence;
        c.g = divergence;
        c.b = 255;
    }
    else if (intensity <= bound[2] && intensity >= bound[1]){
        float divergence = (intensity - bound[1]) / range*255.0;
        c.r = 255;
        c.g = 255 - divergence;
        c.b = 255 - divergence;
    }
    return c;

}

void ShowScalar6(unsigned char *scalar_field){

    float bound[3];
    float range = 255.0 / 2.0;
    for (int i = 0; i < 3; i++){
        bound[i] = range*i;
    }

    //computeIntensityA(pixels_input, pixels_output);
    //calculateIntensityRange(pixels_output);

    for (int i = 0; i < 3 * pixels_number; i = i + 3){
        float intensity = scalar_field[i];
        color c = getBlue_WhiteColor(intensity);
        pixels_output[i] = c.r;
        pixels_output[i + 1] = c.g;
        pixels_output[i + 2] = c.b;
        //intensity = (intensity - intensity_min) / intensity_range * 255;
        //if (intensity < bound[1] && intensity >= bound[0]){
        //    float divergence = (intensity - bound[0]) / range*255.0;
        //    pixels_output[i] = divergence;
        //    pixels_output[i + 1] = divergence;
        //    pixels_output[i + 2] = 255;
        //}
        //else if (intensity <= bound[2] && intensity >= bound[1]){
        //    float divergence = (intensity - bound[1]) / range*255.0;
        //    pixels_output[i] = 255;
        //    pixels_output[i + 1] = 255 - divergence;
        //    pixels_output[i + 2] = 255 - divergence;
        //}

    }
}

//rainbow with contour
void ShowScalar7(unsigned char *scalar_field){

    float bound[5];
    float range = 255.0 / 4.0;
    for (int i = 0; i < 5; i++){
        bound[i] = range*i;
    }

    //computeIntensityA(pixels_input, pixels_output);
    //calculateIntensityRange(pixels_output);

    for (int y = 0; y < win_height; y++){
        for (int x = 0; x < win_width; x ++){
            int index = y*win_width + x;
            int i = index * 3;
            float intensity = scalar_field[i];
            intensity = (intensity - intensity_min) / intensity_range * 255;

            if (intensity < bound[1] && intensity > bound[0]){
                float divergence = (intensity - bound[0]) / range*255.0;
                pixels_output[i] = 0.0;
                pixels_output[i + 1] = divergence;
                pixels_output[i + 2] = 255;
            }
            else if (intensity < bound[2] && intensity > bound[1]){
                float divergence = (intensity - bound[1]) / range*255.0;
                pixels_output[i] = 0.0;
                pixels_output[i + 1] = 255;
                pixels_output[i + 2] = 255 - divergence;
            }
            else if (intensity < bound[3] && intensity > bound[2]){
                float divergence = (intensity - bound[2]) / range*255.0;
                pixels_output[i] = divergence;
                pixels_output[i + 1] = 255;
                pixels_output[i + 2] = 0.0;
            }
            else if (intensity <= bound[4] && intensity > bound[3]){
                float divergence = (intensity - bound[3]) / range*255.0;
                pixels_output[i] = 255;
                pixels_output[i + 1] = 255 - divergence;
                pixels_output[i + 2] = 0.0;
            }
            float e = 0.5;
            if (intensity < bound[0] + e && intensity > bound[0] - e ||  intensity < bound[1]+e && intensity > bound[1]-e || intensity < bound[2]+e && intensity > bound[2] - e|| intensity > bound[3]-e && intensity < bound[3]+e || intensity> bound[4]-e && intensity < bound[4]+e){
                
                pixels_output[index * 3] = pixels_output[index * 3 + 1] = pixels_output[index*3 + 2] = 255;
                if (y>0)//up
                    pixels_output[(index - win_width) * 3] = pixels_output[(index - win_width) * 3 + 1] = pixels_output[(index - win_width) * 3 + 2] = 255;
                if (y<511)//down
                    pixels_output[(index + win_width) * 3] = pixels_output[(index + win_width) * 3 + 1] = pixels_output[(index + win_width) * 3 + 2] = 255;
                if (x>0)//left
                    pixels_output[(index - 1) * 3] = pixels_output[(index - 1) * 3 + 1] = pixels_output[(index - 1) * 3 + 2] = 255;
                if (x<511)//right
                    pixels_output[(index + 1) * 3] = pixels_output[(index + 1) * 3 + 1] = pixels_output[(index + 1) * 3 + 2] = 255;

                if (x>0 && y>0)//left-up
                    pixels_output[(index - win_width - 1) * 3] = pixels_output[(index - win_width - 1) * 3 + 1] = pixels_output[(index - win_width - 1) * 3 + 2] = 255;
                if (x<511 && y>0)//right-up
                    pixels_output[(index - win_width + 1) * 3] = pixels_output[(index - win_width + 1) * 3 + 1] = pixels_output[(index - win_width + 1) * 3 + 2] = 255;

                if (x<511 && y<511)//right-down
                    pixels_output[(index + win_width + 1) * 3] = pixels_output[(index + win_width + 1) * 3 + 1] = pixels_output[(index + win_width + 1) * 3 + 2] = 255;

                if (x>0 && y<511)//left-down
                    pixels_output[(index + win_width - 1) * 3] = pixels_output[(index + win_width - 1) * 3 + 1] = pixels_output[(index + win_width - 1) * 3 + 2] = 255;


            }

        }
    }
}

//Process a keyboard action.  In particular, exit the program when an "escape" is pressed in the window.
void keyboard_input_window(unsigned char key, int x, int y) {
  /* set escape key to exit */
  switch (key) {
    case 27:
			finalize_system();
      exit(0);
      break;
	}
}

void keyboard_output_window(unsigned char key, int x, int y) {
  /* set escape key to exit */
  switch (key) {
    case 'a':
        //Computed the intensity of the image with using Average method
        computeIntensityA(pixels_input, pixels_output);
        display_image_output_window();
        break;
    case 'b':
        //Computed the intensity of the image with using Colorimetric method
        computeIntensityB(pixels_input, pixels_output);
        display_image_output_window();
        break;
    case 'c':
        //Compute the Red Channel
        computeChannelRed(pixels_input, pixels_output);
        display_image_output_window();
        break;
    case 'd':
        //Compute the Blue Channel
        computeChannelBlue(pixels_input, pixels_output);
        display_image_output_window();
        break;
    case 'e':
        //Compute the Green Channel
        computeChannelGreen(pixels_input, pixels_output);
        display_image_output_window();
        break;
    case 'f':
        //computeIntensityA2(pixels_input, scalar_field2);
        //calculateIntensityRange(scalar_field2);
        //computeIntensityA(pixels_input, scalar_field);
        calculateIntensityRange(scalar_field);
        display_contour(1);
        break;
    case 'g':
        calculateIntensityRange(scalar_field);
        display_height();
        break;
    case 'h':
        calculateIntensityRange(scalar_field);
        display_height_with_contour();
        break;
    case '1':
        //Compute the Green Channel
        calculateIntensityRange(scalar_field);
        ShowScalar1(scalar_field);
        display_image_output_window();
        break;
    case '2':
        //Compute the Green Channel
        calculateIntensityRange(scalar_field);
        ShowScalar2(scalar_field);
        display_image_output_window();
        break;
    case '3':
        //Compute the Green Channel
        calculateIntensityRange(scalar_field);
        ShowScalar3(scalar_field);
        display_image_output_window();
        break;
    case '4':
        //Compute the Green Channel
        calculateIntensityRange(scalar_field);
        ShowScalar4(scalar_field);
        display_image_output_window();
        break;
    case '5':
        //Compute the Green Channel
        calculateIntensityRange(scalar_field);
        ShowScalar5(scalar_field);
        display_image_output_window();
        break;
    case '6':
        //Compute the Green Channel
        calculateIntensityRange(scalar_field);
        ShowScalar6(scalar_field);
        display_image_output_window();
        break;
    case '7':
        //Compute the Green Channel
        calculateIntensityRange(scalar_field);
        ShowScalar7(scalar_field);
        display_image_output_window();
        break;
    case's':
        saveCount++;
        char str[100];
        sprintf(str,"../ppm/result%d.ppm",saveCount);
        SSS_SavePPM(win_width, win_height, pixels_output, str);

    case 27:
		finalize_system();
        exit(0);
        break;
	}
}

//Allocate resources.
void initialize_system()
{
	int i;

	if ((pixels_input = (unsigned char *)malloc(MAX_TEXTURE_SIZE*MAX_TEXTURE_SIZE*3*sizeof(unsigned char))) == NULL) {
		return;
	}
	if ((pixels_output = (unsigned char *)malloc(MAX_TEXTURE_SIZE*MAX_TEXTURE_SIZE*3*sizeof(unsigned char))) == NULL) {
		return;
	}
    if ((scalar_field = (unsigned char *)malloc(MAX_TEXTURE_SIZE*MAX_TEXTURE_SIZE * 3 * sizeof(unsigned char))) == NULL) {
        return;
    }
    if ((scalar_field2 = (float *)malloc(MAX_TEXTURE_SIZE*MAX_TEXTURE_SIZE * 3 * sizeof(float))) == NULL) {
        return;
    }
	for (i=0; i<3*MAX_TEXTURE_SIZE*MAX_TEXTURE_SIZE; i++) {
        pixels_output[i] = pixels_input[i] = scalar_field[i] = scalar_field2[i] = 0.0;
	}
}

//Release resources.
void finalize_system()
{
	free(pixels_input);
	free(pixels_output);
}

void selectImage(){
    cout << "1: lion" << endl << "2: duck" << endl << "3: Mona Lisa" << endl << "4: rose" << endl << "5: cat" << endl;
    cout << "6: blurred lion" << endl << "7: blurred duck" << endl << "8: blurred Mona Lisa" << endl << "9: blurred rose" << endl << "0: blurred cat" << endl;
    int n;
    cin >> n;
    switch (n)
    {
    case 1:
        progname = "../ppm/lion.ppm";
        flag_ppm_blurred = 0;
        break;
    case 2:
        progname = "../ppm/duck.ppm";
        flag_ppm_blurred = 0;
        break;
    case 3:
        progname = "../ppm/mona_lisa.ppm";
        flag_ppm_blurred = 0;
        break;
    case 4:
        progname = "../ppm/rose.ppm";
        flag_ppm_blurred = 0;
        break;
    case 5:
        progname = "../ppm/cat.ppm";
        flag_ppm_blurred = 0;
        break;
    case 6:
        progname = "../ppm/lion_blurred.ppm";
        flag_ppm_blurred = 1;
        break;
    case 7:
        progname = "../ppm/duck_blurred.ppm";
        flag_ppm_blurred = 1;
        break;
    case 8:
        progname = "../ppm/mona_lisa_blurred.ppm";
        flag_ppm_blurred = 1;
        break;
    case 9:
        progname = "../ppm/rose_blurred.ppm";
        flag_ppm_blurred = 1;
        break;
    case 0:
        progname = "../ppm/cat_blurred.ppm";
        flag_ppm_blurred = 1;
        break;
    default:
        progname = "../ppm/lion.ppm";
        flag_ppm_blurred = 1;
        break;
    }
    usage(progname);
}

void showNotes(int n){
    for (int i = 0; i < n; i++){
        //std::string n = "test";
        cout << notes[i] << endl;
    }
}

//Main program
int main(int argc, char *argv[])
{
    char *s;
   
    int w, h, i;

    int n;
    cout << "please select the image you want to load" << endl;
    selectImage();

    strcpy(texture_name, progname);
    //usage(progname);
	initialize_system();

    if (flag_ppm_blurred == 0)
	    SSS_ReadPPM(&w, &h,pixels_input, texture_name);
    else
        SSS_ReadPPM_blurred(&w, &h, pixels_input, texture_name);
    initialize();

	int mmmm = 1;
	char *ll = new char[20];
	glutInit(&mmmm, &ll);

    //showNotes(2);
    int ic;
    cout << "please select the scalar function" << endl << "1: average intensity" << endl << "2: colormetric" << endl << "3: Red Channel" << endl << "4: Green Channel" << endl << "5: blue Channel" << endl;
    cin >> ic;

    switch (ic)
    {
    case 1:
        computeIntensityA(pixels_input, scalar_field);
        break;
    case 2:
        computeIntensityB(pixels_input, scalar_field);
        break;
    case 3:
        computeChannelRed(pixels_input, scalar_field);
        break;
    case 4:
        computeChannelGreen(pixels_input, scalar_field);
        break;
    case 5:
        computeChannelBlue(pixels_input, scalar_field);
        break;
    default:
        computeIntensityA(pixels_input, scalar_field);
        break;
    }

	glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB );
    //create the input window
	glutInitWindowSize (win_width, win_height); 
	glutInitWindowPosition (100, 100);
	glutCreateWindow ("Input Image");
	init ();
	glutKeyboardFunc (keyboard_input_window);
	glutDisplayFunc(display_image_input_window); 

    //create the output window
	glutInitWindowSize (win_width, win_height); 
	glutInitWindowPosition (200, 100);
	glutCreateWindow ("Output Image");
	init ();
	glutKeyboardFunc (keyboard_output_window);
	glutDisplayFunc(display_image_output_window); 

	glutMainLoop(); 

	finalize_system();

  return 0;    /* ANSI C requires main to return int. */
}
