#include <stdio.h>
// yes, I know stdio.h is not good C++, but I like the *printf( )
#include <stdlib.h>
#include <ctype.h>

#define _USE_MATH_DEFINES
#include <math.h>

#ifdef WIN32
#include <windows.h>
#pragma warning(disable:4996)
#endif

#include <GL/gl.h>
#include <GL/glu.h>
#include "glut.h"
#include "glui.h"


// title of these windows:

const char *WINDOWTITLE = { "OpenGL / GLUT / GLUI final project" };
const char *GLUITITLE = { "User Interface Window" };

#define TimeStep 0.02
#define NX 300
#define NY 300
#define XMIN    -1
#define XMAX    1
#define YMIN    -1
#define YMAX    1

#define MAX_ITERATIONS 50

// what the glui package defines as true and false:

const int GLUITRUE = { true };
const int GLUIFALSE = { false };
COpenGL::COpenGL(int x, int y, CMyDlgOpenGLDlg *pParent)
{
	win_width = x;
	win_height = y;
	this->parent = pParent;
	initialize();

}

COpenGL::~COpenGL()
{
	finalize();
	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(hglrc);
	::ReleaseDC(m_hWnd, hdc);

}


// the escape key:

#define ESCAPE      0x1b


// initial window size:

const int INIT_WINDOW_SIZE = { 600 };


// size of the box:

const float BOXSIZE = { 2.f };



// multiplication factors for input interaction:
//  (these are known from previous experience)

const float ANGFACT = { 1. };
const float SCLFACT = { 0.005f };


// able to use the left mouse for either rotation or scaling,
// in case have only a 2-button mouse:

unsigned char *BmpToTexture(char *, int *, int *);
int	ReadInt(FILE *);
short	ReadShort(FILE *);

struct vector
{
	float x, y;
	
	float vx, vy;
	float noise;

};
struct vector Vect[NX][NY];

struct bmfh
{
	short bfType;
	int bfSize;
	short bfReserved1;
	short bfReserved2;
	int bfOffBits;
} FileHeader;

struct bmih
{
	int biSize;
	int biWidth;
	int biHeight;
	short biPlanes;
	short biBitCount;
	int biCompression;
	int biSizeImage;
	int biXPelsPerMeter;
	int biYPelsPerMeter;
	int biClrUsed;
	int biClrImportant;
} InfoHeader;

const int birgb = { 0 };

enum LeftButton
{
	ROTATE,
	SCALE
};

inline float SQR(float x)
{
	return x * x;
}
// minimum allowable scale factor:

const float MINSCALE = { 0.05f };


// active mouse buttons (or them together):

const int LEFT = { 4 };
const int MIDDLE = { 2 };
const int RIGHT = { 1 };


// which projection:

enum Projections
{
	ORTHO,
	PERSP
};


// which button:

enum ButtonVals
{
	RESET,
	QUIT
};


// window background color (rgba):

const float BACKCOLOR[] = { 0., 0., 0., 0. };






// fog parameters:

const GLfloat FOGCOLOR[4] = { .0, .0, .0, 1. };
const GLenum  FOGMODE = { GL_LINEAR };
const GLfloat FOGDENSITY = { 0.30f };
const GLfloat FOGSTART = { 1.5 };
const GLfloat FOGEND = { 4. };


// non-constant global variables:

int ActiveButton;           // current button that is down
GLuint  AxesList;           // list to hold the axes
int AxesOn;                 // != 0 means to draw the axes
int GrayOn;                 // !=0 means to draw the color graph
int PlaneOn;
int PlaneXYOn;
int PlaneXZOn;
int PlaneYZOn;
int times = 0;
int flag = 0;
//project 6
//Glui->add_checkbox("Bounding Box", &BoxOn);
//Glui->add_checkbox("Vector Cloud", &VectorcloudOn);
//Glui->add_checkbox("Streamlines", &StreamlinesOn);
int BoxOn = 1;
int VectorcloudOn;
int StreamlinesOn;

int DebugOn;                // != 0 means to print debugging info
int DepthCueOn;             // != 0 means to use intensity depth cueing
GLUI *  Glui;               // instance of glui window
int GluiWindow;             // the glut id for the glui window
int LeftButton;             // either ROTATE or SCALE
GLuint  BoxList;            // object display list
GLuint PointList;
int MainWindow;             // window id for main graphics window
GLfloat RotMatrix[4][4];    // set by glui rotation widget
float   Scale, Scale2;      // scaling factors
int WhichColor;             // index into Colors[ ]
int WhichProjection;        // ORTHO or PERSP

int WhichCuttingPlane;
int WireSurfaceOn;

//vector
int WhichKindVector;

int Xmouse, Ymouse;         // mouse values
float   Xrot, Yrot;         // rotation angles in degrees


GLUI_HSlider *      TempSlider;
GLUI_HSlider *      XSlider;
GLUI_HSlider *      YSlider;
GLUI_HSlider *      ZSlider;
GLUI_HSlider *      RadSlider;
GLUI_HSlider *      GradSlider;
GLUI_HSlider *		VectorCloudScale;
GLUI_HSlider *		FlowSpeed;
GLUI_HSlider *		CurlRange;
GLUI_StaticText *   XLabel;
GLUI_StaticText *   YLabel;
GLUI_StaticText *   ZLabel;
GLUI_StaticText *   TempLabel;
GLUI_StaticText *   RadLabel;
GLUI_StaticText *   GradLabel;
GLUI_StaticText *	VectorLabel;
GLUI_StaticText *	FlowLabel;
GLUI_StaticText *	CurlLabel;
// function prototypes:

void    Animate();

void    Display();
void    DoRasterString(float, float, float, char *);
void    DoStrokeString(float, float, float, float, char *);
float   ElapsedSeconds();
void    InitGlui();
void    InitGraphics();
void    InitLists();

void    MouseButton(int, int, int, int);
void    MouseMotion(int, int);
void    Reset();
void    Resize(int, int);
void    Visibility(int);

void    Arrow(float[3], float[3]);
void    Cross(float[3], float[3], float[3]);
float   Dot(float[3], float[3]);
float   Unit(float[3], float[3]);
void    Axes(float);
void    HsvRgb(float[3], float[3]);

//set slider and buttons

void    Sliders(int);
void	NewVector(int);

void	Vector(float, float, float *, float *);
void	Streamline(float, float);
void	Advect(float *, float *);
void	LittleBox(float, float, float);
void	Colormapping(float)

void interField(double **, double **, int **)
void NewVector(int id)
{

}

// main program:

int
main(int argc, char *argv[])
{
	// turn on the glut package:
	// (do this before checking argc and argv since it might
	// pull some command line arguments out)

	glutInit(&argc, argv);


	// setup all the graphics stuff:

	InitGraphics();


	// create the display structures that will not change:

	InitLists();


	// init all the global variables used by Display( ):
	// this will also post a redisplay
	// it is important to call this before InitGlui( )
	// so that the variables that glui will control are correct
	// when each glui widget is created

	Reset();


	// setup all the user interface stuff:

	InitGlui();


	// draw the scene once and wait for some interaction:
	// (this will never return)

	glutMainLoop();


	// this is here to make the compiler happy:

	return 0;
}


// this is where one would put code that is to be called
// everytime the glut main loop has nothing to do
//
// this is typically where animation parameters are set
//
// do not call Display( ) from here -- let glutMainLoop( ) do it









// draw the complete scene:

void
Display()
{

	if (DebugOn != 0)
	{
		fprintf(stderr, "Display\n");
	}



	// set which window we want to do the graphics into:

	glutSetWindow(MainWindow);


	// erase the background:

	glDrawBuffer(GL_BACK);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//glEnable(GL_DEPTH_TEST);


	// specify shading to be flat:

	glShadeModel(GL_FLAT);


	// set the viewport to a square centered in the window:

	GLsizei vx = glutGet(GLUT_WINDOW_WIDTH);
	GLsizei vy = glutGet(GLUT_WINDOW_HEIGHT);
	GLsizei v = vx < vy ? vx : vy;          // minimum dimension
	GLint xl = (vx - v) / 2;
	GLint yb = (vy - v) / 2;
	glViewport(xl, yb, v, v);


	// set the viewing volume:
	// remember that the Z clipping  values are actually
	// given as DISTANCES IN FRONT OF THE EYE
	// USE gluOrtho2D( ) IF YOU ARE DOING 2D !

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();


	//// place the objects into the scene:

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();




	// set the eye position, look-at position, and up-vector:
	// IF DOING 2D, REMOVE THIS -- OTHERWISE ALL YOUR 2D WILL DISAPPEAR !

	//gluLookAt(0., 0., 3., 0., 0., 0., 0., 1., 0.);


	// translate the objects in the scene:
	// note the minus sign on the z value
	// this is to make the appearance of the glui z translate
	// widget more intuitively match the translate behavior
	// DO NOT TRANSLATE IN Z IF YOU ARE DOING 2D !

	glClearColor(1.0, 1.0, 1.0, 1.0);  // background for rendering color coding and lighting

	glDisable(GL_BLEND);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	glEnable(GL_TEXTURE_2D);



	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();

	glViewport(0, 0, win_width, win_height);

	glOrtho(-1.0, 1.0, -1.0, 1.0, 0.0, 40.0);
	//DrawColorBox();

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, win_width, win_height, 0, GL_RGB, GL_UNSIGNED_BYTE, pixelsBuffer);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE); 

	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0);	glVertex3d(-1.0, -1.0, 0.0);
	glTexCoord2f(0.0, 1.0);	glVertex3d(-1.0, 1.0, 0.0);
	glTexCoord2f(1.0, 1.0);	glVertex3d(1.0, 1.0, 0.0);
	glTexCoord2f(1.0, 0.0);	glVertex3d(1.0, -1.0, 0.0);
	glEnd();

	//glMatrixMode(GL_MODELVIEW);
	//glPopMatrix();
	//glMatrixMode(GL_PROJECTION);
	//glPopMatrix();
	//glFlush();

	SwapBuffers(hdc);
	//glFinish();

	if (flag_firtTime){
		makeNoisePatterns();
		flag_firtTime = false;
		Invalidate();


	// uniformly scale the scene:
	 
	

	// set the fog parameters:
	// DON'T NEED THIS IF DOING 2D !

	

	glDisable(GL_DEPTH_TEST);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(-1., 1., -1., 1.);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glColor3f(1., 1., 1.);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, whn);
	glEnable(GL_TEXTURE_2D);
	//draw white noise;
	//glColor3f(1., 0., 1.);

	//for (int i = 0; i < 100; i++)
	//{
	//	for (int j = 0; j < 100; j++)
	//	{
	//		glBegin(GL_QUADS);
	//		glTexCoord2f((double)i / 100., (double)j / 100.);
	//		glVertex2f(-1 + 0.02 * (double)i, -1. + 0.02 * (double)j);
	//		glTexCoord2f((double)i / 100. + 0.01, (double)j / 100.);
	//		glVertex2f(-1 + 0.02 * ((double)i + 1), -1. + 0.02 * (double)j);
	//		glTexCoord2f((double)i / 100. + 0.01, (double)j / 100. + 0.01);
	//		glVertex2f(-1 + 0.02 * ((double)i + 1), -1. + 0.02 * ((double)j + 1));
	//		glTexCoord2f((double)i / 100., (double)j / 100. + 0.01);
	//		glVertex2f(-1 + 0.02 * (double)i, -1. + 0.02 * ((double)j + 1));
	//		glEnd();
	//	}
	//}
	glDisable(GL_TEXTURE_2D);

	float noise;
	glBegin(GL_QUADS);
	for (int i = 0; i < NX; i++)
	{
		for (int j = 0; j < NY; j++)
		{
			Vect[i][j].noise = (double)rand()/(double)RAND_MAX;
			glColor3f(Vect[i][j].noise, Vect[i][j].noise, Vect[i][j].noise);
			glVertex2f(-1 + (double)i / ((double)NX / 2.), -1 + (double)j / ((double)NY / 2.));
			glVertex2f(-1 + (double)i / ((double)NX / 2.) + 0.1, -1 + (double)j / ((double)NY / 2.));
			glVertex2f(-1 + (double)i / ((double)NX / 2.) + 0.1, -1 + (double)j / ((double)NY / 2.) + 0.1);
			glVertex2f(-1 + (double)i / ((double)NX / 2.), -1 + (double)j / ((double)NY / 2.) + 0.1);
		}
	}
	glEnd();

	
	for (int i = 0; i < NX; i++)
	{
		for (int j = 0; j < NY; j++)
		{
			if ((i % 2 == 0) && (j % 2 == 0))
			{
				break;
			}
			Streamline(Vect[i][j].x, Vect[i][j].y);
		}
	}

	//glShadeModel(GL_FLAT);
	//glBegin(GL_LINES);
	//glColor3f(1., 0., 0.); 
	//glVertex2f(-1., -1.);
	//
	//glColor3f(0., 1., 0.);
	//glVertex2f(1., 1.);

	//glEnd();
	// swap the double-buffered framebuffers:
	glDisable(GL_TEXTURE_2D);

	// be sure the graphics buffer has been sent:
	// note: be sure to use glFlush( ) here, not glFinish( ) !
	glutSwapBuffers();
	glFlush();

	for (int y = 0; y < win_height; y++){
		for (int x = 0; x < win_width; x++){

			int index = y*win_width + x;

			Point p(x + 0.5, y + 0.5);
			    double length_total = 0.0;
    double length_segment;
    double length_top, length_bottom, length_left, length_right;

    Point p = start;

    while (length_total<maxLength){
        icVector2 v = getVector(int(p.x), int(p.y));
        normalize(v);
    
        double top = (p.y == int(p.y) ? p.y + 1 : ceil(p.y));
        double bottom = (p.y == int(p.y) ? p.y - 1 : floor(p.y));
        double right = (p.x == int(p.x) ? p.x + 1 : ceil(p.x));
        double left = (p.x == int(p.x)? p.x -1: floor(p.x));
        
        if (v.x == 0 && v.y == 0)
            break;

        length_top = (top - p.y) / v.y;
        length_bottom = (bottom - p.y) / v.y;
        length_left = (left - p.x) / v.x;
        length_right = (right - p.x) / v.x;

        length_segment = getMinimumPositive(length_top, length_bottom, length_left, length_right);
    
        Point p_next = Point(p.x + length_segment*v.x, p.y + length_segment*v.y);
        if (isLegal(p_next)){
            streamline.push_back(p_next);
            length_total += length_segment;
            p = p_next;
        }
        else{
            break;
        }
    }


	//LIC visualization
	double length_total = 0.0;
	double length_segment;
	double length_top, length_bottom, length_left, length_right;

	Point p = start;

	while (length_total<maxLength){
		icVector2 v = getVector(int(p.x), int(p.y));
		normalize(v);

		double top = (p.y == int(p.y) ? p.y + 1 : ceil(p.y));
		double bottom = (p.y == int(p.y) ? p.y - 1 : floor(p.y));
		double right = (p.x == int(p.x) ? p.x + 1 : ceil(p.x));
		double left = (p.x == int(p.x) ? p.x - 1 : floor(p.x));

		if (v.x == 0 && v.y == 0)
			break;

		length_top = (top - p.y) / v.y;
		length_bottom = (bottom - p.y) / v.y;
		length_left = (left - p.x) / v.x;
		length_right = (right - p.x) / v.x;

		length_segment = getMinimumPositive(length_top, length_bottom, length_left, length_right);

		Point p_next = Point(p.x + length_segment*v.x, p.y + length_segment*v.y);
		if (isLegal(p_next)){
			streamline.push_back(p_next);
			length_total += length_segment;
			p = p_next;
		}
		else{
			break;
		}
	}

	double length_total = 0.0;
	double length_segment;
	double length_top, length_bottom, length_left, length_right;

	Point p = start;

	while (length_total<maxLength){
		icVector2 v = getVector(int(p.x), int(p.y));
		v = -v;
		normalize(v);

		double top = (p.y == int(p.y) ? p.y + 1 : ceil(p.y));
		double bottom = (p.y == int(p.y) ? p.y - 1 : floor(p.y));
		double right = (p.x == int(p.x) ? p.x + 1 : ceil(p.x));
		double left = (p.x == int(p.x) ? p.x - 1 : floor(p.x));

		if (v.x == 0 && v.y == 0)
			break;

		length_top = (top - p.y) / v.y;
		length_bottom = (bottom - p.y) / v.y;
		length_left = (left - p.x) / v.x;
		length_right = (right - p.x) / v.x;

		length_segment = getMinimumPositive(length_top, length_bottom, length_left, length_right);

		Point p_next = Point(p.x + length_segment*v.x, p.y + length_segment*v.y);
		if (isLegal(p_next)){
			streamline.push_back(p_next);
			length_total += length_segment;
			p = p_next;
		}
		else{
			break;
		}

	}
}


// use glut to display a string of characters using a raster font:

void
DoRasterString(float x, float y, float z, char *s)
{
	char c;         // one character to print

	glRasterPos3f((GLfloat)x, (GLfloat)y, (GLfloat)z);
	for (; (c = *s) != '\0'; s++)
	{
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, c);
	}
}


// use glut to display a string of characters using a stroke font:

void
DoStrokeString(float x, float y, float z, float ht, char *s)
{
	char c;         // one character to print

	glPushMatrix();
	glTranslatef((GLfloat)x, (GLfloat)y, (GLfloat)z);
	float sf = ht / (119.05 + 33.33);
	glScalef((GLfloat)sf, (GLfloat)sf, (GLfloat)sf);
	for (; (c = *s) != '\0'; s++)
	{
		glutStrokeCharacter(GLUT_STROKE_ROMAN, c);
	}
	glPopMatrix();
}


// return the number of seconds since the start of the program:

float
ElapsedSeconds()
{
	// get # of milliseconds since the start of the program:

	int ms = glutGet(GLUT_ELAPSED_TIME);

	// convert it to seconds:

	return (float)ms / 1000.;
}


// initialize the glui window:

void
InitGlui()
{
	//GLUI_Panel *panel, *bigpanel;
	//GLUI_RadioGroup *group;
	//GLUI_Rotation *rot;
	//GLUI_Translation *trans, *scale;
	//GLUI_HSlider *slider;
	//GLUI_Spinner *spinner;
	//char str[256];


	//// setup the glui window:

	//glutInitWindowPosition(INIT_WINDOW_SIZE + 50, 0);
	//Glui = GLUI_Master.create_glui((char *)GLUITITLE);


	//Glui->add_statictext((char *)GLUITITLE);
	//Glui->add_separator();

	//Glui->add_checkbox("Axes", &AxesOn);

	//Glui->add_checkbox("Perspective", &WhichProjection);
	////Glui->add_checkbox("Grayscale", &GrayOn);
	////Glui->add_checkbox("Cutting Plane", &PlaneOn);
	//Glui->add_checkbox("Intensity Depth Cue", &DepthCueOn);

	////Project6 glui
	//Glui->add_checkbox("Bounding Box", &BoxOn);
	//Glui->add_checkbox("Vector Cloud", &VectorcloudOn);
	//Glui->add_checkbox("Streamlines", &StreamlinesOn);

	//bigpanel = Glui->add_panel("Probe");
	//group = Glui->add_radiogroup_to_panel(bigpanel, &WhichKindVector, 0, (GLUI_Update_CB)NewVector);
	//Glui->add_radiobutton_to_group(group, "Off");
	//Glui->add_radiobutton_to_group(group, "Single");
	//Glui->add_radiobutton_to_group(group, "Multi");
	//Glui->add_radiobutton_to_group(group, "Ribbon");
	//Glui->add_radiobutton_to_group(group, "Blob");

	//panel = Glui->add_panel_to_panel(bigpanel, "Move The Probe");

	//Glui->add_column_to_panel(panel, GLUIFALSE);
	//trans = Glui->add_translation_to_panel(panel, "Probe XY", GLUI_TRANSLATION_XY, &ProbeXY[0]);
	//trans->set_speed(0.05f);

	//Glui->add_column_to_panel(panel, GLUIFALSE);
	//trans = Glui->add_translation_to_panel(panel, "Probe YZ", GLUI_TRANSLATION_XY, &ProbeYZ[0]);
	//trans->set_speed(0.05f);

	//Glui->add_column_to_panel(panel, GLUIFALSE);
	//trans = Glui->add_translation_to_panel(panel, "Probe XZ", GLUI_TRANSLATION_XY, &ProbeXZ[0]);
	//trans->set_speed(0.05f);

	//Glui->add_separator();
	//spinner = Glui->add_spinner("Time Step", GLUI_SPINNER_FLOAT, &TimeStep, 0);
	//spinner->set_float_limits(0.02, 1.);
	//spinner->set_speed(4.0);

	//Glui->add_separator();
	//VectorLabel = Glui->add_statictext("Vector Cloud Scale:");
	//VectorCloudScale = Glui->add_slider(false, GLUI_HSLIDER_FLOAT, VectorLowHigh, VECTORSCALE, (GLUI_Update_CB)Sliders);
	//VectorCloudScale->set_float_limits(0.f, 0.3);
	//VectorCloudScale->set_w(200);
	//sprintf(str, "", VectorLowHigh[0], VectorLowHigh[1]);


	//Glui->add_separator();
	//FlowLabel = Glui->add_statictext("Flow Speed Range Slider:");
	//FlowSpeed = Glui->add_slider(true, GLUI_HSLIDER_FLOAT, FlowLowHigh, FLOWSPEED, (GLUI_Update_CB)Sliders);
	//FlowSpeed->set_float_limits(0, sqrt(12.));
	//FlowSpeed->set_w(200);



	//panel = Glui->add_panel("Object Transformation");

	//rot = Glui->add_rotation_to_panel(panel, "Rotation", (float *)RotMatrix);



	//// allow the object to be spun via the glui rotation widget:
	//rot->set_spin(1.0);


	//Glui->add_column_to_panel(panel, GLUIFALSE);
	//scale = Glui->add_translation_to_panel(panel, "Scale", GLUI_TRANSLATION_Y, &Scale2);
	//scale->set_speed(0.005f);

	//Glui->add_column_to_panel(panel, GLUIFALSE);
	//trans = Glui->add_translation_to_panel(panel, "Trans XY", GLUI_TRANSLATION_XY, &TransXYZ[0]);
	//trans->set_speed(0.05f);

	//Glui->add_column_to_panel(panel, GLUIFALSE);
	//trans = Glui->add_translation_to_panel(panel, "Trans Z", GLUI_TRANSLATION_Z, &TransXYZ[2]);
	//trans->set_speed(0.05f);

	//Glui->add_checkbox("Debug", &DebugOn);


	//panel = Glui->add_panel("", GLUIFALSE);

	//Glui->add_button_to_panel(panel, "Reset", RESET, (GLUI_Update_CB)Buttons);

	//Glui->add_column_to_panel(panel, GLUIFALSE);

	//Glui->add_button_to_panel(panel, "Quit", QUIT, (GLUI_Update_CB)Buttons);

	// //tell glui what graphics window it needs to post a redisplay to:

	//Glui->set_main_gfx_window(MainWindow);


	//// set the graphics window's idle function if needed:

	//GLUI_Master.set_glutIdleFunc(Animate);



}


// initialize the glut and OpenGL libraries:
//  also setup display lists and callback functions

void
InitGraphics()
{
	// setup the display mode:
	// ( *must* be done before call to glutCreateWindow( ) )
	// ask for color, double-buffering, and z-buffering:

	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);


	// set the initial window configuration:

	glutInitWindowPosition(0, 0);
	glutInitWindowSize(INIT_WINDOW_SIZE, INIT_WINDOW_SIZE);


	// open the window and set its title:

	MainWindow = glutCreateWindow(WINDOWTITLE);
	glutSetWindowTitle(WINDOWTITLE);


	// setup the clear values:

	glClearColor(BACKCOLOR[0], BACKCOLOR[1], BACKCOLOR[2], BACKCOLOR[3]);





	// setup the callback functions:

	// DisplayFunc -- redraw the window
	// ReshapeFunc -- handle the user resizing the window
	// KeyboardFunc -- handle a keyboard input
	// MouseFunc -- handle the mouse button going down or up
	// MotionFunc -- handle the mouse moving with a button down
	// PassiveMotionFunc -- handle the mouse moving with a button up
	// VisibilityFunc -- handle a change in window visibility
	// EntryFunc    -- handle the cursor entering or leaving the window
	// SpecialFunc -- handle special keys on the keyboard
	// SpaceballMotionFunc -- handle spaceball translation
	// SpaceballRotateFunc -- handle spaceball rotation
	// SpaceballButtonFunc -- handle spaceball button hits
	// ButtonBoxFunc -- handle button box hits
	// DialsFunc -- handle dial rotations
	// TabletMotionFunc -- handle digitizing tablet motion
	// TabletButtonFunc -- handle digitizing tablet button hits
	// MenuStateFunc -- declare when a pop-up menu is in use
	// TimerFunc -- trigger something to happen a certain time from now
	// IdleFunc -- what to do when nothing else is going on

	glutSetWindow(MainWindow);
	glutDisplayFunc(Display);
	glutReshapeFunc(Resize);

	glutMouseFunc(MouseButton);
	glutMotionFunc(MouseMotion);
	glutPassiveMotionFunc(NULL);
	glutVisibilityFunc(Visibility);
	glutEntryFunc(NULL);
	glutSpecialFunc(NULL);
	glutSpaceballMotionFunc(NULL);
	glutSpaceballRotateFunc(NULL);
	glutSpaceballButtonFunc(NULL);
	glutButtonBoxFunc(NULL);
	glutDialsFunc(NULL);
	glutTabletMotionFunc(NULL);
	glutTabletButtonFunc(NULL);
	glutMenuStateFunc(NULL);
	glutTimerFunc(0, NULL, 0);


	vectorField = (double *)malloc(sizeof(double)*win_width*win_height * 2);
	memset(vectorField, 1.0, sizeof(unsigned char)*win_width*win_height * 2);

	pixelsBuffer = (unsigned char *)malloc(sizeof(unsigned char*)*win_width*win_height * 3);
	memset(pixelsBuffer, 255, sizeof(unsigned char)*win_width*win_height * 3);

	noiseBuffer = (unsigned char *)malloc(sizeof(unsigned char*)*win_width*win_height * 3);
	memset(noiseBuffer, 255, sizeof(unsigned char)*win_width*win_height * 3);

	outputImage = (unsigned char *)malloc(sizeof(unsigned char*)*win_width*win_height * 3);
	memset(outputImage, 255, sizeof(unsigned char)*win_width*win_height * 3);

	inputImage = (unsigned char *)malloc(sizeof(unsigned char*)*win_width*win_height * 3);
	memset(inputImage, 255, sizeof(unsigned char)*win_width*win_height * 3);

	flowVectorField = (double *)malloc(sizeof(double)*win_width*win_height * 2);
	flowScalarField = (double *)malloc(sizeof(double)*win_width*win_height);
	
	fstream infile;
	char version[1000];

	int v_width = DATAWIDTH;
	int v_height = DATAHEIGHT;

	//vectorField = new double*[v_height];
	//for (int i = 0; i < v_height; i++){
	//    vectorField[i] = new double[v_width];
	//}

	scalarField = (double**)malloc(sizeof(double*)*v_height);
	for (int i = 0; i < v_height; i++){
		scalarField[i] = (double*)malloc(sizeof(double)*v_width);
	}

	vectorField = (icVector2**)malloc(sizeof(icVector2*)*v_height);
	for (int i = 0; i < v_height; i++){
		vectorField[i] = (icVector2*)malloc(sizeof(icVector2)*v_width);
	}

	v_status = (int **)malloc(sizeof(int*)*v_height);
	for (int i = 0; i < v_height; i++){
		v_status[i] = (int *)malloc(sizeof(int)*v_width);
	}

	string str;
	infile.open(filename, ios::in);

	if (infile.is_open() == false) {
		return false;
	}

	while (!infile.eof())
	{
		infile.getline(version, 1000);

		str = version;
		if (str == "")
			continue;
		int index = 0;
		//int index_end = 0;
		string temp;

		int x, y, status;
		double v_x, v_y, length;

		index = str.find("\t");
		temp = str.substr(0, index);
		x = stof(temp);
		str = str.substr(index + 1, str.size());

		index = str.find("\t");
		temp = str.substr(0, index);
		y = stof(temp);
		str = str.substr(index + 1, str.size());

		index = str.find("\t");
		temp = str.substr(0, index);
		v_x = stof(temp);
		str = str.substr(index + 1, str.size());

		index = str.find("\t");
		temp = str.substr(0, index);
		v_y = stof(temp);
		str = str.substr(index + 1, str.size());

		index = str.find("\t");
		temp = str.substr(0, index);
		length = stof(temp);
		str = str.substr(index + 1, str.size());

		index = str.find("\t");
		temp = str.substr(0, index);
		status = stof(temp);
		str = str.substr(index + 1, str.size());

		vectorField[y][x] = icVector2(v_x, v_y);
		scalarField[y][x] = length;
		v_status[y][x] = status;

	}

	for (int y = 0; y < win_height; y++){
		for (int x = 0; x < win_width; x++){
			int index = y*win_height + x;

			pixelsBuffer[3 * index] = outputImage[3 * index];
			pixelsBuffer[3 * index + 1] = outputImage[3 * index + 1];
			pixelsBuffer[3 * index + 2] = outputImage[3 * index + 2];
		}
	}
	flag_fieldLoaded = true;

	return true;
	// DO NOT SET THE GLUT IDLE FUNCTION HERE !!
	// glutIdleFunc( NULL );
	// let glui take care of it in InitGlui( )
	for (int i = 0; i < NX; i++)
	{
		for (int j = 0; j < NY; j++)
		{
			Vect[i][j].x = -1. + 2. * (float)i / (float)(NX - 1);
			Vect[i][j].y = -1. + 2. * (float)j / (float)(NY - 1);
		}
	}

	Texture = BmpToTexture("whitenoise.bmp", &Width, &Height);

	if (Texture == NULL || Width != 512|| Height != 512)
	{
		fprintf(stderr,"Wrong!\n");
	}

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glGenTextures(1, &whn);
	glBindTexture(GL_TEXTURE_2D, whn);

	glTexImage2D(GL_TEXTURE_2D, 0, 3, Width, Height, 0, GL_RGB, GL_UNSIGNED_BYTE, Texture);
}


// initialize the display lists that will not change:
// (a display list is a way to store opengl commands in
//  memory so that they can be played back efficiently at a later time
//  with a call to glCallList( )

void
InitLists()
{
	float dx = BOXSIZE / 2.;
	float dy = BOXSIZE / 2.;
	float dz = BOXSIZE / 2.;

	// create the object:

	BoxList = glGenLists(1);
	glNewList(BoxList, GL_COMPILE);

	glBegin(GL_LINE_STRIP);

	glColor3f(1., 1., 1.);
	glVertex3f(-dx, -dy, dz);
	glVertex3f(dx, -dy, dz);
	glVertex3f(dx, dy, dz);
	glVertex3f(-dx, dy, dz);
	glVertex3f(-dx, -dy, dz);

	glVertex3f(-dx, -dy, -dz);
	glVertex3f(-dx, dy, -dz);
	glVertex3f(dx, dy, -dz);
	glVertex3f(dx, -dy, -dz);
	glVertex3f(-dx, -dy, -dz);

	glEnd();
	glBegin(GL_LINES);
	glVertex3f(-dx, dy, -dz);
	glVertex3f(-dx, dy, dz);

	glVertex3f(dx, dy, -dz);
	glVertex3f(dx, dy, dz);

	glVertex3f(dx, -dy, -dz);
	glVertex3f(dx, -dy, dz);

	glEnd();


	glEndList();



	// create the axes:
	AxesList = glGenLists(1);
	glNewList(AxesList, GL_COMPILE);
	
	
	glLineWidth(1.);
	glEndList();
}




// called when the mouse button transitions down or up:

void
MouseButton(int button, int state, int x, int y)
{
	int b = 0;          // LEFT, MIDDLE, or RIGHT

	if (DebugOn != 0)
		fprintf(stderr, "MouseButton: %d, %d, %d, %d\n", button, state, x, y);


	// get the proper button bit mask:

	switch (button)
	{
	case GLUT_LEFT_BUTTON:
		b = LEFT;       break;

	case GLUT_MIDDLE_BUTTON:
		b = MIDDLE;     break;

	case GLUT_RIGHT_BUTTON:
		b = RIGHT;      break;

	default:
		b = 0;
		fprintf(stderr, "Unknown mouse button: %d\n", button);
	}


	// button down sets the bit, up clears the bit:

	if (state == GLUT_DOWN)
	{
		Xmouse = x;
		Ymouse = y;
		ActiveButton |= b;      // set the proper bit
	}
	else
	{
		ActiveButton &= ~b;     // clear the proper bit
	}
}


// called when the mouse moves while a button is down:

void
MouseMotion(int x, int y)
{
	if (DebugOn != 0)
		fprintf(stderr, "MouseMotion: %d, %d\n", x, y);


	int dx = x - Xmouse;        // change in mouse coords
	int dy = y - Ymouse;

	if ((ActiveButton & LEFT) != 0)
	{
		switch (LeftButton)
		{
		case ROTATE:
			Xrot += (ANGFACT*dy);
			Yrot += (ANGFACT*dx);
			break;

		case SCALE:
			Scale += SCLFACT * (float)(dx - dy);
			if (Scale < MINSCALE)
				Scale = MINSCALE;
			break;
		}
	}


	if ((ActiveButton & MIDDLE) != 0)
	{
		Scale += SCLFACT * (float)(dx - dy);

		// keep object from turning inside-out or disappearing:

		if (Scale < MINSCALE)
			Scale = MINSCALE;
	}

	Xmouse = x;         // new current position
	Ymouse = y;

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}



// reset the transformations and the colors:
// this only sets the global variables --
// the glut main loop is responsible for redrawing the scene

void
Reset()
{
	ActiveButton = 0;
	AxesOn = GLUITRUE;
	GrayOn = GLUIFALSE;

	DebugOn = GLUIFALSE;
	DepthCueOn = GLUIFALSE;
	LeftButton = ROTATE;
	Scale = 1.0;
	Scale2 = 0.0;       // because we add 1. to it in Display( )


	RotMatrix[0][1] = RotMatrix[0][2] = RotMatrix[0][3] = 0.;
	RotMatrix[1][0] = RotMatrix[1][2] = RotMatrix[1][3] = 0.;
	RotMatrix[2][0] = RotMatrix[2][1] = RotMatrix[2][3] = 0.;
	RotMatrix[3][0] = RotMatrix[3][1] = RotMatrix[3][3] = 0.;
	RotMatrix[0][0] = RotMatrix[1][1] = RotMatrix[2][2] = RotMatrix[3][3] = 1.;

	//Reset graph


}


// called when user resizes the window:

void
Resize(int width, int height)
{
	if (DebugOn != 0)
		fprintf(stderr, "ReSize: %d, %d\n", width, height);

	// don't really need to do anything since window size is
	// checked each time in Display( ):

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}


// handle a change to the window's visibility:

void
Visibility(int state)
{
	if (DebugOn != 0)
		fprintf(stderr, "Visibility: %d\n", state);

	if (state == GLUT_VISIBLE)
	{
		glutSetWindow(MainWindow);
		glutPostRedisplay();
	}
	else
	{
		// could optimize by keeping track of the fact
		// that the window is not visible and avoid
		// animating or redrawing it ...
	}
}




///////////////////////////////////////   HANDY UTILITIES:  //////////////////////////

// size of wings as fraction of length:

#define WINGS   0.10


// axes:

#define X   1
#define Y   2
#define Z   3


// x, y, z, axes:

static float axx[3] = { 1., 0., 0. };
static float ayy[3] = { 0., 1., 0. };
static float azz[3] = { 0., 0., 1. };


void
Arrow(float tail[3], float head[3])
{
	float u[3], v[3], w[3];     // arrow coordinate system

	// set w direction in u-v-w coordinate system:

	w[0] = head[0] - tail[0];
	w[1] = head[1] - tail[1];
	w[2] = head[2] - tail[2];


	// determine major direction:

	int axis = X;
	float mag = fabs(w[0]);
	if (fabs(w[1])  > mag)
	{
		axis = Y;
		mag = fabs(w[1]);
	}
	if (fabs(w[2])  > mag)
	{
		axis = Z;
		mag = fabs(w[2]);
	}


	// set size of wings and turn w into a Unit vector:

	float d = WINGS * Unit(w, w);


	// draw the shaft of the arrow:

	glBegin(GL_LINE_STRIP);
	glVertex3fv(tail);
	glVertex3fv(head);
	glEnd();

	// draw two sets of wings in the non-major directions:

	float x, y, z;

	if (axis != X)
	{
		Cross(w, axx, v);
		(void)Unit(v, v);
		Cross(v, w, u);
		x = head[0] + d * (u[0] - w[0]);
		y = head[1] + d * (u[1] - w[1]);
		z = head[2] + d * (u[2] - w[2]);
		glBegin(GL_LINE_STRIP);
		glVertex3fv(head);
		glVertex3f(x, y, z);
		glEnd();
		x = head[0] + d * (-u[0] - w[0]);
		y = head[1] + d * (-u[1] - w[1]);
		z = head[2] + d * (-u[2] - w[2]);
		glBegin(GL_LINE_STRIP);
		glVertex3fv(head);
		glVertex3f(x, y, z);
		glEnd();
	}


	if (axis != Y)
	{
		Cross(w, ayy, v);
		(void)Unit(v, v);
		Cross(v, w, u);
		x = head[0] + d * (u[0] - w[0]);
		y = head[1] + d * (u[1] - w[1]);
		z = head[2] + d * (u[2] - w[2]);
		glBegin(GL_LINE_STRIP);
		glVertex3fv(head);
		glVertex3f(x, y, z);
		glEnd();
		x = head[0] + d * (-u[0] - w[0]);
		y = head[1] + d * (-u[1] - w[1]);
		z = head[2] + d * (-u[2] - w[2]);
		glBegin(GL_LINE_STRIP);
		glVertex3fv(head);
		glVertex3f(x, y, z);
		glEnd();
	}



	if (axis != Z)
	{
		Cross(w, azz, v);
		(void)Unit(v, v);
		Cross(v, w, u);
		x = head[0] + d * (u[0] - w[0]);
		y = head[1] + d * (u[1] - w[1]);
		z = head[2] + d * (u[2] - w[2]);
		glBegin(GL_LINE_STRIP);
		glVertex3fv(head);
		glVertex3f(x, y, z);
		glEnd();
		x = head[0] + d * (-u[0] - w[0]);
		y = head[1] + d * (-u[1] - w[1]);
		z = head[2] + d * (-u[2] - w[2]);
		glBegin(GL_LINE_STRIP);
		glVertex3fv(head);
		glVertex3f(x, y, z);
		glEnd();
	}
}



float
Dot(float v1[3], float v2[3])
{
	return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2];
}



void
Cross(float v1[3], float v2[3], float vout[3])
{
	float tmp[3];

	tmp[0] = v1[1] * v2[2] - v2[1] * v1[2];
	tmp[1] = v2[0] * v1[2] - v1[0] * v2[2];
	tmp[2] = v1[0] * v2[1] - v2[0] * v1[1];

	vout[0] = tmp[0];
	vout[1] = tmp[1];
	vout[2] = tmp[2];
}



float
Unit(float vin[3], float vout[3])
{
	float dist = vin[0] * vin[0] + vin[1] * vin[1] + vin[2] * vin[2];

	if (dist > 0.0)
	{
		dist = sqrt(dist);
		vout[0] = vin[0] / dist;
		vout[1] = vin[1] / dist;
		vout[2] = vin[2] / dist;
	}
	else
	{
		vout[0] = vin[0];
		vout[1] = vin[1];
		vout[2] = vin[2];
	}

	return dist;
}



// the stroke characters 'X' 'Y' 'Z' :

static float xx[] = {
	0.f, 1.f, 0.f, 1.f
};

static float xy[] = {
	-.5f, .5f, .5f, -.5f
};

static int xorder[] = {
	1, 2, -3, 4
};


static float yx[] = {
	0.f, 0.f, -.5f, .5f
};

static float yy[] = {
	0.f, .6f, 1.f, 1.f
};

static int yorder[] = {
	1, 2, 3, -2, 4
};


static float zx[] = {
	1.f, 0.f, 1.f, 0.f, .25f, .75f
};

static float zy[] = {
	.5f, .5f, -.5f, -.5f, 0.f, 0.f
};

static int zorder[] = {
	1, 2, 3, 4, -5, 6
};


// fraction of the length to use as height of the characters:

const float LENFRAC = 0.10f;


// fraction of length to use as start location of the characters:

const float BASEFRAC = 1.10f;


//  Draw a set of 3D axes:
//  (length is the axis length in world coordinates)

void
Axes(float length)
{
	glBegin(GL_LINE_STRIP);
	glVertex3f(length, 0., 0.);
	glVertex3f(0., 0., 0.);
	glVertex3f(0., length, 0.);
	glEnd();
	glBegin(GL_LINE_STRIP);
	glVertex3f(0., 0., 0.);
	glVertex3f(0., 0., length);
	glEnd();

	float fact = LENFRAC * length;
	float base = BASEFRAC * length;

	glBegin(GL_LINE_STRIP);
	for (int i = 0; i < 4; i++)
	{
		int j = xorder[i];
		if (j < 0)
		{

			glEnd();
			glBegin(GL_LINE_STRIP);
			j = -j;
		}
		j--;
		glVertex3f(base + fact*xx[j], fact*xy[j], 0.0);
	}
	glEnd();

	glBegin(GL_LINE_STRIP);
	for (int i = 0; i < 5; i++)
	{
		int j = yorder[i];
		if (j < 0)
		{

			glEnd();
			glBegin(GL_LINE_STRIP);
			j = -j;
		}
		j--;
		glVertex3f(fact*yx[j], base + fact*yy[j], 0.0);
	}
	glEnd();

	glBegin(GL_LINE_STRIP);
	for (int i = 0; i < 6; i++)
	{
		int j = zorder[i];
		if (j < 0)
		{

			glEnd();
			glBegin(GL_LINE_STRIP);
			j = -j;
		}
		j--;
		glVertex3f(0.0, fact*zy[j], base + fact*zx[j]);
	}
	glEnd();

}

//the function to calculate one point's speed
void
Vector(float x, float y, float *vx, float *vy)
{
	/*x = A * x + B * y;
	y = C *x + D * y*/;
	*vx = x*y *x ;
	*vy = -x *y *y;
	*vx = A * *vx + B * *vy;
	*vy = C * *vx + D * *vy;
}




//the function to calculate streamlines
void
Streamline(float x, float y)
{
	float vx ,vy;
	glLineWidth(1.);

	glBegin(GL_LINE_STRIP);

	for (int i = 0; i < MAX_ITERATIONS; i++)
	{
		if (x < XMIN || x > XMAX)
			break;
		if (y < YMIN || y > YMAX)
			break;

		glColor3f(Vect[((int)x + 1) / 2 * NX][((int)y + 1) / 2 * NY].noise, Vect[((int)x + 1) / 2 * NX][((int)y + 1) / 2 * NY].noise, Vect[((int)x + 1) / 2 * NX][((int)y + 1) / 2 * NY].noise);
		glVertex2f(x, y);

		Vector(x, y, &vx, &vy);
		/*if (sqrt(SQR(vx)) < 0 || sqrt(SQR(vy)) < 0)
			break;*/
		Advect(&x, &y);
	}
	glEnd();
}
void
interpolateFields(double **scalarField, double **vectorField, int **v_status){

	memset(flowVectorField, 0.0, sizeof(unsigned char)*win_width*win_height * 2);
	memset(flowScalarField, 0.0, sizeof(unsigned char)*win_width*win_height);

	//double grid_width = double(win_width) / 125.0;
	//double grid_height = double(win_height) / 124.0;

	double grid_width = double(win_width) / double(DATAWIDTH - 1);
	double grid_height = double(win_height) / double(DATAHEIGHT - 1);

	//double grid_width = 10.0;
	//double grid_height = 10.0;

	Point p1, p2, p3;
	double scalar_a, scalar_b, scalar_c;
	icVector2 vec_a, vec_b, vec_c;
	double alpha, beta, gama;

	for (int i = 0; i < win_height; i++){
		for (int j = 0; j < win_width; j++){

			int index = i*win_width + j;

			int n = j / grid_width;
			int m = i / grid_height;

			double k1, k2;
			k1 = grid_height / grid_width;
			if ((i - m*grid_height) == 0)
				k2 = 0;
			else if ((j - n*grid_width) == 0)
				k2 = k1 + 100;
			else
				k2 = double(i - m*grid_height) / double(j - n*grid_width);

			if (k2 <= k1){

				scalar_a = scalarField[m][n];
				scalar_b = scalarField[m][n + 1];
				scalar_c = scalarField[m + 1][n + 1];

				vec_a = vectorField[m][n];
				vec_b = vectorField[m][n + 1];
				vec_c = vectorField[m + 1][n + 1];

				p1 = Point(n*grid_width, m*grid_height);
				p2 = Point((n + 1)*grid_width, m*grid_height);
				p3 = Point((n + 1)*grid_width, (m + 1)*grid_height);

			}
			else{
				scalar_a = scalarField[m][n];
				scalar_b = scalarField[m + 1][n + 1];
				scalar_c = scalarField[m + 1][n];

				vec_a = vectorField[m][n];
				vec_b = vectorField[m + 1][n + 1];
				vec_c = vectorField[m + 1][n];

				p1 = Point(n*grid_width, m*grid_height);
				p2 = Point((n + 1)*grid_width, (m + 1)*grid_height);
				p3 = Point(n*grid_width, (m + 1)*grid_height);

			}

			double scalar = interpolate(j, i, p1, p2, p3, scalar_a, scalar_b, scalar_c);

			flowScalarField[index] = scalar;

			double vector_x = interpolate(j, i, p1, p2, p3, vec_a.x, vec_b.x, vec_c.x);
			double vector_y = interpolate(j, i, p1, p2, p3, vec_a.y, vec_b.y, vec_c.y);
			if (v_status[m][n] == 0 || v_status[m][n] == 16){
				flowVectorField[2 * index] = vector_x;
				flowVectorField[2 * index + 1] = vector_y;
			}

		}
	}

}

void
Advect(float *x, float *y)
{
	float xa = *x;
	float ya = *y;
	float xc;
	float yc;
	float xb;
	float yb;

	float vxa;
	float vya;
	float vxb;
	float vyb;

	float vx;
	float vy;

	Vector(xa, ya, &vxa, &vya);
	
	xb = xa + TimeStep * vxa;
	yb = ya + TimeStep * vya;

	Vector(xb, yb, &vxb, &vyb);

	vx = (vxa + vxb) / 2;


	vy = (vya + vyb) / 2;

	xc = xa + TimeStep * vx;
	yc = ya + TimeStep * vy;

	*x = xc;
	*y = yc;
}



void
interpolate(double x, double y, Point v1, Point v2, Point v3, double va1, double va2, double va3){
	double area, s;
	double a1, b1, c1, a2, b2, c2, a3, b3, c3;
	double alpha, beta, gamma;
	double edge1, edge2, edge3; 
	double h1, h2, h3;

	edge1 = sqrt((v3.x - v2.x)*(v3.x - v2.x) + (v3.y - v2.y)*(v3.y - v2.y));
	edge2 = sqrt((v1.x - v3.x)*(v1.x - v3.x) + (v1.y - v3.y)*(v1.y - v3.y));
	edge3 = sqrt((v2.x - v1.x)*(v2.x - v1.x) + (v2.y - v1.y)*(v2.y - v1.y));

	s = (edge1 + edge2 + edge3)*0.5;
	area = sqrt(s*(s - edge1)*(s - edge2)*(s - edge3));

	a1 = v3.y - v2.y;
	b1 = v2.x - v3.x;
	c1 = v3.x*v2.y - v2.x*v3.y;

	a2 = v1.y - v3.y;
	b2 = v3.x - v1.x;
	c2 = v1.x*v3.y - v3.x*v1.y;

	a3 = v2.y - v1.y;
	b3 = v1.x - v2.x;
	c3 = v2.x*v1.y - v1.x*v2.y;

	alpha = (fabs(x * a1 + y * b1 + c1)*0.5*edge1) / (sqrt(a1*a1 + b1*b1)*area);
	beta = (fabs(x * a2 + y * b2 + c2)*0.5*edge2) / (sqrt(a2*a2 + b2*b2)*area);
	gamma = (fabs(x * a3 + y * b3 + c3)*0.5*edge3) / (sqrt(a3*a3 + b3*b3)*area);

	double result;
	result = alpha*val_1 + beta*val_2 + gamma*val_3;
	return result;
}

/**
** read a BMP file into a Texture:
**/

unsigned char *
BmpToTexture(char *filename, int *width, int *height)
{

	int s, t, e;		// counters
	int numextra;		// # extra bytes each line in the file is padded with
	FILE *fp;
	unsigned char *texture;
	int nums, numt;
	unsigned char *tp;


	fp = fopen(filename, "rb");
	if (fp == NULL)
	{
		fprintf(stderr, "Cannot open Bmp file '%s'\n", filename);
		return NULL;
	}

	FileHeader.bfType = ReadShort(fp);


	// if bfType is not 0x4d42, the file is not a bmp:

	if (FileHeader.bfType != 0x4d42)
	{
		fprintf(stderr, "Wrong type of file: 0x%0x\n", FileHeader.bfType);
		fclose(fp);
		return NULL;
	}


	FileHeader.bfSize = ReadInt(fp);
	FileHeader.bfReserved1 = ReadShort(fp);
	FileHeader.bfReserved2 = ReadShort(fp);
	FileHeader.bfOffBits = ReadInt(fp);


	InfoHeader.biSize = ReadInt(fp);
	InfoHeader.biWidth = ReadInt(fp);
	InfoHeader.biHeight = ReadInt(fp);

	nums = InfoHeader.biWidth;
	numt = InfoHeader.biHeight;

	InfoHeader.biPlanes = ReadShort(fp);
	InfoHeader.biBitCount = ReadShort(fp);
	InfoHeader.biCompression = ReadInt(fp);
	InfoHeader.biSizeImage = ReadInt(fp);
	InfoHeader.biXPelsPerMeter = ReadInt(fp);
	InfoHeader.biYPelsPerMeter = ReadInt(fp);
	InfoHeader.biClrUsed = ReadInt(fp);
	InfoHeader.biClrImportant = ReadInt(fp);


	// fprintf( stderr, "Image size found: %d x %d\n", ImageWidth, ImageHeight );


	texture = new unsigned char[3 * nums * numt];
	if (texture == NULL)
	{
		fprintf(stderr, "Cannot allocate the texture array!\b");
		return NULL;
	}


	// extra padding bytes:

	numextra = 4 * (((3 * InfoHeader.biWidth) + 3) / 4) - 3 * InfoHeader.biWidth;


	// we do not support compression:

	if (InfoHeader.biCompression != birgb)
	{
		fprintf(stderr, "Wrong type of image compression: %d\n", InfoHeader.biCompression);
		fclose(fp);
		return NULL;
	}



	rewind(fp);
	fseek(fp, 14 + 40, SEEK_SET);

	if (InfoHeader.biBitCount == 24)
	{
		for (t = 0, tp = texture; t < numt; t++)
		{
			for (s = 0; s < nums; s++, tp += 3)
			{
				*(tp + 2) = fgetc(fp);		// b
				*(tp + 1) = fgetc(fp);		// g
				*(tp + 0) = fgetc(fp);		// r
			}

			for (e = 0; e < numextra; e++)
			{
				fgetc(fp);
			}
		}
	}

	fclose(fp);

	*width = nums;
	*height = numt;
	return texture;
}

void Colormapping(double scalar){

	Color c;
	float bound[5];
	float range = 255.0 / 4.0;
	for (int i = 0; i < 5; i++){
		bound[i] = range*i;
	}


	scalar = 255 - (scalar - scalar_min) / scalar_range * 255.0;

	if (scalar < bound[1] && scalar >= bound[0]){
		float divergence = (scalar - bound[0]) / range*255.0;
		
		c.R = 255.;
		c.G = divergence;
		c.B = 0.;
	}
	else if (scalar < bound[2] && scalar >= bound[1]){
		float divergence = (scalar - bound[1]) / range*255.0;
		
		c.R = 255 - divergence;
		c.G = 255;
		c.B = 0.;
	}
	else if (scalar < bound[3] && scalar >= bound[2]){
		float divergence = (scalar - bound[2]) / range*255.0;
		
		c.R = 0.;
		c.G = 255;
		c.B = divergence;
	}
	else if (scalar <= bound[4] && scalar >= bound[3]){
		float divergence = (scalar - bound[3]) / range*255.0;
		
		c.B = 0.0;*/
		c.R = 0.;
		c.G = 255 - divergence;
		c.B = 255;
	}
	return c;

}


int
ReadInt(FILE *fp)
{
	unsigned char b3, b2, b1, b0;
	b0 = fgetc(fp);
	b1 = fgetc(fp);
	b2 = fgetc(fp);
	b3 = fgetc(fp);
	return (b3 << 24) | (b2 << 16) | (b1 << 8) | b0;
}


short
ReadShort(FILE *fp)
{
	unsigned char b1, b0;
	b0 = fgetc(fp);
	b1 = fgetc(fp);
	return (b1 << 8) | b0;
}



