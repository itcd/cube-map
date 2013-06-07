/** @file
* A bump mapping demo
*
* reference
* Bump Mapping using GLSL
* http://www.ozone3d.net/tutorials/bump_mapping.php
*/

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

#include <GL/glew.h>
#include <GL/freeglut.h>

#include "textfile.h"
#include "tga.h"		/* simple TGA image file loader */
#include "trackball.h"	/* trackball quaternion routines */

GLuint v,f,f2,p;
float lpos[4] = {1,0.5,1,0};

GLuint color_map = 1, normal_map = 2;
GLuint loc_lighting_method, loc_disable_textures;

int spinning = 0, moving = 0;
int beginx, beginy;
int W = 500, H = 500;
float curquat[4];
float lastquat[4];

void draw_cube()
{
	glBegin(GL_QUADS);

	// Front Face
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  1.0f);  // Bottom Left Of The Texture and Quad
	glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  1.0f);  // Bottom Right Of The Texture and Quad
	glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f,  1.0f);  // Top Right Of The Texture and Quad
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f,  1.0f);  // Top Left Of The Texture and Quad

	// Back Face
	glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f, -1.0f);  // Bottom Right Of The Texture and Quad
	glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f,  1.0f, -1.0f);  // Top Right Of The Texture and Quad
	glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f,  1.0f, -1.0f);  // Top Left Of The Texture and Quad
	glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f, -1.0f);  // Bottom Left Of The Texture and Quad

	// Top Face
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f, -1.0f);  // Top Left Of The Texture and Quad
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f,  1.0f,  1.0f);  // Bottom Left Of The Texture and Quad
	glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f,  1.0f,  1.0f);  // Bottom Right Of The Texture and Quad
	glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f, -1.0f);  // Top Right Of The Texture and Quad

	// Bottom Face
	glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f, -1.0f, -1.0f);  // Top Right Of The Texture and Quad
	glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f, -1.0f, -1.0f);  // Top Left Of The Texture and Quad
	glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  1.0f);  // Bottom Left Of The Texture and Quad
	glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  1.0f);  // Bottom Right Of The Texture and Quad

	// Right face
	glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f, -1.0f, -1.0f);  // Bottom Right Of The Texture and Quad
	glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f, -1.0f);  // Top Right Of The Texture and Quad
	glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f,  1.0f,  1.0f);  // Top Left Of The Texture and Quad
	glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  1.0f);  // Bottom Left Of The Texture and Quad

	// Left Face
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f, -1.0f);  // Bottom Left Of The Texture and Quad
	glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  1.0f);  // Bottom Right Of The Texture and Quad
	glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f,  1.0f,  1.0f);  // Top Right Of The Texture and Quad
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f, -1.0f);  // Top Left Of The Texture and Quad

	glEnd();
}

void display(void)
{
	GLfloat m[4][4];

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glPushMatrix();
	build_rotmatrix(m, curquat);
	glMultMatrixf(&m[0][0]);

	draw_cube();

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

/// Sets a uniform texture parameter
GLuint add_texture_uniform(GLuint program, const char* name, int number, GLenum target, GLuint texture) 
{
	GLuint location = glGetUniformLocation(program, name);
	glUniform1i(location, number);
	glActiveTexture(GL_TEXTURE0 + number);
	glBindTexture(target, texture);

	// restore active texture unit to GL_TEXTURE0
	glActiveTexture(GL_TEXTURE0);
	return location;
}

void setShaders() {

	char *vs = NULL,*fs = NULL;

	v = glCreateShader(GL_VERTEX_SHADER);
	f = glCreateShader(GL_FRAGMENT_SHADER);
	f2 = glCreateShader(GL_FRAGMENT_SHADER);

	vs = textFileRead("std.vert");
	fs = textFileRead("bumpmap.frag");

	const char * ff = fs;
	const char * vv = vs;

	glShaderSource(v, 1, &vv,NULL);
	glShaderSource(f, 1, &ff,NULL);

	free(vs);free(fs);

	glCompileShader(v);
	glCompileShader(f);

	p = glCreateProgram();
	glAttachShader(p,f);
	glAttachShader(p,v);

	glLinkProgram(p);
	glUseProgram(p);

	loc_lighting_method = glGetUniformLocation(p, "lightingMethod");
	loc_lighting_method = glGetUniformLocation(p, "disableTextures");
	glUniform1i(loc_lighting_method, 0);
	glUniform1i(loc_disable_textures, 0);

	add_texture_uniform(p, "colorMap", 1, GL_TEXTURE_2D, color_map);
	add_texture_uniform(p, "normalMap", 2, GL_TEXTURE_2D, normal_map);
	add_texture_uniform(p, "glossMap", 3, GL_TEXTURE_2D, color_map);
}

void init()
{
	glewInit();
	if (glewIsSupported("GL_VERSION_2_0"))
		printf("Ready for OpenGL 2.0\n");
	else {
		printf("OpenGL 2.0 not supported\n");
		exit(1);
	}

	if (!loadTGA ("color_map_512.tga", color_map))
		printf ("color_map_512.tga not found!\n");
	if (!loadTGA ("normal_map_512.tga", normal_map))
		printf ("normal_map_512.tga not found!\n");

	setShaders();
}

int main(int argc, char **argv)
{
	glutInitWindowSize(W, H);
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutCreateWindow("bump mapping demo");

	init();

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

	trackball(curquat, 0.0, 0.0, 0.0, 0.0);

	glutMainLoop();
	return 0;
}
