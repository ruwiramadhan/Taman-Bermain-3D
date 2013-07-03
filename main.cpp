#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#ifdef __APPLE__
#include <OpenGL/OpenGL.h>
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#include <GL/glu.h>
#include <GL/gl.h>
#include "imageloader.h"
#include "vec3f.h"
#endif

static GLfloat spin, spin2 = 0.0;
float angle = 0;
float ert=0.0, yui=0.0;
float awal=0.0, akhir=0.0, atas=0.0, bawah=0.0;
float awal2=0.0, akhir2=0.0, atas2=0.0, bawah2=0.0;
float sudut=0.0;
float rotasi=0.0, rotasi2=0.0, rotasi3=0.0, putar=0.0;
float geser=0.0;



using namespace std;

float lastx, lasty;
GLint stencilBits;
static int viewx = 50;
static int viewy = 24;
static int viewz = 80;

float rot = 0;




//train 2D
//class untuk terain 2D
class Terrain {
private:
	int w; //Width
	int l; //Length
	float** hs; //Heights
	Vec3f** normals;
	bool computedNormals; //Whether normals is up-to-date
public:
	Terrain(int w2, int l2) {
		w = w2;
		l = l2;

		hs = new float*[l];
		for (int i = 0; i < l; i++) {
			hs[i] = new float[w];
		}

		normals = new Vec3f*[l];
		for (int i = 0; i < l; i++) {
			normals[i] = new Vec3f[w];
		}

		computedNormals = false;
	}

	~Terrain() {
		for (int i = 0; i < l; i++) {
			delete[] hs[i];
		}
		delete[] hs;

		for (int i = 0; i < l; i++) {
			delete[] normals[i];
		}
		delete[] normals;
	}

	int width() {
		return w;
	}

	int length() {
		return l;
	}

	//Sets the height at (x, z) to y
	void setHeight(int x, int z, float y) {
		hs[z][x] = y;
		computedNormals = false;
	}

	//Returns the height at (x, z)
	float getHeight(int x, int z) {
		return hs[z][x];
	}

	//Computes the normals, if they haven't been computed yet
	void computeNormals() {
		if (computedNormals) {
			return;
		}

		//Compute the rough version of the normals
		Vec3f** normals2 = new Vec3f*[l];
		for (int i = 0; i < l; i++) {
			normals2[i] = new Vec3f[w];
		}

		for (int z = 0; z < l; z++) {
			for (int x = 0; x < w; x++) {
				Vec3f sum(0.0f, 0.0f, 0.0f);

				Vec3f out;
				if (z > 0) {
					out = Vec3f(0.0f, hs[z - 1][x] - hs[z][x], -1.0f);
				}
				Vec3f in;
				if (z < l - 1) {
					in = Vec3f(0.0f, hs[z + 1][x] - hs[z][x], 1.0f);
				}
				Vec3f left;
				if (x > 0) {
					left = Vec3f(-1.0f, hs[z][x - 1] - hs[z][x], 0.0f);
				}
				Vec3f right;
				if (x < w - 1) {
					right = Vec3f(1.0f, hs[z][x + 1] - hs[z][x], 0.0f);
				}

				if (x > 0 && z > 0) {
					sum += out.cross(left).normalize();
				}
				if (x > 0 && z < l - 1) {
					sum += left.cross(in).normalize();
				}
				if (x < w - 1 && z < l - 1) {
					sum += in.cross(right).normalize();
				}
				if (x < w - 1 && z > 0) {
					sum += right.cross(out).normalize();
				}

				normals2[z][x] = sum;
			}
		}

		//Smooth out the normals
		const float FALLOUT_RATIO = 0.5f;
		for (int z = 0; z < l; z++) {
			for (int x = 0; x < w; x++) {
				Vec3f sum = normals2[z][x];

				if (x > 0) {
					sum += normals2[z][x - 1] * FALLOUT_RATIO;
				}
				if (x < w - 1) {
					sum += normals2[z][x + 1] * FALLOUT_RATIO;
				}
				if (z > 0) {
					sum += normals2[z - 1][x] * FALLOUT_RATIO;
				}
				if (z < l - 1) {
					sum += normals2[z + 1][x] * FALLOUT_RATIO;
				}

				if (sum.magnitude() == 0) {
					sum = Vec3f(0.0f, 1.0f, 0.0f);
				}
				normals[z][x] = sum;
			}
		}

		for (int i = 0; i < l; i++) {
			delete[] normals2[i];
		}
		delete[] normals2;

		computedNormals = true;
	}

	//Returns the normal at (x, z)
	Vec3f getNormal(int x, int z) {
		if (!computedNormals) {
			computeNormals();
		}
		return normals[z][x];
	}
};
//end class


void initRendering() {
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_NORMALIZE);
	glShadeModel(GL_SMOOTH);
}

//Loads a terrain from a heightmap.  The heights of the terrain range from
//-height / 2 to height / 2.
//load terain di procedure inisialisasi
Terrain* loadTerrain(const char* filename, float height) {
	Image* image = loadBMP(filename);
	Terrain* t = new Terrain(image->width, image->height);
	for (int y = 0; y < image->height; y++) {
		for (int x = 0; x < image->width; x++) {
			unsigned char color = (unsigned char) image->pixels[3 * (y
					* image->width + x)];
			float h = height * ((color / 255.0f) - 0.5f);
			t->setHeight(x, y, h);
		}
	}

	delete image;
	t->computeNormals();
	return t;
}

float _angle = 60.0f;
//buat tipe data terain
Terrain* _terrain;
//Terrain* _terrainTanah;
Terrain* _terrainAir;

const GLfloat light_ambient[] = { 0.3f, 0.3f, 0.3f, 1.0f };
const GLfloat light_diffuse[] = { 0.7f, 0.7f, 0.7f, 1.0f };
const GLfloat light_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
const GLfloat light_position[] = { 1.0f, 1.0f, 1.0f, 1.0f };

const GLfloat light_ambient2[] = { 0.3f, 0.3f, 0.3f, 0.0f };
const GLfloat light_diffuse2[] = { 0.3f, 0.3f, 0.3f, 0.0f };

const GLfloat mat_ambient[] = { 0.8f, 0.8f, 0.8f, 1.0f };
const GLfloat mat_diffuse[] = { 0.8f, 0.8f, 0.8f, 1.0f };
const GLfloat mat_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
const GLfloat high_shininess[] = { 100.0f };

void cleanup() {
	delete _terrain;
	//delete _terrainTanah;
}

//untuk di display
void drawSceneTanah(Terrain *terrain, GLfloat r, GLfloat g, GLfloat b) {
	//	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	/*
	 glMatrixMode(GL_MODELVIEW);
	 glLoadIdentity();
	 glTranslatef(0.0f, 0.0f, -10.0f);
	 glRotatef(30.0f, 1.0f, 0.0f, 0.0f);
	 glRotatef(-_angle, 0.0f, 1.0f, 0.0f);

	 GLfloat ambientColor[] = {0.4f, 0.4f, 0.4f, 1.0f};
	 glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientColor);

	 GLfloat lightColor0[] = {0.6f, 0.6f, 0.6f, 1.0f};
	 GLfloat lightPos0[] = {-0.5f, 0.8f, 0.1f, 0.0f};
	 glLightfv(GL_LIGHT0, GL_DIFFUSE, lightColor0);
	 glLightfv(GL_LIGHT0, GL_POSITION, lightPos0);
	 */
	float scale = 500.0f / max(terrain->width() - 1, terrain->length() - 1);
	glScalef(scale, scale, scale);
	glTranslatef(-(float) (terrain->width() - 1) / 2, 0.0f,
			-(float) (terrain->length() - 1) / 2);

	glColor3f(r, g, b);
	for (int z = 0; z < terrain->length() - 1; z++) {
		//Makes OpenGL draw a triangle at every three consecutive vertices
		glBegin(GL_TRIANGLE_STRIP);
		for (int x = 0; x < terrain->width(); x++) {
			Vec3f normal = terrain->getNormal(x, z);
			glNormal3f(normal[0], normal[1], normal[2]);
			glVertex3f(x, terrain->getHeight(x, z), z);
			normal = terrain->getNormal(x, z + 1);
			glNormal3f(normal[0], normal[1], normal[2]);
			glVertex3f(x, terrain->getHeight(x, z + 1), z + 1);
		}
		glEnd();
	}

}

unsigned int LoadTextureFromBmpFile(char *filename);

//unit ruang
void cylinder()
{
     GLUquadricObj *p = gluNewQuadric();

//tutup atas
glPushMatrix();
glTranslatef(0,0,0);
gluQuadricDrawStyle(p, GLU_FILL);
gluQuadricOrientation(p, GLU_INSIDE);
gluDisk(p, 0, 1, 360, 1);
glPopMatrix();

glPushMatrix();
glTranslatef(0,0,0);
gluQuadricDrawStyle(p, GLU_FILL);
gluQuadricOrientation(p, GLU_OUTSIDE);
gluDisk(p, 0, 1, 360, 1);
glPopMatrix();

//tabung
glPushMatrix();
glTranslatef(0,0,0);
gluQuadricDrawStyle(p, GLU_FILL);
gluQuadricOrientation(p, GLU_INSIDE);
gluCylinder(p, 1, 1,1, 360, 1);
glPopMatrix();

glPushMatrix();
glTranslatef(0,0,0);
gluQuadricDrawStyle(p, GLU_FILL);
gluQuadricOrientation(p, GLU_OUTSIDE);
gluCylinder(p, 1, 1,1, 360, 1);
glPopMatrix();

//tutup bawah
glPushMatrix();
glTranslatef(0,0,1);
gluQuadricDrawStyle(p, GLU_FILL);
gluQuadricOrientation(p, GLU_INSIDE);
gluDisk(p, 0, 1, 360, 1);
glPopMatrix();

glPushMatrix();
glTranslatef(0,0,1);
gluQuadricDrawStyle(p, GLU_FILL);
gluQuadricOrientation(p, GLU_OUTSIDE);
gluDisk(p, 0, 1, 360, 1);
glPopMatrix();

}

//BIANGLALA
void bianglala()
{
GLUquadricObj *p = gluNewQuadric();

//lingkar utama
glPushMatrix();
glTranslatef(0,10,0);
glRotatef(90,1,0,0);
glutSolidTorus(0.2,10,360,360);
glPopMatrix();

//lingkar dalam
glPushMatrix();
glTranslatef(0,10,0);
glRotatef(90,1,0,0);
gluQuadricDrawStyle(p, GLU_FILL);
gluQuadricOrientation(p, GLU_INSIDE);
gluDisk(p, 4, 5, 360, 1);
glPopMatrix();

glPushMatrix();
glTranslatef(0,10,0);
glRotatef(90,1,0,0);
gluQuadricDrawStyle(p, GLU_FILL);
gluQuadricOrientation(p, GLU_OUTSIDE);
gluDisk(p, 4, 5, 360, 1);
glPopMatrix();

//tiang horizon
glPushMatrix();
glTranslatef(5,10,0);
glScalef(10,0.5,0.5);
glutSolidCube(1);
glPopMatrix();

glPushMatrix();
glTranslatef(-5,10,0);
glScalef(10,0.5,0.5);
glutSolidCube(1);
glPopMatrix();

glPushMatrix();
glTranslatef(0,10,5);
glRotatef(90,0,1,0);
glScalef(10,0.5,0.5);
glutSolidCube(1);
glPopMatrix();

glPushMatrix();
glTranslatef(0,10,-5);
glRotatef(270,0,1,0);
glScalef(10,0.5,0.5);
glutSolidCube(1);
glPopMatrix();

glPushMatrix();
glTranslatef(-2.5,10,-2.5);
glRotatef(135,0,1,0);
glScalef(13,0.5,0.5);
glutSolidCube(1);
glPopMatrix();

glPushMatrix();
glTranslatef(2.5,10,-2.5);
glRotatef(45,0,1,0);
glScalef(13,0.5,0.5);
glutSolidCube(1);
glPopMatrix();

glPushMatrix();
glTranslatef(-2.5,10,2.5);
glRotatef(225,0,1,0);
glScalef(13,0.5,0.5);
glutSolidCube(1);
glPopMatrix();

glPushMatrix();
glTranslatef(2.5,10,2.5);
glRotatef(135,0,1,0);
glScalef(13,0.5,0.5);
glutSolidCube(1);
glPopMatrix();
}

void tiangbianglala()
{
//tiang kecil
glPushMatrix();
glTranslatef(7,11,7);
glRotatef(90,0,0,1);
glScalef(2,0.5,0.5);
glutSolidCube(1);
glPopMatrix();

glPushMatrix();
glTranslatef(-7,11,7);
glRotatef(90,0,0,1);
glScalef(2,0.5,0.5);
glutSolidCube(1);
glPopMatrix();

glPushMatrix();
glTranslatef(7,11,-7);
glRotatef(90,0,0,1);
glScalef(2,0.5,0.5);
glutSolidCube(1);
glPopMatrix();

glPushMatrix();
glTranslatef(-7,11,-7);
glRotatef(90,0,0,1);
glScalef(2,0.5,0.5);
glutSolidCube(1);
glPopMatrix();

glPushMatrix();
glTranslatef(10,11,0);
glRotatef(90,0,0,1);
glScalef(2,0.5,0.5);
glutSolidCube(1);
glPopMatrix();

glPushMatrix();
glTranslatef(-10,11,0);
glRotatef(90,0,0,1);
glScalef(2,0.5,0.5);
glutSolidCube(1);
glPopMatrix();

glPushMatrix();
glTranslatef(0,11,10);
glRotatef(90,0,0,1);
glScalef(2,0.5,0.5);
glutSolidCube(1);
glPopMatrix();

glPushMatrix();
glTranslatef(0,11,-10);
glRotatef(90,0,0,1);
glScalef(2,0.5,0.5);
glutSolidCube(1);
glPopMatrix();
}

void duduk()
{
       GLUquadricObj *p = gluNewQuadric();

//tutup atas
glPushMatrix();
glTranslatef(0,0,0);
gluQuadricDrawStyle(p, GLU_FILL);
gluQuadricOrientation(p, GLU_INSIDE);
gluDisk(p, 0, 1, 5, 1);
glPopMatrix();

glPushMatrix();
glTranslatef(0,0,0);
gluQuadricDrawStyle(p, GLU_FILL);
gluQuadricOrientation(p, GLU_OUTSIDE);
gluDisk(p, 0, 1, 5, 1);
glPopMatrix();

//tabung
glPushMatrix();
glTranslatef(0,0,0);
gluQuadricDrawStyle(p, GLU_FILL);
gluQuadricOrientation(p, GLU_INSIDE);
gluCylinder(p, 1, 1,1, 5, 1);
glPopMatrix();

glPushMatrix();
glTranslatef(0,0,0);
gluQuadricDrawStyle(p, GLU_FILL);
gluQuadricOrientation(p, GLU_OUTSIDE);
gluCylinder(p, 1, 1,1, 5, 1);
glPopMatrix();

//tutup bawah
glPushMatrix();
glTranslatef(0,0,1);
gluQuadricDrawStyle(p, GLU_FILL);
gluQuadricOrientation(p, GLU_INSIDE);
gluDisk(p, 0, 1, 5, 1);
glPopMatrix();

glPushMatrix();
glTranslatef(0,0,1);
gluQuadricDrawStyle(p, GLU_FILL);
gluQuadricOrientation(p, GLU_OUTSIDE);
gluDisk(p, 0, 1, 5, 1);
glPopMatrix();

}


void bianglala2()
{
const double t = glutGet(GLUT_ELAPSED_TIME) / 900.0;
const double a = t*90.0;
 glPushMatrix();
 glTranslatef(0,-3,0);
 bianglala();
 glPopMatrix();

 glPushMatrix();
 glTranslatef(0,-5,0);
 bianglala();
 glPopMatrix();

 glPushMatrix();
 glTranslatef(0,-5,0);
 tiangbianglala();
 glPopMatrix();

 glPushMatrix();
 glTranslated(12,7,0);
 glRotated(90,1,0,0);
 glRotated(a,0,0,1);
 glScaled(2,2,2);
 duduk();
 glPopMatrix();

 glPushMatrix();
 glTranslated(-12,7,0);
 glRotated(90,1,0,0);
 glRotated(a,0,0,1);
 glScaled(2,2,2);
 duduk();
 glPopMatrix();

 glPushMatrix();
 glTranslated(0,7,12);
 glRotated(90,1,0,0);
 glRotated(a,0,0,1);
 glScaled(2,2,2);
 duduk();
 glPopMatrix();

 glPushMatrix();
 glTranslated(0,7,-12);
 glRotated(90,1,0,0);
 glRotated(a,0,0,1);
 glScaled(2,2,2);
 duduk();
 glPopMatrix();

 glPushMatrix();
 glTranslated(-8.4,7,-8.4);
 glRotated(90,1,0,0);
 glRotated(a,0,0,1);
 glScaled(2,2,2);
 duduk();
 glPopMatrix();

 glPushMatrix();
 glTranslated(8.4,7,-8.4);
 glRotated(90,1,0,0);
 glRotated(a,0,0,1);
 glScaled(2,2,2);
 duduk();
 glPopMatrix();

 glPushMatrix();
 glTranslated(-8.4,7,8.4);
 glRotated(90,1,0,0);
 glRotated(a,0,0,1);
 glScaled(2,2,2);
 duduk();
 glPopMatrix();

 glPushMatrix();
 glTranslated(8.4,7,8.4);
 glRotated(90,1,0,0);
 glRotated(a,0,0,1);
 glScaled(2,2,2);
 duduk();
 glPopMatrix();

}

void tiangutama()
{

 //tiang penyangga

glPushMatrix();
glTranslated(0,2.3,0);
glRotated(270,1,0,0);
glScaled(0.7,0.7,7);
cylinder();
glPopMatrix();

//tiang penyangga utama
glPushMatrix();
glTranslatef(8,9,1.5);
glRotatef(90,1,0,0);
glRotated(10,0,0,1);
glScalef(17,0.5,0.5);
glutSolidCube(1);
glPopMatrix();

glPushMatrix();
glTranslatef(8,9,-1.5);
glRotatef(90,1,0,0);
glRotated(350,0,0,1);
glScalef(17,0.5,0.5);
glutSolidCube(1);
glPopMatrix();

glPushMatrix();
glTranslatef(8,2.5,1.5);
glRotatef(90,1,0,0);
glRotated(10,0,0,1);
glScalef(17,0.5,0.5);
glutSolidCube(1);
glPopMatrix();

glPushMatrix();
glTranslatef(8,2.5,-1.5);
glRotatef(90,1,0,0);
glRotated(350,0,0,1);
glScalef(17,0.5,0.5);
glutSolidCube(1);
glPopMatrix();

}


//PAPAN
void papan()
{

//tiang kiri
glPushMatrix();
glTranslatef(-15.5,-10,5);
glRotated(270, 1, 0, 0);
glScalef(0.5,0.5,25.3);
cylinder();
glPopMatrix();

//tiang kanan
glPushMatrix();
glTranslatef(15.5,-10,5);
glRotated(270, 1, 0, 0);
glScalef(0.5,0.5,25.3);
cylinder();
glPopMatrix();

//papan
glPushMatrix();
glTranslatef(0,12,5);
glScalef(30,8,0.5);
glutSolidCube(1);
glPopMatrix();

}

//PAGAR
void pagar()
{
    //Pagar Atas
    glPushMatrix();
	glColor4f(0.5f, 0.5f, 0.5f, 1.0f);
    glTranslatef(9, 1, 0);
    glScaled(40.0, 1.0 , 0.5);
    glutSolidCube(0.5f);
    glPopMatrix();

    //Pagar Bawah
    glPushMatrix();
	glColor4f(0.5f, 0.5f, 0.5f, 1.0f);
    glTranslatef(9, 2, 0.0);
    glScaled(40.0, 1.0 , 0.5);
    glutSolidCube(0.5f);
    glPopMatrix();

    //Pagar Bawah
    glPushMatrix();
	glColor4f(0.5f, 0.5f, 0.5f, 1.0f);
    glTranslatef(9, 3, 0.0);
    glScaled(40.0, 1.0 , 0.5);
    glutSolidCube(0.5f);
    glPopMatrix();

        //Pagar Bawah
    glPushMatrix();
	glColor4f(0.5f, 0.5f, 0.5f, 1.0f);
    glTranslatef(9, 4, 0.0);
    glScaled(40.0, 1.0 , 0.5);
    glutSolidCube(0.5f);
    glPopMatrix();

    //Pagar Tegak
    glPushMatrix();
	glColor4f(0.5f, 0.5f, 0.5f, 1.0f);
    glTranslatef(0.0f, 0.0f, 0.0f);
    glScaled(1.5, 18 , 0.5);
    glutSolidCube(0.5f);
    glPopMatrix();

        //Pagar Tegak
    glPushMatrix();
	glColor4f(0.5f, 0.5f, 0.5f, 1.0f);
    glTranslatef(18, 0, 0);
    glScaled(1.5, 18 , 0.5);
    glutSolidCube(0.5f);
    glPopMatrix();
}



void markajalan(void) {

    // marka jalan
    glPushMatrix();
    glScaled(1, 0.05,0.3);
   glTranslatef(2.4,2.5,67);
   glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
     glColor3f(1,1,1);
    glutSolidCube(5.0);
    glPopMatrix();
}

void pohon(){
	glColor3ub(104,70,14);
	//<<<<<<<<<<<<<<<<<<<< Batang >>>>>>>>>>>>>>>>>>>>>>>
	glPushMatrix();
	glScalef(0.2, 2, 0.2);
	glutSolidSphere(1.0, 10, 16);
	glPopMatrix();
	//<<<<<<<<<<<<<<<<<<<< end Batang >>>>>>>>>>>>>>>>>>>>>>>

	glColor3ub(18,118,13);
	//<<<<<<<<<<<<<<<<<<<< Daun >>>>>>>>>>>>>>>>>>>>>>>
	glPushMatrix();
	glScalef(1.5, 1, 1.5);
	glTranslatef(0, 1, 0);
	glRotatef(270, 1, 0, 0);
	glutSolidCone(1,3,10,1);
	glPopMatrix();

	glPushMatrix();
	glScalef(1.4, 1, 1.4);
	glTranslatef(0, 1.7, 0);
	glRotatef(270, 1, 0, 0);
	glutSolidCone(1,2,10,1);
	glPopMatrix();

	glPushMatrix();
	glScalef(1.2, 1, 1.2);
	glTranslatef(0, 2.4, 0);
	glRotatef(270, 1, 0, 0);
	glutSolidCone(1,1.8,10,1);
	glPopMatrix();
	//<<<<<<<<<<<<<<<<<<<< end Daun >>>>>>>>>>>>>>>>>>>>>>>
}
void awan(void){
glPushMatrix();
glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
glColor3ub(153, 223, 255);
glutSolidSphere(10, 50, 50);
glPopMatrix();
glPushMatrix();
glTranslatef(10,0,1);
glutSolidSphere(5, 50, 50);
glPopMatrix();
glPushMatrix();
glTranslatef(-2,6,-2);
glutSolidSphere(7, 50, 50);
glPopMatrix();
glPushMatrix();
glTranslatef(-10,-3,0);
glutSolidSphere(7, 50, 50);
glPopMatrix();
glPushMatrix();
glTranslatef(6,-2,2);
glutSolidSphere(7, 50, 50);
glPopMatrix();
}
void lampu()
{
   //Tiang Tegak
	glPushMatrix();
	glColor3f(0.5, 0.5, 0.5);
	glScalef(0.04,1.7,0.05);
	glutSolidCube(7.0f);
	glPopMatrix();

    //Tiang Atas
	glPushMatrix();
	glColor3f(0.5f, 0.5f, 0.5f);
	glTranslatef(0.0,5.3,-2.0);
    glScaled(0.5, 1.0 , 7.5);
    glutSolidCube(0.5f);
	glPopMatrix();

	//Lampu
	glPushMatrix();
	glTranslatef(0.0, 4.7, -3.7);
	glColor3f(1, 1, 1);
	glScalef(0.8,0.8,1.5);
	glutSolidSphere(0.5,70,20);
	glPopMatrix();

}

void kora2()
{
GLUquadricObj *p = gluNewQuadric();
//tabung
glPushMatrix();
glTranslatef(0,0,0);
gluQuadricDrawStyle(p, GLU_FILL);
gluQuadricOrientation(p, GLU_INSIDE);
gluCylinder(p, 1, 2,0.9, 6, 1);
glPopMatrix();

glPushMatrix();
glTranslatef(0,0,0);
gluQuadricDrawStyle(p, GLU_FILL);
gluQuadricOrientation(p, GLU_OUTSIDE);
gluCylinder(p, 1, 2,0.9, 6, 1);
glPopMatrix();

//sisi
glPushMatrix();
glTranslated(0,0,0.9);
glRotatef(90,0,0,1);
glScaled(1.7,1.7,1);
glutSolidTorus(0.2,1,8,6);
glPopMatrix();


//tutup
glPushMatrix();
glTranslatef(0,0,0);
gluQuadricDrawStyle(p, GLU_FILL);
gluQuadricOrientation(p, GLU_INSIDE);
gluDisk(p, 0, 1, 6, 1);
glPopMatrix();

glPushMatrix();
glTranslatef(0,0,0);
gluQuadricDrawStyle(p, GLU_FILL);
gluQuadricOrientation(p, GLU_OUTSIDE);
gluDisk(p, 0, 1, 6, 1);
glPopMatrix();

glPushMatrix();
glTranslatef(0,0,0.7);
gluQuadricDrawStyle(p, GLU_FILL);
gluQuadricOrientation(p, GLU_INSIDE);
gluDisk(p, 0, 1.7, 6, 1);
glPopMatrix();

glPushMatrix();
glTranslatef(0,0,0.7);
gluQuadricDrawStyle(p, GLU_FILL);
gluQuadricOrientation(p, GLU_OUTSIDE);
gluDisk(p, 0, 1.7, 6, 1);
glPopMatrix();


}

void tiangkora()
{
    glPushMatrix();
    glTranslatef(2.5,25,5.5);
    glRotated(110,1,0,0);
    glScaled(0.5,0.5,15);
    cylinder();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-2.5,25,5.5);
    glRotated(110,1,0,0);
    glScaled(0.5,0.5,15);
    cylinder();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-2.5,25,-5.5);
    glRotated(70,1,0,0);
    glScaled(0.5,0.5,15);
    cylinder();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(2.5,25,-5.5);
    glRotated(70,1,0,0);
    glScaled(0.5,0.5,15);
    cylinder();
    glPopMatrix();


}

void katrol()
{
//katrol

glPushMatrix();
glTranslatef(-6,40,0);
glRotated(90,0,1,0);
glScalef(1,1,12);
cylinder();
glPopMatrix();

glPushMatrix();
glTranslatef(5.35,40,0);
glScaled(1.3,1.3,15);
glutSolidCube(1);
glPopMatrix();

glPushMatrix();
glTranslatef(-5.35,40,0);
glScaled(1.3,1.3,15);
glutSolidCube(1);
glPopMatrix();

glPushMatrix();
glTranslatef(-5.35,25,-12.2);
glRotated(110,1,0,0);
glScaled(1.3,1.3,32);
glutSolidCube(1);
glPopMatrix();

glPushMatrix();
glTranslatef(5.35,25,-12.2);
glRotated(110,1,0,0);
glScaled(1.3,1.3,32);
glutSolidCube(1);
glPopMatrix();

glPushMatrix();
glTranslatef(5.35,25,12.2);
glRotated(70,1,0,0);
glScaled(1.3,1.3,32);
glutSolidCube(1);
glPopMatrix();

glPushMatrix();
glTranslatef(-5.35,25,12.2);
glRotated(70,1,0,0);
glScaled(1.3,1.3,32);
glutSolidCube(1);
glPopMatrix();




}



void hasil()
{

/*
glPushMatrix();
glTranslated(0,20,0);
glRotated(270,1,0,0);
glRotated(180,0,0,1);

if (rotasi<=90) {
    rotasi+=1;
    glRotated(rotasi,1,0,0);
    if (rotasi==90) {rotasi2=90;}
}
else if (rotasi2>=-90) {
    rotasi2-=1;
    glRotated(rotasi2,1,0,0);
    if (rotasi2==-90) {rotasi3=-90;}
}
else if (rotasi3<=90) {
    rotasi3+=1;
    glRotated(rotasi3,1,0,0);
    if (rotasi3==90) {rotasi2=90;}
}
glScalef(2, 6, 5);
kora2();
glPopMatrix();
*/


glPushMatrix();
glTranslated(0,40,0);
glRotated(180,0,1,0);
glRotated(180,1,0,0);



if (rotasi<=90) {
    rotasi+=5;
    glRotated(rotasi,1,0,0);
    if (rotasi==90) {rotasi2=90;}
}
else if (rotasi2>=-90) {
    rotasi2-=5;
    glRotated(rotasi2,1,0,0);
    if (rotasi2==-90) {rotasi3=-90;}
}
else if (rotasi3<=90) {
    rotasi3+=5;
    glRotated(rotasi3,1,0,0);
    if (rotasi3==90) {rotasi2=90;}
}


glTranslated(0,-10,0);
tiangkora();
glPushMatrix();
glTranslated(0,30,0);
glRotated(90,1,0,0);
glScaled(2,6,5);
kora2();
glPopMatrix();
glPopMatrix();

glPushMatrix();

glTranslated(0,0,0);
katrol();
glPopMatrix();


}

//-------------------------------ONTANG-ANTING------------------------------------------
//--------------------------------------------------------------------------------------
void kursiontang()
{
    glPushMatrix();
    glTranslatef(0,0,0);
    glScaled(1,1,0.5);
    cylinder();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0,-0.8,0.25);
    glScalef(2,1.5,0.5);
    glutSolidCube(1);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0,-1.3,-0.9);
    glRotated(270,1,0,0);
    glScalef(2,2,0.5);
    glutSolidCube(1);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(1,-1.3,-0.6);
    glRotated(90,0,1,0);
    glScaled(1,1,1);
    glutSolidTorus(0.2,1,360,360);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-1,-1.3,-0.6);
    glRotated(90,0,1,0);
    glScaled(1,1,1);
    glutSolidTorus(0.2,1,360,360);
    glPopMatrix();

    //tiangkursi
    glPushMatrix();
    glTranslatef(1,-0.3,-0.7);
    glRotated(270,1,0,0);
    glScaled(0.15,0.15,13);
    cylinder();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-1,-0.3,-0.7);
    glRotated(270,1,0,0);
    glScaled(0.15,0.15,13);
    cylinder();
    glPopMatrix();

}

void cylinder2()
{
    GLUquadricObj *p = gluNewQuadric();

//tutup atas
glPushMatrix();
glTranslatef(0,0,0);
gluQuadricDrawStyle(p, GLU_FILL);
gluQuadricOrientation(p, GLU_INSIDE);
gluDisk(p, 0, 1, 8, 1);
glPopMatrix();

glPushMatrix();
glTranslatef(0,0,0);
gluQuadricDrawStyle(p, GLU_FILL);
gluQuadricOrientation(p, GLU_OUTSIDE);
gluDisk(p, 0, 1, 8, 1);
glPopMatrix();

//tabung
glPushMatrix();
glTranslatef(0,0,0);
gluQuadricDrawStyle(p, GLU_FILL);
gluQuadricOrientation(p, GLU_INSIDE);
gluCylinder(p, 1, 1,1, 8, 1);
glPopMatrix();

glPushMatrix();
glTranslatef(0,0,0);
gluQuadricDrawStyle(p, GLU_FILL);
gluQuadricOrientation(p, GLU_OUTSIDE);
gluCylinder(p, 1, 1,1, 8, 1);
glPopMatrix();

//tutup bawah
glPushMatrix();
glTranslatef(0,0,1);
gluQuadricDrawStyle(p, GLU_FILL);
gluQuadricOrientation(p, GLU_INSIDE);
gluDisk(p, 0, 1, 8, 1);
glPopMatrix();

glPushMatrix();
glTranslatef(0,0,1);
gluQuadricDrawStyle(p, GLU_FILL);
gluQuadricOrientation(p, GLU_OUTSIDE);
gluDisk(p, 0, 1, 8, 1);
glPopMatrix();
}


void ontang()
{
    /*
    //base
    glPushMatrix();
    glTranslatef(0,1,0);
    glRotated(90,1,0,0);
    glScaled(25,25,1);
    cylinder();
    glPopMatrix();

    //tiang
    glPushMatrix();
    glTranslatef(0,1,0);
    glRotated(270,1,0,0);
    glScaled(2,2,40);
    cylinder();
    glPopMatrix();
    */

    //atap
    glPushMatrix();
    glTranslatef(0,39,0);
    glRotated(270,1,0,0);
    //glRotated(180,0,1,0);
    glutSolidCone(25,10,8,1);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0,38,0);
    glRotated(270,1,0,0);
    //glRotated(180,0,1,0);
    glScaled(25,25,1);
    cylinder2();
    glPopMatrix();


    //kursiontang
    glPushMatrix();
    glTranslatef(22,27,0);
    kursiontang();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-22,27,0);
    glRotated(180,0,1,0);
    kursiontang();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(17,27,-15);
    glRotated(45,0,1,0);
    kursiontang();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-17,27,15);
    glRotated(225,0,1,0);
    kursiontang();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0,27,-22);
    glRotated(90,0,1,0);
    kursiontang();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0,27,22);
    glRotated(270,0,1,0);
    kursiontang();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-14,27,-16);
    glRotated(135,0,1,0);
    kursiontang();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(14,27,16);
    glRotated(315,0,1,0);
    kursiontang();
    glPopMatrix();

}

void hasilontang()
{
    glPushMatrix();
    glTranslated(0,-10,0);
    if (putar<=360)
    {
    glRotated(putar,0,1,0);
    putar+=10;
    if (putar==360) {putar=0;}
    }

    ontang();
    glPopMatrix();

    //base
    glPushMatrix();
    glTranslatef(0,1,0);
    glRotated(90,1,0,0);
    glScaled(25,25,2);
    cylinder2();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0,2,0);
    glRotated(90,1,0,0);
    glScaled(22,22,2);
    cylinder2();
    glPopMatrix();

    //tiang
    glPushMatrix();
    glTranslatef(0,1,0);
    glRotated(270,1,0,0);
    glScaled(3.5,3.5,30);
    cylinder();
    glPopMatrix();

}


//-------------------------------ADSASDSADSADA------------------------------------------
//--------------------------------------------------------------------------------------







void display(void) {

const double t = glutGet(GLUT_ELAPSED_TIME) / 1000.0;
const double a = t*90.0;

	glClearStencil(0); //clear the stencil buffer
	glClearDepth(1.0f);
	glClearColor(0.0, 0.6, 0.8, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); //clear the buffers
	glLoadIdentity();
	gluLookAt(viewx, viewy, viewz, 0.0, 0.0, 5.0, 0.0, 1.0, 0.0);

	glPushMatrix();

	//glBindTexture(GL_TEXTURE_3D, texture[0]);
	drawSceneTanah(_terrain, 0.3f, 0.9f, 0.0f);
	glPopMatrix();

	//glPushMatrix();

	//glBindTexture(GL_TEXTURE_3D, texture[0]);
	//drawSceneTanah(_terrainTanah, 0.7f, 0.2f, 0.1f);
	//glPopMatrix();

	glPushMatrix();

	//glBindTexture(GL_TEXTURE_3D, texture[0]);
	drawSceneTanah(_terrainAir, 0.4902f, 0.4683f,0.4594f);
	glPopMatrix();

//-- JALAN
  for( int a = 220; a > -240; a = a - 40 )
   {
glPushMatrix();
glTranslatef(a,-0.5,44);
glScalef(3, 3, 3);
//glBindTexture(GL_TEXTURE_2D, texture[0]);
markajalan();
glPopMatrix();
   }

  for( int a = 70; a > -200; a = a - 40 )
   {
glPushMatrix();
glTranslatef(120,-0.5,a);
glRotated(90, 0, 1, 0);
glScalef(3, 3, 3);
//glBindTexture(GL_TEXTURE_2D, texture[0]);
markajalan();
glPopMatrix();
   }

  for( int a = 70; a > -200; a = a - 40 )
   {
glPushMatrix();
glTranslatef(-242,-0.5,a);
glRotated(90, 0, 1, 0);
glScalef(3, 3, 3);
//glBindTexture(GL_TEXTURE_2D, texture[0]);
markajalan();
glPopMatrix();
   }

//pohon
  for( int a = 70; a > -200; a = a - 40 )
   {
	glPushMatrix();
	glTranslatef(-230,2,a);
	glScalef(7, 7, 7);
	pohon();
	glPopMatrix();
   }


// --- PAGAR
  for( int a = 138; a > 0; a = a-20 )
   {
glPushMatrix();
glTranslatef(a,0.8,80);
pagar();
glPopMatrix();
   }

  for( int a = -35; a > -170; a = a-20 )
   {
glPushMatrix();
glTranslatef(a,0.8,80);
pagar();
glPopMatrix();
   }

for( int a = 79; a > -180; a = a - 20 )
   {
glPushMatrix();
glTranslatef(157,0.8,a);
glRotated(90, 0, 1, 0);
//glBindTexture(GL_TEXTURE_2D, texture[0]);
pagar();
glPopMatrix();
   }

for( int a = 79; a > -180; a = a - 20 )
   {
glPushMatrix();
glTranslatef(-156,0.8,a);
glRotated(90, 0, 1, 0);
//glBindTexture(GL_TEXTURE_2D, texture[0]);
pagar();
glPopMatrix();
   }

  for( int a = 138; a > -170; a = a-20 )
   {
glPushMatrix();
glTranslatef(a,0.8,-180);
pagar();
glPopMatrix();
   }

//--PAPAN
glPushMatrix();
glTranslatef(0.5,9,75);
papan();
glPopMatrix();



/*/ dis bianglala (jadi)
glPushMatrix();
glTranslatef(0,30,-60);
glRotated(90,1,0,0);
glRotated(a,0,1,0);
glScalef(2,2,2);
bianglala2();
glPopMatrix();

glPushMatrix();
glTranslatef(0,30,-60);
glRotated(90,1,0,0);
glRotated(270,0,1,0);
glScalef(2,2,2);
tiangutama();
glPopMatrix();
*/

/*/dis kora (jadi)
glPushMatrix();
glTranslatef(-50,-13, 0);
glScaled(1.2,1.2,1.2);
hasil();
glPopMatrix();
*/

/*/dis ontang (jadi)
glPushMatrix();
glTranslatef(0,0,0);
hasilontang();
glPopMatrix();
/*/









//awan
glPushMatrix();
glTranslatef(0, 90, -150);
glScalef(1.8, 1.0, 1.0);
awan();
glPopMatrix();

glPushMatrix();
glTranslatef(70, 70, -150);
glScalef(1.8, 1.0, 1.0);
awan();
glPopMatrix();

glPushMatrix();
glTranslatef(140, 90, -130);
glScalef(1.8, 1.0, 1.0);
awan();
glPopMatrix();

//LAMPU
    for( int a = 220; a > -240; a = a - 40 )
    {


    glPushMatrix();
    glTranslatef(a,0.05,127);
    glScalef(2, 2, 2);
    lampu();
    glPopMatrix();

    }

for( int a = 70; a > -200; a = a - 40 )
   {
glPushMatrix();
glTranslatef(207,-0.5,a);
glRotated(90, 0, 1, 0);
glScalef(2, 2, 2);
//glBindTexture(GL_TEXTURE_2D, texture[0]);
lampu();
glPopMatrix();
   }

/*glPushMatrix();
glTranslated(0,10,0);
glScaled(2,2,3);
duduk();
glPopMatrix();
*/

for( int a = 200; a > -240; a = a - 60 )
    {

    glPushMatrix();
    glTranslatef(a,0.05,170);
    glScalef(7, 7, 7);
    pohon();
    glPopMatrix();

    }

	glutSwapBuffers();
	glFlush();
	rot++;
	angle++;

}


void init(void) {
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glDepthFunc(GL_LESS);
	glEnable(GL_NORMALIZE);
	glEnable(GL_COLOR_MATERIAL);
	glDepthFunc(GL_LEQUAL);
	glShadeModel(GL_SMOOTH);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glEnable(GL_CULL_FACE);

	_terrain = loadTerrain("heightmap.bmp", 20);
	//_terrainTanah = loadTerrain("heightmapTanah.bmp", 20);
	_terrainAir = loadTerrain("heightmapAir.bmp", 20);

	//binding texture

}

static void kibor(int key, int x, int y) {
	switch (key) {
	case GLUT_KEY_HOME:
		viewy+=3;
		break;
	case GLUT_KEY_END:
		viewy-=3;
		break;
	case GLUT_KEY_UP:
		viewz-=3;
		break;
	case GLUT_KEY_DOWN:
		viewz+=3;
		break;

	case GLUT_KEY_RIGHT:
		viewx+=3;
		break;
	case GLUT_KEY_LEFT:
		viewx-=3;
		break;

	case GLUT_KEY_F1: {
		glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
		glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
		glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
		glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
	}
		;
		break;
	case GLUT_KEY_F2: {
		glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient2);
		glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse2);
		glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
		glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
	}
		;
		break;
	default:
		break;
	}
}

void keyboard(unsigned char key, int x, int y) {
	if (key == 'd') {

		spin = spin - 3;
		if (spin > 360.0)
			spin = spin - 360.0;
	}
	if (key == 'a') {
		spin = spin + 3;
		if (spin > 360.0)
			spin = spin - 360.0;
	}
	if (key == 'q') {
		viewz+=5;
	}
	if (key == 'e') {
		viewz-=5;
	}
	if (key == 's') {
		viewy-=5;
	}
	if (key == 'w') {
		viewy+=5;
	}
}

void reshape(int w, int h) {
	glViewport(0, 0, (GLsizei) w, (GLsizei) h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60, (GLfloat) w / (GLfloat) h, 0.1, 1000.0);
	glMatrixMode(GL_MODELVIEW);
}

int main(int argc, char **argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_STENCIL | GLUT_DEPTH); //add a stencil buffer to the window
	glutInitWindowSize(800, 600);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("Sample Terain");
	init();

	glutDisplayFunc(display);
	glutIdleFunc(display);
	glutReshapeFunc(reshape);
	glutSpecialFunc(kibor);

	glutKeyboardFunc(keyboard);

	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);

	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, high_shininess);
	glColorMaterial(GL_FRONT, GL_DIFFUSE);

	glutMainLoop();
	return 0;
}
