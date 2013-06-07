/** @file
* A cube map demo adapted from
* OpenGL Cube Map Texturing
* http://origin-developer.nvidia.com/object/cube_map_ogl_tutorial.html
*
* Creating a Cubemap Texture
* It's best to set the wrap mode to GL_CLAMP_TO_EDGE and not the other formats.
* Don't forget to define all 6 faces else the texture is considered incomplete.
* Don't forget to setup GL_TEXTURE_WRAP_R because cubemaps require 3D texture coordinates.
* http://www.opengl.org/wiki/Common_Mistakes
*/

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

#include <GL/glew.h>
#include <GL/freeglut.h>

#include "tga.h"		/* simple TGA image file loader */
#include "trackball.h"	/* trackball quaternion routines */

static GLenum faceTarget[6] = {
	GL_TEXTURE_CUBE_MAP_POSITIVE_X,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
	GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
	GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
};

/* Pre-generated cube map images. */
char *faceFile[6] = {
	"cm_left.tga", 
	"cm_right.tga", 
	"cm_top.tga", 
	"cm_bottom.tga", 
	"cm_back.tga", 
	"cm_front.tga", 
};

int hasTextureLodBias = 0;

int mode = GL_REFLECTION_MAP;
int wrap = GL_CLAMP_TO_EDGE;
bool mipmaps = true;

float lodBias = 0.0;

int spinning = 0, moving = 0;
int beginx, beginy;
int W = 500, H = 500;
float curquat[4];
float lastquat[4];

void loadFace(GLenum target, char *filename)
{
	FILE *file;
	gliGenericImage *image;

	file = fopen(filename, "rb");
	if (file == NULL) {
		printf("could not open \"%s\"\n", filename);
		exit(1);
	}
	image = gliReadTGA(file, filename);
	fclose(file);

	if (mipmaps) {
		gluBuild2DMipmaps(target, image->components,
			image->width, image->height,
			image->format, GL_UNSIGNED_BYTE, image->pixels);
	} else {
		glTexImage2D(target, 0, image->components,
			image->width, image->height, 0,
			image->format, GL_UNSIGNED_BYTE, image->pixels);
	}
}

void updateTexgen(void)
{
	assert(mode == GL_NORMAL_MAP || mode == GL_REFLECTION_MAP);
	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, mode);
	glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, mode);
	glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, mode);
}

void updateWrap(void)
{
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, wrap);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, wrap);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, wrap);
}

void makeCubeMap(void)
{
	int i;

	for (i=0; i<6; i++) {
		loadFace(faceTarget[i], faceFile[i]);
	}
	if (mipmaps) {
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER,
			GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	} else {
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	glEnable(GL_TEXTURE_CUBE_MAP);

	updateTexgen();
	updateWrap();

	glEnable(GL_TEXTURE_GEN_S);
	glEnable(GL_TEXTURE_GEN_T);
	glEnable(GL_TEXTURE_GEN_R);
}

void display(void)
{
	GLfloat m[4][4];

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glPushMatrix();
	build_rotmatrix(m, curquat);
	glMultMatrixf(&m[0][0]);

	glutSolidTeapot(1);

	glPopMatrix();

	glutSwapBuffers();
}

void reshape(int w, int h)
{
	glViewport(0, 0, w, h);
	W = w;
	H = h;
}

void keyboard(unsigned char c, int x, int y)
{
	switch (c) {
	case 27:
		exit(0);
		break;
	case ' ':
		if (mode == GL_REFLECTION_MAP) {
			mode = GL_NORMAL_MAP;
		} else {
			mode = GL_REFLECTION_MAP;
		}
		updateTexgen();
		glutPostRedisplay();
		break;
	case 'c':
		if (wrap == GL_REPEAT) {
			wrap = GL_CLAMP_TO_EDGE;
		} else {
			wrap = GL_REPEAT;
		}
		updateWrap();
		glutPostRedisplay();
		break;
	case 'm':
		glMatrixMode(GL_TEXTURE);
		glScalef(-1,-1,-1);
		glMatrixMode(GL_MODELVIEW);
		glutPostRedisplay();
		break;
	case 'a':
		if (hasTextureLodBias) {
			lodBias += 0.05;
			if (lodBias > 5.0) {
				lodBias = 5.0;
			}
			glTexEnvf(GL_TEXTURE_FILTER_CONTROL,
				GL_TEXTURE_LOD_BIAS, lodBias);
			glutPostRedisplay();    
		}
		break;
	case 'z':
		if (hasTextureLodBias) {
			lodBias -= 0.05;
			if (lodBias < 0.0) {
				lodBias = 0.0;
			}
			glTexEnvf(GL_TEXTURE_FILTER_CONTROL,
				GL_TEXTURE_LOD_BIAS, lodBias);
			glutPostRedisplay();    
		}
		break;
	}
}

void mouse(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		spinning = 0;
		glutIdleFunc(NULL);
		moving = 1;
		beginx = x;
		beginy = y;
	}
	if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
		moving = 0;
	}
}

void animate(void)
{
	add_quats(lastquat, curquat, curquat);
	glutPostRedisplay();
}

void motion(int x, int y)
{
	if (moving) {
		trackball(lastquat,
			(2.0 * beginx - W) / W,
			(H - 2.0 * beginy) / H,
			(2.0 * x - W) / W,
			(H - 2.0 * y) / H
			);
		beginx = x;
		beginy = y;
		spinning = 1;
		glutIdleFunc(animate);
	}
}

int main(int argc, char **argv)
{
	int i;

	glutInitWindowSize(W, H);
	glutInit(&argc, argv);
	for (i=1; i<argc; i++) {
		if (!strcmp(argv[i], "-nomipmap")) {
			mipmaps = false;
		}
		if (!strcmp(argv[i], "-v")) {
			gliVerbose(1);
		}
	}
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutCreateWindow("cubemap demo");

	glewInit();

	hasTextureLodBias = glutExtensionSupported("GL_EXT_texture_lod_bias");

	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective( /* field of view in degree */ 40.0,
		/* aspect ratio */ 1.0,
		/* Z near */ 1.0, /* Z far */ 10.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(0.0, 0.0, 5.0,  /* eye is at (0,0,5) */
		0.0, 0.0, 0.0,      /* center is at (0,0,0) */
		0.0, 1.0, 0.);      /* up is in positive Y direction */

	glEnable(GL_DEPTH_TEST);

	makeCubeMap();

	trackball(curquat, 0.0, 0.0, 0.0, 0.0);

	glutMainLoop();
	return 0;
}
