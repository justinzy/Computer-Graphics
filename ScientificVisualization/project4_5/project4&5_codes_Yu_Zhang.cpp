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

const float TEMPMIN = { 0.f };
const float TEMPMAX = { 100.f };
const float GRAYMIN = { 0.20f };
const float GRAYMAX = { 1.00f };
const float AXISMAX = { 1.f };
const float AXISMIN = { -1.f };
const float RADMIN = { 0.f };
const float RADMAX = { sqrt(3.f) };
const float GRADMAX = { 300.f };
const float GRADMIN = { 0.f };

#define NX  50
#define NY  50
#define NZ  50

#define TEMP    0
#define XAXIS   1
#define YAXIS   2
#define ZAXIS   3
#define RAD     4
#define GRAD    5

#define XMIN    -1
#define XMAX    1
#define YMIN    -1
#define YMAX    1
#define ZMIN    -1
#define ZMAX    1

float           PlaneX;
float           PlaneY;
float           PlaneZ;
float           TempLowHigh[2];
float           XaxisLowHigh[2];
float           YaxisLowHigh[2];
float           ZaxisLowHigh[2];
float           RadLowHigh[2];
float           GradLowHigh[2];
const char *    TEMPFORMAT = { "Temperature: %5.2f - %5.2f" };
const char *    XFORMAT = { "X axis: %f - %f" };
const char *    YFORMAT = { "Y axis: %f - %f" };
const char *    ZFORMAT = { "Z axis: %f - %f" };
const char *    RADFORMAT = { "Radius: %f - %f" };
const char *    GRADFORMAT = { "Gradient: %f -%f" };
int				NsSteps;
float			WireSstar;
//define a 3D array of structure
struct node
{
	float x, y, z;          // location
	float T;                // temperature
	float r, g, b;      // the assigned color
	float hsv[3];
	float rgb[3];
	float rad;              // radius
	float dTdx, dTdy, dTdz; // can store these if you want, or not
	float grad;             // total gradient
	float gray;
};
struct node  Nodes[NX][NY][NZ];

struct centers
{
	float xc, yc, zc;       // center location
	float a;                // amplitude
} Centers[] =
{
	{ 1.00f, 0.00f, 0.00f, 90.00f },
	{ -1.00f, 0.30f, 0.00f, 120.00f },
	{ 0.00f, 1.00f, 0.00f, 120.00f },
	{ 0.00f, 0.40f, 1.00f, 170.00f },
};

inline float SQR(float x)
{
	return x * x;
}

float
Temperature(float x, float y, float z)
{
	float t = 0.0;

	for (int i = 0; i <= 3; i++)
	{
		float dx = x - Centers[i].xc;
		float dy = y - Centers[i].yc;
		float dz = z - Centers[i].zc;
		float rsqd = SQR(dx) + SQR(dy) + SQR(dz);
		t += Centers[i].a * exp(-5.*rsqd);
	}

	if (t > TEMPMAX)
		t = TEMPMAX;

	return t;
}

//Setup a two dimensional array representing the discrete nodes in a cutting plane.
struct nodeTwoD
{
	float x, y, z;          // location
	float T;                // temperature
	float r, g, b;        // the assigned color
	float hsv[3];
	float rgb[3];
};
struct nodeTwoD PlaneXY[NX][NY], PlaneXZ[NX][NZ], PlaneYZ[NY][NZ], Plane[NX][NY][NZ];


//  This is a sample OpenGL / GLUT / GLUI program
//
//  The objective is to draw a 3d object and change the color of the axes
//      with radio buttons
//
//  The left mouse button allows rotation
//  The middle mouse button allows scaling
//  The glui window allows:
//      1. The 3d object to be transformed
//      2. The projection to be changed
//      3. The color of the axes to be changed
//      4. The axes to be turned on and off
//      5. The transformations to be reset
//      6. The program to quit
//
//  Author:         Joe Graphics
//
//  Latest update:  March 26, 2015

// constants:
//
// NOTE: There are a bunch of good reasons to use const variables instead
// of #define's.  However, Visual C++ does not allow a const variable
// to be used as an array size or as the case in a switch( ) statement.  So in
// the following, all constants are const variables except those which need to
// be array sizes or cases in switch( ) statements.  Those are #defines.
//


// title of these windows:

const char *WINDOWTITLE = { "OpenGL / GLUT / GLUI Project#3-- Author: Yu Zhang" };
const char *GLUITITLE = { "User Interface Window" };


// what the glui package defines as true and false:

const int GLUITRUE = { true };
const int GLUIFALSE = { false };


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

enum LeftButton
{
	ROTATE,
	SCALE
};


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


// line width for the axes:

const GLfloat AXES_WIDTH = { 3. };


// the color numbers:
// this order must match the radio button order

enum Colors
{
	RED,
	YELLOW,
	GREEN,
	CYAN,
	BLUE,
	MAGENTA,
	WHITE,
	BLACK
};


// the color definitions:
// this order must match the radio button order

const GLfloat Colors[][3] =
{
	{ 1., 0., 0. },     // red
	{ 1., 1., 0. },     // yellow
	{ 0., 1., 0. },     // green
	{ 0., 1., 1. },     // cyan
	{ 0., 0., 1. },     // blue
	{ 1., 0., 1. },     // magenta
	{ 1., 1., 1. },     // white
	{ 0., 0., 0. },     // black
};


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
int Xmouse, Ymouse;         // mouse values
float   Xrot, Yrot;         // rotation angles in degrees
float   TransXYZ[3];        // set by glui translation widgets
GLUI_HSlider *      TempSlider;
GLUI_HSlider *      XSlider;
GLUI_HSlider *      YSlider;
GLUI_HSlider *      ZSlider;
GLUI_HSlider *      RadSlider;
GLUI_HSlider *      GradSlider;
GLUI_StaticText *   XLabel;
GLUI_StaticText *   YLabel;
GLUI_StaticText *   ZLabel;
GLUI_StaticText *   TempLabel;
GLUI_StaticText *   RadLabel;
GLUI_StaticText *   GradLabel;
// function prototypes:

void    Animate();
void    Buttons(int);
void    Display();
void    DoRasterString(float, float, float, char *);
void    DoStrokeString(float, float, float, float, char *);
float   ElapsedSeconds();
void    InitGlui();
void    InitGraphics();
void    InitLists();
void    Keyboard(unsigned char, int, int);
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
void    Buttons(int);
void    Sliders(int);
void    NewPlane(int);
void	NewWireSurface(int);
void	ColorSquare(struct nodeTwoD*, struct nodeTwoD*, struct nodeTwoD*, struct nodeTwoD*);
void	Processquad(struct nodeTwoD*, struct nodeTwoD*, struct nodeTwoD*, struct nodeTwoD*, int);
void	Isoquad(struct node*, struct node*, struct node*, struct node*, int);
void NewPlane(int id)
{

}

void NewWireSurface(int id)
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

void
Animate()
{
	// put animation stuff in here -- change some global variables
	// for Display( ) to find:



	// force a call to Display( ) next time it is convenient:

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}



// glui buttons callback:

void
Buttons(int id)
{
	char str[1024];
	switch (id)
	{
	case RESET:
		Reset();
		sprintf(str, TEMPFORMAT, TempLowHigh[0], TempLowHigh[1]);
		TempLabel->set_text(str);

		sprintf(str, XFORMAT, XaxisLowHigh[0], XaxisLowHigh[1]);
		XLabel->set_text(str);

		sprintf(str, YFORMAT, YaxisLowHigh[0], YaxisLowHigh[1]);
		YLabel->set_text(str);

		sprintf(str, ZFORMAT, ZaxisLowHigh[0], ZaxisLowHigh[1]);
		ZLabel->set_text(str);

		sprintf(str, RADFORMAT, RadLowHigh[0], RadLowHigh[1]);
		RadLabel->set_text(str);

		sprintf(str, GRADFORMAT, GradLowHigh[0], GradLowHigh[1]);
		GradLabel->set_text(str);

		Glui->sync_live();
		glutSetWindow(MainWindow);
		glutPostRedisplay();
		break;

	case QUIT:
		// gracefully close the glui window:
		// gracefully close out the graphics:
		// gracefully close the graphics window:
		// gracefully exit the program:

		Glui->close();
		glutSetWindow(MainWindow);
		glFinish();
		glutDestroyWindow(MainWindow);
		exit(0);
		break;

	default:
		fprintf(stderr, "Don't know what to do with Button ID %d\n", id);
	}

}


//slider generation

void
Sliders(int id)
{
	char str[64];

	switch (id)
	{
	case TEMP:
		sprintf(str, TEMPFORMAT, TempLowHigh[0], TempLowHigh[1]);
		TempLabel->set_text(str);
		break;

	case XAXIS:
		sprintf(str, XFORMAT, XaxisLowHigh[0], XaxisLowHigh[1]);
		XLabel->set_text(str);
		break;

	case YAXIS:
		sprintf(str, YFORMAT, YaxisLowHigh[0], YaxisLowHigh[1]);
		YLabel->set_text(str);
		break;

	case ZAXIS:
		sprintf(str, ZFORMAT, ZaxisLowHigh[0], ZaxisLowHigh[1]);
		ZLabel->set_text(str);
		break;

	case RAD:
		sprintf(str, RADFORMAT, RadLowHigh[0], RadLowHigh[1]);
		RadLabel->set_text(str);
		break;

	case GRAD:
		sprintf(str, GRADFORMAT, GradLowHigh[0], GradLowHigh[1]);
		GradLabel->set_text(str);
		break;
	}
	glutSetWindow(MainWindow);
	glutPostRedisplay();
}



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
	glEnable(GL_DEPTH_TEST);


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
	if (WhichProjection == ORTHO)
		glOrtho(-3., 3., -3., 3., 0.1, 1000.);
	else
		gluPerspective(90., 1., 0.1, 1000.);


	// place the objects into the scene:

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();


	// set the eye position, look-at position, and up-vector:
	// IF DOING 2D, REMOVE THIS -- OTHERWISE ALL YOUR 2D WILL DISAPPEAR !

	gluLookAt(0., 0., 3., 0., 0., 0., 0., 1., 0.);


	// translate the objects in the scene:
	// note the minus sign on the z value
	// this is to make the appearance of the glui z translate
	// widget more intuitively match the translate behavior
	// DO NOT TRANSLATE IN Z IF YOU ARE DOING 2D !

	glTranslatef((GLfloat)TransXYZ[0], (GLfloat)TransXYZ[1], -(GLfloat)TransXYZ[2]);


	// rotate the scene:
	// DO NOT ROTATE (EXCEPT ABOUT Z) IF YOU ARE DOING 2D !

	glRotatef((GLfloat)Yrot, 0., 1., 0.);
	glRotatef((GLfloat)Xrot, 1., 0., 0.);
	glMultMatrixf((const GLfloat *)RotMatrix);


	// uniformly scale the scene:

	glScalef((GLfloat)Scale, (GLfloat)Scale, (GLfloat)Scale);
	GLfloat scale2 = 1. + Scale2;       // because glui translation starts at 0.
	if (scale2 < MINSCALE)
		scale2 = MINSCALE;
	glScalef((GLfloat)scale2, (GLfloat)scale2, (GLfloat)scale2);


	// set the fog parameters:
	// DON'T NEED THIS IF DOING 2D !

	if (DepthCueOn != 0)
	{
		glFogi(GL_FOG_MODE, FOGMODE);
		glFogfv(GL_FOG_COLOR, FOGCOLOR);
		glFogf(GL_FOG_DENSITY, FOGDENSITY);
		glFogf(GL_FOG_START, FOGSTART);
		glFogf(GL_FOG_END, FOGEND);
		glEnable(GL_FOG);
	}
	else
	{
		glDisable(GL_FOG);
	}


	// possibly draw the axes:

	if (AxesOn != 0)
	{
		glColor3fv(&Colors[WhichColor][0]);
		glCallList(AxesList);
	}


	//computer 2d array
	for (int i = 0; i < NX; i++)
	{
		for (int j = 0; j < NY; j++)
		{
			PlaneXY[i][j].x = -1. + 2. * (float)i / (float)(NX - 1);
			PlaneXY[i][j].y = -1. + 2. * (float)j / (float)(NY - 1);
			PlaneXY[i][j].z = PlaneZ;
			PlaneXY[i][j].hsv[1] = 1.;
			PlaneXY[i][j].hsv[2] = 1.;
			PlaneXY[i][j].T = Temperature(PlaneXY[i][j].x, PlaneXY[i][j].y, PlaneXY[i][j].z);
			PlaneXY[i][j].hsv[0] = 240. - 240. * (PlaneXY[i][j].T - TEMPMIN) / (TEMPMAX - TEMPMIN);
			HsvRgb(PlaneXY[i][j].hsv, PlaneXY[i][j].rgb);

			PlaneXZ[i][j].x = -1. + 2. * (float)i / (float)(NX - 1);
			PlaneXZ[i][j].y = PlaneY;
			PlaneXZ[i][j].z = -1. + 2. * (float)j / (float)(NZ - 1);
			PlaneXZ[i][j].hsv[1] = 1.;
			PlaneXZ[i][j].hsv[2] = 1.;
			PlaneXZ[i][j].T = Temperature(PlaneXZ[i][j].x, PlaneXZ[i][j].y, PlaneXZ[i][j].z);
			PlaneXZ[i][j].hsv[0] = 240. - 240. * (PlaneXZ[i][j].T - TEMPMIN) / (TEMPMAX - TEMPMIN);
			HsvRgb(PlaneXZ[i][j].hsv, PlaneXZ[i][j].rgb);

			PlaneYZ[i][j].x = PlaneX;
			PlaneYZ[i][j].y = -1. + 2. * (float)i / (float)(NY - 1);
			PlaneYZ[i][j].z = -1. + 2. * (float)j / (float)(NZ - 1);
			PlaneYZ[i][j].hsv[1] = 1.;
			PlaneYZ[i][j].hsv[2] = 1.;
			PlaneYZ[i][j].T = Temperature(PlaneYZ[i][j].x, PlaneYZ[i][j].y, PlaneYZ[i][j].z);
			PlaneYZ[i][j].hsv[0] = 240. - 240. * (PlaneYZ[i][j].T - TEMPMIN) / (TEMPMAX - TEMPMIN);
			HsvRgb(PlaneYZ[i][j].hsv, PlaneYZ[i][j].rgb);

		}
	}

	// set the color of the object:

	glColor3fv(Colors[WhichColor]);


	if (PlaneOn == 0)
	{
		if (GrayOn != 0)
		{
			glPointSize(2);
			glBegin(GL_POINTS);
			for (int i = 0; i < NX; i++)
			{
				if (Nodes[i][0][0].x < XaxisLowHigh[0] || Nodes[i][0][0].x > XaxisLowHigh[1])
					continue;

				for (int j = 0; j < NY; j++)
				{
					if (Nodes[i][j][0].y < YaxisLowHigh[0] || Nodes[i][j][0].y > YaxisLowHigh[1])
						continue;

					for (int k = 0; k < NZ; k++)
					{
						if (Nodes[i][j][k].z < ZaxisLowHigh[0] || Nodes[i][j][k].z > ZaxisLowHigh[1])
							continue;

						if (Nodes[i][j][k].T <TempLowHigh[0] || Nodes[i][j][k].T >TempLowHigh[1])
							continue;

						if (Nodes[i][j][k].rad < RadLowHigh[0] || Nodes[i][j][k].rad > RadLowHigh[1])
							continue;

						if (Nodes[i][j][k].grad < GradLowHigh[0] || Nodes[i][j][k].grad > GradLowHigh[1])
							continue;

						// finally draw the point if it passes all the tests:

						glColor3f(Nodes[i][j][k].gray, Nodes[i][j][k].gray, Nodes[i][j][k].gray);
						glVertex3f(Nodes[i][j][k].x, Nodes[i][j][k].y, Nodes[i][j][k].z);
					}
				}
			}
			glEnd();

		}

		else if (GrayOn == 0)
		{
			glPointSize(2);
			glBegin(GL_POINTS);

			for (int i = 0; i < NX; i++)
			{
				if (Nodes[i][0][0].x < XaxisLowHigh[0] || Nodes[i][0][0].x > XaxisLowHigh[1])
					continue;

				for (int j = 0; j < NY; j++)
				{
					if (Nodes[i][j][0].y < YaxisLowHigh[0] || Nodes[i][j][0].y > YaxisLowHigh[1])
						continue;

					for (int k = 0; k < NZ; k++)
					{
						if (Nodes[i][j][k].z < ZaxisLowHigh[0] || Nodes[i][j][k].z > ZaxisLowHigh[1])
							continue;

						if (Nodes[i][j][k].T <TempLowHigh[0] || Nodes[i][j][k].T >TempLowHigh[1])
							continue;
						if (Nodes[i][j][k].rad < RadLowHigh[0] || Nodes[i][j][k].rad > RadLowHigh[1])
							continue;

						if (Nodes[i][j][k].grad < GradLowHigh[0] || Nodes[i][j][k].grad > GradLowHigh[1])
							continue;
						// finally draw the point if it passes all the tests:

						glColor3fv(Nodes[i][j][k].rgb);
						glVertex3f(Nodes[i][j][k].x, Nodes[i][j][k].y, Nodes[i][j][k].z);
					}
				}
			}
			glEnd();
		}
	}

	else
	{
		if (WhichCuttingPlane == 0)
		{
			glShadeModel(GL_SMOOTH);
			glBegin(GL_QUADS);
			if (PlaneXYOn == 1)
			{
				for (int i = 0; i < NX - 1; i++)
				{
					for (int j = 0; j < NY - 1; j++)
					{
						ColorSquare(&PlaneXY[i][j], &PlaneXY[i + 1][j], &PlaneXY[i + 1][j + 1], &PlaneXY[i][j + 1]);
					}
				}


			}

			if (PlaneXZOn == 1)
			{
				for (int i = 0; i < NX - 1; i++)
				{
					for (int j = 0; j < NZ - 1; j++)
					{
						ColorSquare(&PlaneXZ[i][j], &PlaneXZ[i + 1][j], &PlaneXZ[i + 1][j + 1], &PlaneXZ[i][j + 1]);
					}
				}

			}

			if (PlaneYZOn == 1)
			{
				for (int i = 0; i < NY - 1; i++)
				{
					for (int j = 0; j < NZ - 1; j++)
					{
						ColorSquare(&PlaneYZ[i][j], &PlaneYZ[i + 1][j], &PlaneYZ[i + 1][j + 1], &PlaneYZ[i][j + 1]);
					}
				}

			}
			glEnd();

		}

		else if (WhichCuttingPlane == 1)
		{
			glBegin(GL_LINES);

			if (PlaneXYOn == 1)
			{
				for (int i = 0; i < NX - 1; i++)
				{
					for (int j = 0; j < NY - 1; j++)
					{
						for (int Sstar = TEMPMIN; Sstar <= TEMPMAX; Sstar += NsSteps)
						{
							Processquad(&PlaneXY[i][j], &PlaneXY[i + 1][j], &PlaneXY[i + 1][j + 1], &PlaneXY[i][j + 1], Sstar);
						}
					}
				}
			}

			if (PlaneXZOn == 1)
			{
				for (int i = 0; i < NX - 1; i++)
				{
					for (int j = 0; j < NY - 1; j++)
					{
						for (int Sstar = TEMPMIN; Sstar <= TEMPMAX; Sstar += NsSteps)
						{
							Processquad(&PlaneXZ[i][j], &PlaneXZ[i + 1][j], &PlaneXZ[i + 1][j + 1], &PlaneXZ[i][j + 1], Sstar);
						}
					}
				}
			}

			if (PlaneYZOn == 1)
			{
				for (int i = 0; i < NX - 1; i++)
				{
					for (int j = 0; j < NY - 1; j++)
					{
						for (int Sstar = TEMPMIN; Sstar <= TEMPMAX; Sstar += NsSteps)
						{
							Processquad(&PlaneYZ[i][j], &PlaneYZ[i + 1][j], &PlaneYZ[i + 1][j + 1], &PlaneYZ[i][j + 1], Sstar);
						}
					}
				}
			}
			glEnd();
		}

		if (WireSurfaceOn == 1)
		{
			glBegin(GL_LINES);

			for (int i = 0; i < NX; i++)
			{
				for (int y = 0; y < NX - 1; y++)
				{
					for (int z = 0; z < NY - 1; z++)
					{
						Isoquad(&Nodes[i][y][z], &Nodes[i][y + 1][z], &Nodes[i][y + 1][z + 1], &Nodes[i][y][z + 1], WireSstar);
					}
				}
			}

			for (int j = 0; j < NY; j++)
			{
				for (int x = 0; x < NX - 1; x++)
				{
					for (int z = 0; z < NZ - 1; z++)
					{
						Isoquad(&Nodes[x][j][z], &Nodes[x + 1][j][z], &Nodes[x + 1][j][z + 1], &Nodes[x][j][z + 1], WireSstar);
					}
				}
			}
			for (int k = 0; k < NZ; k++)
			{
				for (int x = 0; x < NX - 1; x++)
				{
					for (int y = 0; y < NY - 1; y++)
					{
						Isoquad(&Nodes[x][y][k], &Nodes[x + 1][y][k], &Nodes[x + 1][y + 1][k], &Nodes[x][y + 1][k], WireSstar);
					}
				}
			}
			glEnd();
		}

	}

	glDisable(GL_DEPTH_TEST);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0., 100., 0., 100.);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glColor3f(1., 1., 1.);



	// swap the double-buffered framebuffers:

	glutSwapBuffers();


	// be sure the graphics buffer has been sent:
	// note: be sure to use glFlush( ) here, not glFinish( ) !

	glFlush();
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
	GLUI_Panel *panel, *bigpanel;
	GLUI_RadioGroup *group;
	GLUI_Rotation *rot;
	GLUI_Translation *trans, *scale;
	GLUI_HSlider *slider;
	GLUI_Spinner *spinner;
	char str[256];


	// setup the glui window:

	glutInitWindowPosition(INIT_WINDOW_SIZE + 50, 0);
	Glui = GLUI_Master.create_glui((char *)GLUITITLE);


	Glui->add_statictext((char *)GLUITITLE);
	Glui->add_separator();

	Glui->add_checkbox("Axes", &AxesOn);

	Glui->add_checkbox("Perspective", &WhichProjection);
	Glui->add_checkbox("Grayscale", &GrayOn);
	Glui->add_checkbox("Cutting Plane", &PlaneOn);
	Glui->add_checkbox("Intensity Depth Cue", &DepthCueOn);

	panel = Glui->add_panel("Sliders");
	TempSlider = Glui->add_slider_to_panel(panel, true, GLUI_HSLIDER_FLOAT, TempLowHigh, TEMP, (GLUI_Update_CB)Sliders);
	TempSlider->set_float_limits(TEMPMIN, TEMPMAX);
	TempSlider->set_w(200);       // good slider width
	sprintf(str, TEMPFORMAT, TempLowHigh[0], TempLowHigh[1]);
	TempLabel = Glui->add_statictext_to_panel(panel, str);

	//setup X axis slider
	XSlider = Glui->add_slider_to_panel(panel, true, GLUI_HSLIDER_FLOAT, XaxisLowHigh, XAXIS, (GLUI_Update_CB)Sliders);
	XSlider->set_float_limits(AXISMIN, AXISMAX);
	XSlider->set_w(200);      // good slider width
	sprintf(str, XFORMAT, XaxisLowHigh[0], XaxisLowHigh[1]);
	XLabel = Glui->add_statictext_to_panel(panel, str);

	//setup Y axis slider
	YSlider = Glui->add_slider_to_panel(panel, true, GLUI_HSLIDER_FLOAT, YaxisLowHigh, YAXIS, (GLUI_Update_CB)Sliders);
	YSlider->set_float_limits(AXISMIN, AXISMAX);
	YSlider->set_w(200);      // good slider width
	sprintf(str, YFORMAT, YaxisLowHigh[0], YaxisLowHigh[1]);
	YLabel = Glui->add_statictext_to_panel(panel, str);

	//setup Z axis slider
	ZSlider = Glui->add_slider_to_panel(panel, true, GLUI_HSLIDER_FLOAT, ZaxisLowHigh, ZAXIS, (GLUI_Update_CB)Sliders);
	ZSlider->set_float_limits(AXISMIN, AXISMAX);
	ZSlider->set_w(200);      // good slider width
	sprintf(str, ZFORMAT, ZaxisLowHigh[0], ZaxisLowHigh[1]);
	ZLabel = Glui->add_statictext_to_panel(panel, str);

	//setup Radius Slider
	RadSlider = Glui->add_slider_to_panel(panel, true, GLUI_HSLIDER_FLOAT, RadLowHigh, RAD, (GLUI_Update_CB)Sliders);
	RadSlider->set_float_limits(RADMIN, RADMAX);
	RadSlider->set_w(200);        // good slider width
	sprintf(str, RADFORMAT, RadLowHigh[0], RadLowHigh[1]);
	RadLabel = Glui->add_statictext_to_panel(panel, str);;

	//setup Gradient Slider
	GradSlider = Glui->add_slider_to_panel(panel, true, GLUI_HSLIDER_FLOAT, GradLowHigh, GRAD, (GLUI_Update_CB)Sliders);
	GradSlider->set_float_limits(GRADMIN, GRADMAX);
	GradSlider->set_w(200);       // good slider width
	sprintf(str, GRADFORMAT, GradLowHigh[0], GradLowHigh[1]);
	GradLabel = Glui->add_statictext_to_panel(panel, str);


	bigpanel = Glui->add_panel("Cutting Plane");
	panel = Glui->add_panel_to_panel(bigpanel, "");
	Glui->add_checkbox_to_panel(panel, "XY", &PlaneXYOn, 0, (GLUI_Update_CB)NewPlane);
	Glui->add_column_to_panel(panel, FALSE);
	Glui->add_checkbox_to_panel(panel, "YZ", &PlaneYZOn, 0, (GLUI_Update_CB)NewPlane);
	Glui->add_column_to_panel(panel, FALSE);
	Glui->add_checkbox_to_panel(panel, "XZ", &PlaneXZOn, 0, (GLUI_Update_CB)NewPlane);
	group = Glui->add_radiogroup_to_panel(bigpanel, &WhichCuttingPlane, 0, (GLUI_Update_CB)NewPlane);
	Glui->add_radiobutton_to_group(group, "Shading");
	Glui->add_radiobutton_to_group(group, "Contours");
	spinner = Glui->add_spinner_to_panel(bigpanel, "# contours", GLUI_SPINNER_INT, &NsSteps, 0, (GLUI_Update_CB)NewPlane);
	spinner->set_int_limits(5, 100);
	spinner->set_speed(4.0);
	panel = Glui->add_panel_to_panel(bigpanel, "");
	spinner = Glui->add_spinner_to_panel(panel, "X Plane", GLUI_SPINNER_FLOAT, &PlaneX, 0);
	spinner->set_float_limits(XMIN, XMAX);
	spinner->set_speed(5.0);
	spinner = Glui->add_spinner_to_panel(panel, "Y Plane", GLUI_SPINNER_FLOAT, &PlaneY, 0);
	spinner->set_float_limits(YMIN, YMAX);
	spinner->set_speed(5.0);
	spinner = Glui->add_spinner_to_panel(panel, "Z Plane", GLUI_SPINNER_FLOAT, &PlaneZ, 0);
	spinner->set_float_limits(ZMIN, ZMAX);
	spinner->set_speed(5.0);

	panel = Glui->add_panel("Wireframe Isosurface");
	Glui->add_checkbox_to_panel(panel, "Wireframe Isosurfaces", &WireSurfaceOn, 0, (GLUI_Update_CB)NewWireSurface);
	spinner = Glui->add_spinner_to_panel(panel, "Isovalue", GLUI_SPINNER_FLOAT, &WireSstar, 0, (GLUI_Update_CB)NewWireSurface);
	spinner->set_float_limits(TEMPMIN, TEMPMAX);
	spinner->set_speed(5.0);

	//Glui->add_radiobutton_to_group( group, "Red" );
	//Glui->add_radiobutton_to_group( group, "Yellow" );
	//Glui->add_radiobutton_to_group( group, "Green" );
	//Glui->add_radiobutton_to_group( group, "Cyan" );
	//Glui->add_radiobutton_to_group( group, "Blue" );
	//Glui->add_radiobutton_to_group( group, "Magenta" );
	//Glui->add_radiobutton_to_group( group, "White" );
	//Glui->add_radiobutton_to_group( group, "Black" );

	panel = Glui->add_panel("Object Transformation");

	rot = Glui->add_rotation_to_panel(panel, "Rotation", (float *)RotMatrix);



	// allow the object to be spun via the glui rotation widget:
	rot->set_spin(1.0);


	Glui->add_column_to_panel(panel, GLUIFALSE);
	scale = Glui->add_translation_to_panel(panel, "Scale", GLUI_TRANSLATION_Y, &Scale2);
	scale->set_speed(0.005f);

	Glui->add_column_to_panel(panel, GLUIFALSE);
	trans = Glui->add_translation_to_panel(panel, "Trans XY", GLUI_TRANSLATION_XY, &TransXYZ[0]);
	trans->set_speed(0.05f);

	Glui->add_column_to_panel(panel, GLUIFALSE);
	trans = Glui->add_translation_to_panel(panel, "Trans Z", GLUI_TRANSLATION_Z, &TransXYZ[2]);
	trans->set_speed(0.05f);

	Glui->add_checkbox("Debug", &DebugOn);


	panel = Glui->add_panel("", GLUIFALSE);

	Glui->add_button_to_panel(panel, "Reset", RESET, (GLUI_Update_CB)Buttons);

	Glui->add_column_to_panel(panel, GLUIFALSE);

	Glui->add_button_to_panel(panel, "Quit", QUIT, (GLUI_Update_CB)Buttons);

	// tell glui what graphics window it needs to post a redisplay to:

	Glui->set_main_gfx_window(MainWindow);


	// set the graphics window's idle function if needed:

	GLUI_Master.set_glutIdleFunc(NULL);



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
	glutKeyboardFunc(Keyboard);
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

	// DO NOT SET THE GLUT IDLE FUNCTION HERE !!
	// glutIdleFunc( NULL );
	// let glui take care of it in InitGlui( )


	/*Nodes[NX][NY][NZ].hsv[1] = 1.;
	Nodes[NX][NY][NZ].hsv[2] = 1.;*/
	PointList = glGenLists(1);
	glNewList(PointList, GL_COMPILE);
	glPointSize(0.1); // hardwire this, or set it with a spinner
	glBegin(GL_POINTS);

	for (int i = 0; i < NX; i++)
	{
		for (int j = 0; j < NY; j++)
		{
			for (int k = 0; k < NZ; k++)
			{
				Nodes[i][j][k].x = -1. + 2. * (float)i / (float)(NX - 1);
				Nodes[i][j][k].y = -1. + 2. * (float)j / (float)(NY - 1);
				Nodes[i][j][k].z = -1. + 2. * (float)k / (float)(NZ - 1);
				Nodes[i][j][k].hsv[1] = 1.;
				Nodes[i][j][k].hsv[2] = 1.;

				Nodes[i][j][k].T = Temperature(Nodes[i][j][k].x, Nodes[i][j][k].y, Nodes[i][j][k].z);
				Nodes[i][j][k].hsv[0] = 240. - 240. * (Nodes[i][j][k].T - TEMPMIN) / (TEMPMAX - TEMPMIN);
				Nodes[i][j][k].gray = GRAYMIN + (GRAYMAX - GRAYMIN) * (Nodes[i][j][k].T - TEMPMIN) / (TEMPMAX - TEMPMIN);  //compute the grayscale
				HsvRgb(Nodes[i][j][k].hsv, Nodes[i][j][k].rgb);

				//Radius Calculation
				Nodes[i][j][k].rad = sqrt(SQR(Nodes[i][j][k].x) + SQR(Nodes[i][j][k].y) + SQR(Nodes[i][j][k].z));



			}
		}
	}

	for (int i = 0; i < NX; i++)
	{
		for (int j = 0; j < NY; j++)
		{
			for (int k = 0; k < NZ; k++)
			{
				//Gradient Calculation
				if (i == 0)
					Nodes[i][j][k].dTdx = (Nodes[i + 1][j][k].T - Nodes[i][j][k].T) / (Nodes[i + 1][j][k].x - Nodes[i][j][k].x);
				else if (i == NX - 1)
					Nodes[i][j][k].dTdx = (Nodes[i][j][k].T - Nodes[i - 1][j][k].T) / (Nodes[i][j][k].x - Nodes[i - 1][j][k].x);
				else
					Nodes[i][j][k].dTdx = (Nodes[i + 1][j][k].T - Nodes[i - 1][j][k].T) / (Nodes[i + 1][j][k].x - Nodes[i - 1][j][k].x);

				if (j == 0)
					Nodes[i][j][k].dTdy = (Nodes[i][j + 1][k].T - Nodes[i][j][k].T) / (Nodes[i][j + 1][k].y - Nodes[i][j][k].y);
				else if (j == NY - 1)
					Nodes[i][j][k].dTdy = (Nodes[i][j][k].T - Nodes[i][j - 1][k].T) / (Nodes[i][j][k].y - Nodes[i][j - 1][k].y);
				else
					Nodes[i][j][k].dTdy = (Nodes[i][j + 1][k].T - Nodes[i][j - 1][k].T) / (Nodes[i][j + 1][k].y - Nodes[i][j - 1][k].y);

				if (k == 0)
					Nodes[i][j][k].dTdz = (Nodes[i][j][k + 1].T - Nodes[i][j][k].T) / (Nodes[i][j][k + 1].z - Nodes[i][j][k].z);
				else if (k == NZ - 1)
					Nodes[i][j][k].dTdz = (Nodes[i][j][k].T - Nodes[i][j][k - 1].T) / (Nodes[i][j][k].z - Nodes[i][j][k - 1].z);
				else
					Nodes[i][j][k].dTdz = (Nodes[i][j][k + 1].T - Nodes[i][j][k - 1].T) / (Nodes[i][j][k + 1].z - Nodes[i][j][k - 1].z);

				Nodes[i][j][k].grad = sqrt(SQR(Nodes[i][j][k].dTdx) + SQR(Nodes[i][j][k].dTdy) + SQR(Nodes[i][j][k].dTdz));
			}
		}
	}
	glEnd();
	glEndList();

	XaxisLowHigh[0] = -1.f;
	XaxisLowHigh[1] = 1.f;
	YaxisLowHigh[0] = -1.f;
	YaxisLowHigh[1] = 1.f;
	ZaxisLowHigh[0] = -1.f;
	ZaxisLowHigh[1] = 1.f;
	TempLowHigh[0] = 0.f;
	TempLowHigh[1] = 100.f;
	RadLowHigh[0] = 0.f;
	RadLowHigh[1] = sqrt(3.);
	GradLowHigh[0] = 0.;
	GradLowHigh[1] = 300.f;



}


// initialize the display lists that will not change:
// (a display list is a way to store opengl commands in
//  memory so that they can be played back efficiently at a later time
//  with a call to glCallList( )

void
InitLists()
{
	//    float dx = BOXSIZE / 2.;
	//    float dy = BOXSIZE / 2.;
	//    float dz = BOXSIZE / 2.;
	//    
	// create the object:

	//    BoxList = glGenLists( 1 );
	//    glNewList( BoxList, GL_COMPILE );
	//    
	//    glBegin( GL_QUADS );
	//    
	//    glColor3f( 0., 0., 1. );
	//    glNormal3f( 0., 0.,  1. );
	//              glVertex3f( -dx, -dy,  dz );
	//              glVertex3f(  dx, -dy,  dz );
	//              glVertex3f(  dx,  dy,  dz );
	//              glVertex3f( -dx,  dy,  dz );
	//    
	//    glNormal3f( 0., 0., -1. );
	//              glTexCoord2f( 0., 0. );
	//              glVertex3f( -dx, -dy, -dz );
	//              glTexCoord2f( 0., 1. );
	//              glVertex3f( -dx,  dy, -dz );
	//              glTexCoord2f( 1., 1. );
	//              glVertex3f(  dx,  dy, -dz );
	//              glTexCoord2f( 1., 0. );
	//              glVertex3f(  dx, -dy, -dz );
	//    
	//    glColor3f( 1., 0., 0. );
	//    glNormal3f(  1., 0., 0. );
	//              glVertex3f(  dx, -dy,  dz );
	//              glVertex3f(  dx, -dy, -dz );
	//              glVertex3f(  dx,  dy, -dz );
	//              glVertex3f(  dx,  dy,  dz );
	//    
	//    glNormal3f( -1., 0., 0. );
	//              glVertex3f( -dx, -dy,  dz );
	//              glVertex3f( -dx,  dy,  dz );
	//              glVertex3f( -dx,  dy, -dz );
	//              glVertex3f( -dx, -dy, -dz );
	//    
	//    glColor3f( 0., 1., 0. );
	//    glNormal3f( 0.,  1., 0. );
	//              glVertex3f( -dx,  dy,  dz );
	//              glVertex3f(  dx,  dy,  dz );
	//              glVertex3f(  dx,  dy, -dz );
	//              glVertex3f( -dx,  dy, -dz );
	//    
	//    glNormal3f( 0., -1., 0. );
	//              glVertex3f( -dx, -dy,  dz );
	//              glVertex3f( -dx, -dy, -dz );
	//              glVertex3f(  dx, -dy, -dz );
	//              glVertex3f(  dx, -dy,  dz );
	//    
	//    glEnd( );
	//    
	//    glEndList( );
	//    
	//

	// create the axes:
	AxesList = glGenLists(1);
	glNewList(AxesList, GL_COMPILE);
	glLineWidth(AXES_WIDTH);
	Axes(1.5);
	glLineWidth(1.);
	glEndList();
}


// the keyboard callback:

void
Keyboard(unsigned char c, int x, int y)
{
	if (DebugOn != 0)
		fprintf(stderr, "Keyboard: '%c' (0x%0x)\n", c, c);

	switch (c)
	{
	case 'o':
	case 'O':
		WhichProjection = ORTHO;
		break;

	case 'p':
	case 'P':
		WhichProjection = PERSP;
		break;

	case 'q':
	case 'Q':
	case ESCAPE:
		Buttons(QUIT);    // will not return here
		break;          // happy compiler

	case 'r':
	case 'R':
		LeftButton = ROTATE;
		break;

	case 's':
	case 'S':
		LeftButton = SCALE;
		break;

	default:
		fprintf(stderr, "Don't know what to do with keyboard hit: '%c' (0x%0x)\n", c, c);
	}


	// synchronize the GLUI display with the variables:

	Glui->sync_live();


	// force a call to Display( ):

	glutSetWindow(MainWindow);
	glutPostRedisplay();
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
	WhichColor = WHITE;
	WhichProjection = PERSP;
	Xrot = Yrot = 0.;
	TransXYZ[0] = TransXYZ[1] = TransXYZ[2] = 0.;

	RotMatrix[0][1] = RotMatrix[0][2] = RotMatrix[0][3] = 0.;
	RotMatrix[1][0] = RotMatrix[1][2] = RotMatrix[1][3] = 0.;
	RotMatrix[2][0] = RotMatrix[2][1] = RotMatrix[2][3] = 0.;
	RotMatrix[3][0] = RotMatrix[3][1] = RotMatrix[3][3] = 0.;
	RotMatrix[0][0] = RotMatrix[1][1] = RotMatrix[2][2] = RotMatrix[3][3] = 1.;

	//Reset graph
	TempLowHigh[0] = TEMPMIN;
	TempLowHigh[1] = TEMPMAX;
	XaxisLowHigh[0] = AXISMIN;
	XaxisLowHigh[1] = AXISMAX;
	YaxisLowHigh[0] = AXISMIN;
	YaxisLowHigh[1] = AXISMAX;
	ZaxisLowHigh[0] = AXISMIN;
	ZaxisLowHigh[1] = AXISMAX;
	RadLowHigh[0] = RADMIN;
	RadLowHigh[1] = RADMAX;
	GradLowHigh[0] = GRADMIN;
	GradLowHigh[1] = GRADMAX;
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

//function to compute 2D Interpolated Color Plots
void ColorSquare(struct nodeTwoD *p0, struct nodeTwoD *p1, struct nodeTwoD *p2, struct nodeTwoD *p3)
{


	glColor3fv(p0->rgb);
	glVertex3f(p0->x, p0->y, p0->z);

	glColor3fv(p1->rgb);
	glVertex3f(p1->x, p1->y, p1->z);

	glColor3fv(p2->rgb);
	glVertex3f(p2->x, p2->y, p2->z);

	glColor3fv(p3->rgb);
	glVertex3f(p3->x, p3->y, p3->z);

}

//function to compute contour part
void Processquad(struct nodeTwoD *p0, struct nodeTwoD *p1, struct nodeTwoD *p2, struct nodeTwoD *p3, int Sstar)
{
	float Tstar = 0;
	int n = 0;
	float plot[4][3];
	float hsv[3] = { 1.f };
	float rgb[3] = { 0.f };

	if (p0->T != p1->T)
	{
		Tstar = (Sstar - p0->T) / (p1->T - p0->T);

		if (Tstar >= 0 && Tstar <= 1)
		{
			plot[n][0] = p0->x + Tstar * (p1->x - p0->x);	//x value
			plot[n][1] = p0->y + Tstar * (p1->y - p0->y);	//y value
			plot[n][2] = p0->z + Tstar * (p1->z - p0->z);	//z value
			n++;
		}

	}

	if (p2->T != p1->T)
	{
		Tstar = (Sstar - p1->T) / (p2->T - p1->T);
		if (Tstar >= 0 && Tstar <= 1)
		{
			plot[n][0] = p1->x + Tstar * (p2->x - p1->x);
			plot[n][1] = p1->y + Tstar * (p2->y - p1->y);
			plot[n][2] = p1->z + Tstar * (p2->z - p1->z);
			n++;
		}
	}

	if (p3->T != p2->T)
	{
		Tstar = Tstar = (Sstar - p2->T) / (p3->T - p2->T);
		if (Tstar >= 0 && Tstar <= 1)
		{
			plot[n][0] = p2->x + Tstar * (p3->x - p2->x);
			plot[n][1] = p2->y + Tstar * (p3->y - p2->y);
			plot[n][2] = p2->z + Tstar * (p3->z - p2->z);
			n++;
		}
	}

	if (p3->T != p0->T)
	{
		Tstar = Tstar = (Sstar - p3->T) / (p0->T - p3->T);
		if (Tstar >= 0 && Tstar <= 1)
		{
			plot[n][0] = p3->x + Tstar * (p0->x - p3->x);
			plot[n][1] = p3->y + Tstar * (p0->y - p3->y);
			plot[n][2] = p3->z + Tstar * (p0->z - p3->z);
			n++;
		}

	}

	if (n == 2)
	{
		hsv[0] = 240. - 240. * (Sstar - TEMPMIN) / (TEMPMAX - TEMPMIN);
		hsv[1] = 1.;
		hsv[2] = 1.;
		HsvRgb(hsv, rgb);
		glColor3fv(rgb);
		glVertex3f(plot[0][0], plot[0][1], plot[0][2]);
		glVertex3f(plot[1][0], plot[1][1], plot[1][2]);
	}
}

void Isoquad(struct node *p0, struct node *p1, struct node *p2, struct node *p3, int Sstar)
{
	float Tstar = 0;
	int n = 0;
	float plot[4][3];
	float hsv[3] = { 1.f };
	float rgb[3] = { 0.f };

	if (p0->T != p1->T)
	{
		Tstar = (Sstar - p0->T) / (p1->T - p0->T);

		if (Tstar >= 0 && Tstar <= 1)
		{
			plot[n][0] = p0->x + Tstar * (p1->x - p0->x);	//x value
			plot[n][1] = p0->y + Tstar * (p1->y - p0->y);	//y value
			plot[n][2] = p0->z + Tstar * (p1->z - p0->z);	//z value
			n++;
		}

	}

	if (p2->T != p1->T)
	{
		Tstar = (Sstar - p1->T) / (p2->T - p1->T);
		if (Tstar >= 0 && Tstar <= 1)
		{
			plot[n][0] = p1->x + Tstar * (p2->x - p1->x);
			plot[n][1] = p1->y + Tstar * (p2->y - p1->y);
			plot[n][2] = p1->z + Tstar * (p2->z - p1->z);
			n++;
		}
	}

	if (p3->T != p2->T)
	{
		 Tstar = (Sstar - p2->T) / (p3->T - p2->T);
		if (Tstar >= 0 && Tstar <= 1)
		{
			plot[n][0] = p2->x + Tstar * (p3->x - p2->x);
			plot[n][1] = p2->y + Tstar * (p3->y - p2->y);
			plot[n][2] = p2->z + Tstar * (p3->z - p2->z);
			n++;
		}
	}

	if (p3->T != p0->T)
	{
		Tstar = (Sstar - p3->T) / (p0->T - p3->T);
		if (Tstar >= 0 && Tstar <= 1)
		{
			plot[n][0] = p3->x + Tstar * (p0->x - p3->x);
			plot[n][1] = p3->y + Tstar * (p0->y - p3->y);
			plot[n][2] = p3->z + Tstar * (p0->z - p3->z);
			n++;
		}

	}
	//if the number of intersections is 2
	if (n == 2)
	{
		hsv[0] = 240. - 240. * (Sstar - TEMPMIN) / (TEMPMAX - TEMPMIN);
		hsv[1] = 1.;
		hsv[2] = 1.;
		HsvRgb(hsv, rgb);
		glColor3fv(rgb);
		glVertex3f(plot[0][0], plot[0][1], plot[0][2]);
		glVertex3f(plot[1][0], plot[1][1], plot[1][2]);
	}
}

// function to convert HSV to RGB
//
// Reference:  Foley, van Dam, Feiner, Hughes,
//      "Computer Graphics Principles and Practices,"

void
HsvRgb(float hsv[3], float rgb[3])
{
	float r, g, b;          // red, green, blue

	// guarantee valid input:

	float h = hsv[0] / 60.;
	while (h >= 6.)    h -= 6.;
	while (h <  0.)    h += 6.;

	float s = hsv[1];
	if (s < 0.)
		s = 0.;
	if (s > 1.)
		s = 1.;

	float v = hsv[2];
	if (v < 0.)
		v = 0.;
	if (v > 1.)
		v = 1.;


	// if sat==0, then is a gray:

	if (s == 0.0)
	{
		rgb[0] = rgb[1] = rgb[2] = v;
		return;
	}


	// get an rgb from the hue itself:

	float i = floor(h);
	float f = h - i;
	float p = v * (1. - s);
	float q = v * (1. - s*f);
	float t = v * (1. - (s * (1. - f)));

	switch ((int)i)
	{
	case 0:
		r = v;  g = t;  b = p;
		break;

	case 1:
		r = q;  g = v;  b = p;
		break;

	case 2:
		r = p;  g = v;  b = t;
		break;

	case 3:
		r = p;  g = q;  b = v;
		break;

	case 4:
		r = t;  g = p;  b = v;
		break;

	case 5:
		r = v;  g = p;  b = q;
		break;
	}


	rgb[0] = r;
	rgb[1] = g;
	rgb[2] = b;
}