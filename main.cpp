#include <cstdlib>
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



GLUquadricObj *p = gluNewQuadric(); //membuat objek quadric yang baru



using namespace std;

float lastx, lasty;
GLint stencilBits;


static int viewx = 0;
static int viewy = 300;
static int viewz = 300;


float rot = 0;

float gerak=27.0;


GLuint texture[10];

struct Gambar {
	unsigned long sizeX;
	unsigned long sizeY;
	char *data;
};
typedef struct Gambar Gambar; //struktur data untuk


//ukuran gambar #bisa di set sesuai kebutuhan


//mengambil gambar BMP
int GambarLoad(char *filename, Gambar *gambar) {
	FILE *file;
	unsigned long size; // ukuran gambar dalam bytes
	unsigned long i; // standard counter.
	unsigned short int plane; // number of planes in gambar

	unsigned short int bpp; // jumlah bits per pixel
	char temp; // temporary color storage for var warna sementara untuk memastikan filenya ada


	if ((file = fopen(filename, "rb")) == NULL) {
		printf("File Not Found : %s\n", filename);
		return 0;
	}
	// mencari file header bmp
	fseek(file, 18, SEEK_CUR);
	// read the width
	if ((i = fread(&gambar->sizeX, 4, 1, file)) != 1) {
		printf("Error reading width from %s.\n", filename);
		return 0;
	}
	//printf("Width of %s: %lu\n", filename, gambar->sizeX);
	// membaca nilai height
	if ((i = fread(&gambar->sizeY, 4, 1, file)) != 1) {
		printf("Error reading height from %s.\n", filename);
		return 0;
	}
	//printf("Height of %s: %lu\n", filename, gambar->sizeY);
	//menghitung ukuran gambar(asumsi 24 bits or 3 bytes per pixel).

	size = gambar->sizeX * gambar->sizeY * 3;
	// read the planes
	if ((fread(&plane, 2, 1, file)) != 1) {
		printf("Error reading planes from %s.\n", filename);
		return 0;
	}
	if (plane != 1) {
		printf("Planes from %s is not 1: %u\n", filename, plane);
		return 0;
	}
	// read the bitsperpixel
	if ((i = fread(&bpp, 2, 1, file)) != 1) {
		printf("Error reading bpp from %s.\n", filename);

		return 0;
	}
	if (bpp != 24) {
		printf("Bpp from %s is not 24: %u\n", filename, bpp);
		return 0;
	}
	// seek past the rest of the bitmap header.
	fseek(file, 24, SEEK_CUR);
	// read the data.
	gambar->data = (char *) malloc(size);
	if (gambar->data == NULL) {
		printf("Error allocating memory for color-corrected gambar data");
		return 0;
	}
	if ((i = fread(gambar->data, size, 1, file)) != 1) {
		printf("Error reading gambar data from %s.\n", filename);
		return 0;
	}
	for (i = 0; i < size; i += 3) { // membalikan semuan nilai warna (gbr - > rgb)
		temp = gambar->data[i];
		gambar->data[i] = gambar->data[i + 2];
		gambar->data[i + 2] = temp;
	}
	// we're done.
	return 1;
}

//mengambil tekstur
Gambar * loadTexture() {
	Gambar *gambar1;
	// alokasi memmory untuk tekstur
	gambar1 = (Gambar *) malloc(sizeof(Gambar));
	if (gambar1 == NULL) {
		printf("Error allocating space for gambar");
		exit(0);
	}
	//pic.bmp is a 64x64 picture
	if (!GambarLoad("atap.bmp", gambar1)) {
		exit(1);
	}
	return gambar1;
}

Gambar * loadTexture2() {
	Gambar *gambar2;
	// alokasi memmory untuk tekstur
	gambar2 = (Gambar *) malloc(sizeof(Gambar));
	if (gambar2 == NULL) {
		printf("Error allocating space for gambar");
		exit(0);
	}
	//pic.bmp is a 64x64 picture
	if (!GambarLoad("dinding.bmp", gambar2)) {
		exit(1);
	}
	return gambar2;
}

Gambar * loadTexture3() {
	Gambar *gambar3;
	// alokasi memmory untuk tekstur
	gambar3 = (Gambar *) malloc(sizeof(Gambar));
	if (gambar3 == NULL) {
		printf("Error allocating space for gambar");
		exit(0);
	}
	//pic.bmp is a 64x64 picture
	if (!GambarLoad("pintu.bmp", gambar3)) {
		exit(1);
	}
	return gambar3;
}

Gambar * loadTexture4() {
	Gambar *gambar4;
	// alokasi memmory untuk tekstur
	gambar4 = (Gambar *) malloc(sizeof(Gambar));
	if (gambar4 == NULL) {
		printf("Error allocating space for gambar");
		exit(0);
	}
	//pic.bmp is a 64x64 picture
	if (!GambarLoad("bunga.bmp", gambar4)) {
		exit(1);
	}
	return gambar4;
}





/*------------------------------------------------------*\
    Class Terrain 
    -- Objek daratan --
\*------------------------------------------------------*/
class Terrain {
private:
	int w; //variabel lebar
	int l; //variabel panjang
	float** hs; //variabel tinggi
	Vec3f** normals; //variabel untuk pengembangan vektor objek 3 dimensi
	bool computedNormals; //Whether normals is up-to-date
public:
 	//Method terrain berisi 2 parameter
	Terrain(int w2, int l2) {
		w = w2;
		l = l2;

		hs = new float*[l]; //new objek hs dengan tipe float 
		for (int i = 0; i < l; i++) {
			hs[i] = new float[w];
		}

		normals = new Vec3f*[l]; //new objek vektor normal
		for (int i = 0; i < l; i++) {
			normals[i] = new Vec3f[w];
		}

		computedNormals = false;
	}
	//Method jika not terrain ..
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
	//Fungsi yang digunakan sebagai width (lebar)
	int width() {
		return w;
	}
	 //Fungsi yang digunakan sebagai length (panjang)
	int length() {
		return l;
	}

	//Procedure setHeight untuk mengatur tinggi x y dan z
	void setHeight(int x, int z, float y) {
		hs[z][x] = y;
		computedNormals = false;
	}

	//Function menghasilkan tinggi
	float getHeight(int x, int z) {
		return hs[z][x];
	}

	/ / Menghitung normals, jika belum dihitung
	void computeNormals() {
		if (computedNormals) {{ //jika true
			return;
		}

		//hitung
		Vec3f** normals2 = new Vec3f*[l];//new vektor pointer normal
		for (int i = 0; i < l; i++) {
			normals2[i] = new Vec3f[w];
		}

		for (int z = 0; z < l; z++) {
			for (int x = 0; x < w; x++) {
				Vec3f sum(0.0f, 0.0f, 0.0f);

				Vec3f out;
				if (z > 0) { // vektor out yang digunakan pembuatan vector normal belakang sesuai dengan x y z
					out = Vec3f(0.0f, hs[z - 1][x] - hs[z][x], -1.0f);
				}
				Vec3f in;
				if (z < l - 1) { // vector in yang digunakan vector normal depan sesuai dengan x y z
					in = Vec3f(0.0f, hs[z + 1][x] - hs[z][x], 1.0f);
				}
				Vec3f left;
				if (x > 0) {
					left = Vec3f(-1.0f, hs[z][x - 1] - hs[z][x], 0.0f);
				}
				Vec3f right;
				if (x < w - 1) { // vector right yang digunakan pembuatan vector normal kanan sesuai x y z 
					right = Vec3f(1.0f, hs[z][x + 1] - hs[z][x], 0.0f);
				}
				//Perhitungan Jumlah untuk pengisian nilai normal x dan z
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

	//Function untuk nilai normal
	Vec3f getNormal(int x, int z) {
		if (!computedNormals) {
			computeNormals();
		}
		return normals[z][x];
	}
};
//end class


/*------------------------------------------------*\
    Procedure Terrain 
    Load terrain berdasakan file dan tinggi
\*------------------------------------------------*/
// load terain di prosedur inisialisasi
//ketinggian
Terrain* loadTerrain(const char* filename, float height) {
	Image* image = loadBMP(filename);//Pengambilan gambar berdasarkan filename
	Terrain* t = new Terrain(image->width, image->height);//New Objek Terrain
	//--Pengaturan tinggi berdasarkan kedalaman warna 
	for (int y = 0; y < image->height; y++) {
		for (int x = 0; x < image->width; x++) {
			unsigned char color = (unsigned char) image->pixels[3 * (y
					* image->width + x)];
			float h = height * ((color / 255.0f) - 0.5f);// perhitungan kedalaman warna
			t->setHeight(x, y, h);
		}
	}
	 //--Hapus image setelah pengaturan tinggi
	delete image;
	t->computeNormals();
	return t;
}

float _angle = 60.0f;
/* 
    Inisialisasi Terain baru
    Pembuatan : 
        terain air
        terain Jalan
        terain kolam
*/

Terrain* _terrain;
Terrain* _terrainAir;
Terrain* _terrainStreet;
Terrain* _terrainKolam;

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

/*-----------------------------------------------------------------------*\
    Procedure drawSceneTanah()
    Penggambaran untuk display tanah berdasarkan nilai terain
    dan Nilai RGB
\*-----------------------------------------------------------------------*/
void drawSceneTanah(Terrain *terrain, GLfloat r, GLfloat g, GLfloat b) {
	//Penskalaan 
	float scale = 500.0f / max(terrain->width() - 1, terrain->length() - 1);
	//Skala ukuran objek
	glScalef(scale, scale, scale);
	//Pengaturan Posisi
	glTranslatef(-(float) (terrain->width() - 1) / 2, 0.0f,
			-(float) (terrain->length() - 1) / 2);
	//Pewarnaan sesuai nilai RGB
	glColor3f(r, g, b);
	//Pengulangan hingga terrain->length
	for (int z = 0; z < terrain->length() - 1; z++) {
		//Fungsi Pembuatam objek segitiga (triangle) pembentukan 3 sudut
		glBegin(GL_TRIANGLE_STRIP);
		//Pengulangan hingga terrain->width
		for (int x = 0; x < terrain->width(); x++) {
			Vec3f normal = terrain->getNormal(x, z);
			glNormal3f(normal[0], normal[1], normal[2]);
			glVertex3f(x, terrain->getHeight(x, z), z);
			normal = terrain->getNormal(x, z + 1);
			glNormal3f(normal[0], normal[1], normal[2]);
			glVertex3f(x, terrain->getHeight(x, z + 1), z + 1);
		}
		glEnd();//akhir penggambaran
	}
}

//Inisialisasi procedur LoadTextureFromBmpFile()
unsigned int LoadTextureFromBmpFile(char *filename);

/*--------------------------------------------------*\
    Procedure Awan
\*--------------------------------------------------*/

void awan(void){
      //Pembentukkan awan    
    glPushMatrix();//penyimpanan objek
    glColor3ub(153, 223, 255);//Penentuan warna
    glutSolidSphere(10, 50, 50);//Penggambaran objek
    glPopMatrix();//pemanggilan objek
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

//PAGAR
void pagar()
{
    //Pagar Atas
    glPushMatrix();//untuk menyimpan koordinat yang telah dibuat
	glColor4f(0.5f, 0.5f, 0.5f, 1.0f);//menentukan warna abu-abu untuk pagar
    glTranslatef(9, 1, 0);//untuk mentranslasi / memindahkan posisi objek
    glScaled(40.0, 1.0 , 0.5);//untuk mengubah ukuran(memperbesar/memperkecil) objek
    glutSolidCube(0.5f);// untuk membuat objek kubus. 0.5 merupakan ukuran bentuk kubus
    glPopMatrix();//untuk memanggil fungsi - fungsi yang telah dibuat pada glPushMatrix()

    //Pagar Bawah
    glPushMatrix();//untuk menyimpan koordinat yang telah dibuat
	glColor4f(0.5f, 0.5f, 0.5f, 1.0f);//menentukan warna abu-abu untuk pagar
    glTranslatef(9, 2, 0.0);//untuk mentranslasi / memindahkan posisi objek
    glScaled(40.0, 1.0 , 0.5);//untuk mengubah ukuran(memperbesar/memperkecil) objek
    glutSolidCube(0.5f);// untuk membuat objek kubus. 0.5 merupakan ukuran bentuk kubus
    glPopMatrix();//untuk memanggil fungsi - fungsi yang telah dibuat pada glPushMatrix()

    //Pagar Bawah
    glPushMatrix();//untuk menyimpan koordinat yang telah dibuat
	glColor4f(0.5f, 0.5f, 0.5f, 1.0f);//menentukan warna abu-abu untuk pagar
    glTranslatef(9, 3, 0.0);//untuk mentranslasi / memindahkan posisi objek
    glScaled(40.0, 1.0 , 0.5);//untuk mengubah ukuran(memperbesar/memperkecil) objek
    glutSolidCube(0.5f);// untuk membuat objek kubus. 0.5 merupakan ukuran bentuk kubus
    glPopMatrix();//untuk memanggil fungsi - fungsi yang telah dibuat pada glPushMatrix()

        //Pagar Bawah
    glPushMatrix();//untuk menyimpan koordinat yang telah dibuat
	glColor4f(0.5f, 0.5f, 0.5f, 1.0f);//menentukan warna abu-abu untuk pagar
    glTranslatef(9, 4, 0.0);//untuk mentranslasi / memindahkan posisi objek
    glScaled(40.0, 1.0 , 0.5);//untuk mengubah ukuran(memperbesar/memperkecil) objek
    glutSolidCube(0.5f);// untuk membuat objek kubus. 0.5 merupakan ukuran bentuk kubus
    glPopMatrix();//untuk memanggil fungsi - fungsi yang telah dibuat pada glPushMatrix()

    //Pagar Tegak
    glPushMatrix();//untuk menyimpan koordinat yang telah dibuat
	glColor4f(0.5f, 0.5f, 0.5f, 1.0f);//menentukan warna abu-abu untuk pagar
    glTranslatef(0.0f, 0.0f, 0.0f);//untuk mentranslasi / memindahkan posisi objek
    glScaled(1.5, 18 , 0.5);//untuk mengubah ukuran(memperbesar/memperkecil) objek
    glutSolidCube(0.5f);// untuk membuat objek kubus. 0.5 merupakan ukuran bentuk kubus
    glPopMatrix();//untuk memanggil fungsi - fungsi yang telah dibuat pada glPushMatrix()

        //Pagar Tegak
    glPushMatrix();//untuk menyimpan koordinat yang telah dibuat
	glColor4f(0.5f, 0.5f, 0.5f, 1.0f);//menentukan warna abu-abu untuk pagar
    glTranslatef(18, 0, 0);//untuk mentranslasi / memindahkan posisi objek
    glScaled(1.5, 18 , 0.5);//untuk mengubah ukuran(memperbesar/memperkecil) objek
    glutSolidCube(0.5f);// untuk membuat objek kubus. 0.5 merupakan ukuran bentuk kubus
    glPopMatrix();//untuk memanggil fungsi - fungsi yang telah dibuat pada glPushMatrix()
}

void bendera()
 {

GLUquadricObj *quad = gluNewQuadric();

//tiang bendera
    glPushMatrix();
    glColor3f(1.0,1.0,1.0);
    glTranslated(0.0,-10,0.0);
    glRotated(90,-1.0,0.0,0.0);
    glScaled(2.5,2.5,20);
    gluCylinder(quad,0.2,0.2,2,50,50);
    glPopMatrix();


//dasar tiang
    glPushMatrix();
    glColor3f(1.0,1.0,1.0);
    glTranslated(0.0,-8.0,0.0);
    glRotated(90,-1.0,0.0,0.0);
    glScaled(10,10,1);
    gluCylinder(quad,0.2,0.2,2,50,50);
    glPopMatrix();

//tutup dasartiang
    glDisable(GL_CULL_FACE);
    glPushMatrix();
    glColor3f(0.8f, 0.4f, 0.1f);
    glTranslated(0,-8,0);
    glRotated(90,-1,0,0);
    gluDisk(quad,0,2,50,1);
    glPopMatrix();
    glEnable(GL_CULL_FACE);

//benderamerah
    glPushMatrix();
    glColor3f(255,255,255);
    glTranslated(4,29,0);
    glScaled(8,2,0.1);
    glutSolidCube(1);
    glPopMatrix();

//benderaputih
    glPushMatrix();
    glColor3f(255,0,0);
    glTranslated(4,27,0);
    glScaled(8,2,0.1);
    glutSolidCube(1);
    glPopMatrix();

//tutup tiang
    glDisable(GL_CULL_FACE);
    glPushMatrix();
    glColor3f(0.8f, 0.4f, 0.1f);
    glTranslated(0,-10,0.0);
    glRotated(90,-1.0,0.0,0.0);
    gluDisk(quad,0,0.5,50,1);
    glPopMatrix();
    glEnable(GL_CULL_FACE);



//tangga 1
    glPushMatrix();
    glTranslated(0,-8,0);
    glScaled(25,1.5,25);
    glutSolidCube(1);
    glPopMatrix();

//tangga 2
    glPushMatrix();
    glTranslated(0,-9,0);
    glScaled(29,1.5,29);
    glutSolidCube(1);
    glPopMatrix();

//tangga 3
    glPushMatrix();
    glTranslated(0,-10,0);
    glScaled(33,1.5,33);
    glutSolidCube(1);
    glPopMatrix();

 }

//kelompok fitri

//rumah

void rumah()
{
    glDisable(GL_CULL_FACE);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D,texture[0]);
    glPushMatrix();
    glTranslatef(-5.0f,0.0f,-3.0f);

     //atap kanan
    glBegin(GL_POLYGON);
    glColor3f(1.0f, 0.4f, 0.0f);
        glTexCoord2f(1.0,0.0);glVertex3f(0.75,0.7,0.0);
        glTexCoord2f(1.0,1.0);glVertex3f(1.5,0.5,0.0);
        glTexCoord2f(0.0,1.0);glVertex3f(1.5,0.5,-1.8);
        glTexCoord2f(0.0,0.0);glVertex3f(0.75,0.7,-1.8);
    glEnd();

    //atap kiri
    glBegin(GL_POLYGON);
        glTexCoord2f(1.0,0.0);glVertex3f(0.75,0.7,0.0);
        glTexCoord2f(1.0,1.0);glVertex3f(0.0,0.5,0.0);
        glTexCoord2f(0.0,1.0);glVertex3f(0.0,0.5,-1.8);
        glTexCoord2f(0.0,0.0);glVertex3f(0.75,0.7,-1.8);
    glEnd();

    glDisable(GL_TEXTURE_2D);
    //bagian atas
    glBegin(GL_POLYGON);
    glColor3f(1.0f,0.4f,0.0f);
        glVertex3f(0.0,0.5,0.0);
        glVertex3f(1.5,0.5,0.0);
        glVertex3f(1.5,0.5,-1.8);
        glVertex3f(0.0,0.5,-1.8);
    glEnd();

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D,texture[1]);
    //bagian belakang
    glBegin(GL_POLYGON);
    //glColor3f(0.1f,0.0f,0.0f);
        glTexCoord2f(0.0,0.0);glVertex3f(0.0,0.0,-1.8);
        glTexCoord2f(1.0,0.0);glVertex3f(1.5,0.0,-1.8);
        glTexCoord2f(1.0,1.0);glVertex3f(1.5,0.5,-1.8);
        glTexCoord2f(0.0,1.0);glVertex3f(0.0,0.5,-1.8);
    glEnd();

    //bagian kanan
    glBegin(GL_POLYGON);
        glTexCoord2f(1.0,0.0);glVertex3f(0.0,0.0,0.0);
        glTexCoord2f(1.0,1.0);glVertex3f(0.0,0.5,0.0);
        glTexCoord2f(0.0,1.0);glVertex3f(0.0,0.5,-1.8);
        glTexCoord2f(0.0,0.0);glVertex3f(0.0,0.0,-1.8);
    glEnd();

    //bagian tutup kanan
    glBegin(GL_POLYGON);
        glTexCoord2f(0.0,0.0);glVertex3f(0.0,0.0,0.0);
        glTexCoord2f(1.0,0.0);glVertex3f(0.5,0.0,0.0);
        glTexCoord2f(1.0,1.0);glVertex3f(0.5,0.5,0.0);
        glTexCoord2f(0.0,1.0);glVertex3f(0.0,0.5,0.0);
    glEnd();

    //bagian kiri
    glBegin(GL_POLYGON);
        glTexCoord2f(1.0,0.0);glVertex3f(1.5,0.0,0.0);
        glTexCoord2f(1.0,1.0);glVertex3f(1.5,0.5,0.0);
        glTexCoord2f(0.0,1.0);glVertex3f(1.5,0.5,-1.8);
        glTexCoord2f(0.0,0.0);glVertex3f(1.5,0.0,-1.8);
    glEnd();

    //bagian tutup kiri
    glBegin(GL_POLYGON);
        glTexCoord2f(0.0,0.0);glVertex3f(1.5,0.0,0.0);
        glTexCoord2f(1.0,0.0);glVertex3f(1,0.0,0.0);
        glTexCoord2f(1.0,1.0);glVertex3f(1,0.5,0.0);
        glTexCoord2f(0.0,1.0);glVertex3f(1.5,0.5,0.0);
    glEnd();
    glDisable(GL_TEXTURE_2D);

    //pintu
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D,texture[2]);
    glBegin(GL_POLYGON);
        glTexCoord2f(0.0,0.0);glVertex3f(0.5,0.0,0.0);
        glTexCoord2f(1.0,0.0);glVertex3f(1,0.0,0.0);
        glTexCoord2f(1.0,1.0);glVertex3f(1,0.5,0.0);
        glTexCoord2f(0.0,1.0);glVertex3f(0.5,0.5,0.0);
    glEnd();

    glPopMatrix();
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_CULL_FACE);
}

void bunga()
{
    glColor3f(1,1,1);//menentukan warna untuk texture bunga
    glEnable(GL_TEXTURE_2D);//digunakan untuk mengaktifkan fungsi openGL, dengan tipe texture 2D
    glBindTexture(GL_TEXTURE_2D, texture[3]);//untuk memasang texture pada objek 2D dengan gambar texture array ke 3 yaitu bunga.bmp
    gluQuadricTexture(p,1);//menentukan apakah texture diberikan kepada quadric, 1 menandakan texture diberikan kepada quadric
    glPushMatrix();//untuk mentranslasi / memindahkan posisi objek
    glTranslated(0,10,0);//untuk mentranslasi / memindahkan posisi objek
    glScaled(3,3,3);//untuk mengubah ukuran(memperbesar/memperkecil) objek
    gluSphere(p,1,10,5);//menggambar sebuah objek bola, texture diberikan kepada objek bola.
    glPopMatrix();//untuk memanggil fungsi - fungsi yang telah dibuat pada glPushMatrix()
    glDisable(GL_TEXTURE_2D);
}


void pohon(){
	glColor3ub(104,70,14); // menentukan warna coklat untuk batang pohon
	//<<<<<<<<<<<<<<<<<<<< Batang >>>>>>>>>>>>>>>>>>>>>>>
	glPushMatrix(); //untuk menyimpan koordinat yang telah dibuat
	glScalef(0.2, 2, 0.2); //untuk mengubah ukuran(memperbesar/memperkecil) objek
	glutSolidSphere(1.0, 10, 16);//membuat bola berpusat pada asal pemodelan sistem koordinat.
	glPopMatrix();//untuk memanggil fungsi - fungsi yang telah dibuat pada glPushMatrix()
	//<<<<<<<<<<<<<<<<<<<< end Batang >>>>>>>>>>>>>>>>>>>>>>>

	glColor3ub(18,118,13);//menentukan warna hijau untuk daun pohon
	//<<<<<<<<<<<<<<<<<<<< Daun >>>>>>>>>>>>>>>>>>>>>>>
	glPushMatrix();//untuk menyimpan koordinat yang telah dibuat
	glScalef(1.5, 1, 1.5);//untuk mengubah ukuran(memperbesar/memperkecil) objek
	glTranslatef(0, 1, 0);//untuk mentranslasi / memindahkan posisi objek
	glRotatef(270, 1, 0, 0);//untuk merotasi/memutar objek. 270 merupakan derajat rotasi, dengan x=1,y=0,z=0
	glutSolidCone(1,3,10,1);//untuk membuat objek kerucut. 1 merupakan ukuran dari bentuk kerucut, dengan x=3,y=10,z=1
	glPopMatrix();//untuk memanggil fungsi - fungsi yang telah dibuat pada glPushMatrix()

	glPushMatrix();//untuk menyimpan koordinat yang telah dibuat
	glScalef(1.4, 1, 1.4);//untuk mengubah ukuran(memperbesar/memperkecil) objek
	glTranslatef(0, 1.7, 0);//untuk mentranslasi / memindahkan posisi objek
	glRotatef(270, 1, 0, 0);//untuk merotasi/memutar objek. 270 merupakan derajat rotasi, dengan x=1,y=0,z=0
	glutSolidCone(1,2,10,1);//untuk membuat objek kerucut. 1 merupakan ukuran dari bentuk kerucut, dengan x=2,y=10,z=1
	glPopMatrix();

	glPushMatrix();//untuk menyimpan koordinat yang telah dibuat
	glScalef(1.2, 1, 1.2);//untuk mengubah ukuran(memperbesar/memperkecil) objek
	glTranslatef(0, 2.4, 0);//untuk mentranslasi / memindahkan posisi objek
	glRotatef(270, 1, 0, 0);//untuk merotasi/memutar objek. 270 merupakan derajat rotasi, dengan x=1,y=0,z=0
	glutSolidCone(1,1.8,10,1);//untuk membuat objek kerucut. 1 merupakan ukuran dari bentuk kerucut, dengan x=8,y=10,z=1
	glPopMatrix();//untuk memanggil fungsi - fungsi yang telah dibuat pada glPushMatrix()
}

void pagarjadi()
{
	//pohon
    glPushMatrix();//untuk menyimpan koordinat yang telah dibuat
    glTranslatef(0,0,0);//untuk mentranslasi / memindahkan posisi objek
    glScalef(7, 7, 7);//untuk mengubah ukuran(memperbesar/memperkecil) objek
    pohon();//mengambil fungsi yang ada pada void pohon
    glPopMatrix();//untuk memanggil fungsi - fungsi yang telah dibuat pada glPushMatrix()


    //pagarbelakang
    for (int x=-35; x>=-60; x=x-25)//melakukan perulangan dengan nilai awal x=-35 dan nilai akhir x=-60, pada setiap perulangan nilai x dikurang 25
    {
        glPushMatrix();//untuk menyimpan koordinat yang telah dibuat
        glTranslatef(x,0,-40);//untuk mentranslasi / memindahkan posisi objek, objek akan diulang dengan pengurangan titik x sebanyak 25.
        glScalef(1.5, 3, 3);//untuk mengubah ukuran(memperbesar/memperkecil) objek
        pagar();//mengambil fungsi yang ada pada void pagar
        glPopMatrix();//untuk memanggil fungsi - fungsi yang telah dibuat pada glPushMatrix()
    }

    //pagar samping kanan
    for (int z1=25; z1>=-30; z1=z1-10)//melakukan perulangan dengan nilai awal z1=25 dan nilai akhir z1=-30, pada setiap perulangan nilai z1 dikurang 10
    {
        glPushMatrix();//untuk menyimpan koordinat yang telah dibuat
        glTranslatef(-8,0,z1);//untuk mentranslasi / memindahkan posisi objek, objek akan diulang dengan pengurangan titik z sebanyak 10.
        glScalef(1.5, 3, 0.8);//untuk mengubah ukuran(memperbesar/memperkecil) objek
        glRotatef(90,0,1,0);//untuk merotasi/memutar objek. 90 merupakan derajat rotasi, dengan x=0,y=1,z=0
        pagar();//mengambil fungsi yang ada pada void pagar
        glPopMatrix();//untuk memanggil fungsi - fungsi yang telah dibuat pada glPushMatrix()
    }

    //pagar samping kiri
    for (int z2=25; z2>=-30; z2=z2-10)//melakukan perulangan dengan nilai awal z2=25 dan nilai akhir z2=-30, pada setiap perulangan nilai z2 dikurang 10
    {
        glPushMatrix();//untuk menyimpan koordinat yang telah dibuat
        glTranslatef(-60,0,z2);//untuk mentranslasi / memindahkan posisi objek, objek akan diulang dengan pengurangan titik z sebanyak 10.
        glScalef(1.5, 3, 0.8);//untuk mengubah ukuran(memperbesar/memperkecil) objek
        glRotatef(90,0,1,0);//untuk merotasi/memutar objek. 90 merupakan derajat rotasi, dengan x=0,y=1,z=0
        pagar();//mengambil fungsi yang ada pada void pagar
        glPopMatrix();//untuk memanggil fungsi - fungsi yang telah dibuat pada glPushMatrix()
    }

        //pager depan kanan
        glPushMatrix();//untuk menyimpan koordinat yang telah dibuat
        glTranslatef(-27,0,25);//untuk mentranslasi / memindahkan posisi objek
        glScalef(1, 3, 0.8);//untuk mengubah ukuran(memperbesar/memperkecil) objek
        pagar();//mengambil fungsi yang ada pada void pagar
        glPopMatrix();//untuk memanggil fungsi - fungsi yang telah dibuat pada glPushMatrix()

        //pager depan kiri
        glPushMatrix();//untuk menyimpan koordinat yang telah dibuat
        glTranslatef(-60,0,25);//untuk mentranslasi / memindahkan posisi objek
        glScalef(1, 3, 0.8);//untuk mengubah ukuran(memperbesar/memperkecil) objek
        pagar();//mengambil fungsi yang ada pada void pagar
        glPopMatrix();//untuk memanggil fungsi - fungsi yang telah dibuat pada glPushMatrix()
        glDisable(GL_TEXTURE_2D);//digunakan untuk mematikan fungsi openGL yang sedang aktifkan

}

//BUS
void bus()
{
    //Bodi
    glColor3f(0.0, 0.8, 1.0);// menentukan warna biru muda untuk body bus
    glPushMatrix();//untuk menyimpan koordinat yang telah dibuat

    glTranslatef(0.0, 3.8, 0.0);//untuk mentranslasi / memindahkan posisi objek
    glScalef(4.0, 1.0, 1.5);//untuk mengubah ukuran(memperbesar/memperkecil) objek
    glutSolidCube(6.0f);//membuat objek kubus dengan ukuran objek 6
    glPopMatrix();//untuk memanggil fungsi - fungsi yang telah dibuat pada glPushMatrix()

    //Kaca Belakang
    glColor3f(0.0, 0.0, 1.0);// menentukan warna biru untuk kaca belakang bus
    glPushMatrix();//untuk menyimpan koordinat yang telah dibuat
    glTranslatef(-12.0, 5.0, 0.0);//untuk mentranslasi / memindahkan posisi objek
    glScalef(0.05, 0.5, 1.2);//untuk mengubah ukuran(memperbesar/memperkecil) objek
    glutSolidCube(6.0f);//membuat objek kubus dengan ukuran objek 6
    glPopMatrix();//untuk memanggil fungsi - fungsi yang telah dibuat pada glPushMatrix()

    //Lampu depan
    glColor3f(1.0, 0.0, 0.0);// menentukan warna merah untuk lampu bus
    glPushMatrix();//untuk menyimpan koordinat yang telah dibuat
    glTranslatef(-12.1, 2.0, -3.5);//untuk mentranslasi / memindahkan posisi objek
    glScalef(0.02, 0.19, 0.08);//untuk mengubah ukuran(memperbesar/memperkecil) objek
    glutSolidCube(6.0f);//membuat objek kubus dengan ukuran objek 6
    glPopMatrix();//untuk memanggil fungsi - fungsi yang telah dibuat pada glPushMatrix()

    glColor3f(1.0, 0.0, 0.0);// menentukan warna merah untuk lampu bus
    glPushMatrix();//untuk menyimpan koordinat yang telah dibuat
    glTranslatef(-12.1, 2.0, 3.5);//untuk mentranslasi / memindahkan posisi objek
    glScalef(0.02, 0.19, 0.08);//untuk mengubah ukuran(memperbesar/memperkecil) objek
    glutSolidCube(6.0f);//membuat objek kubus dengan ukuran objek 6
    glPopMatrix();//untuk memanggil fungsi - fungsi yang telah dibuat pada glPushMatrix()

    //Kaca Depan
    glColor3f(0.0, 0.0, 1.0);// menentukan warna biru untuk lampu bus
    glPushMatrix();//untuk menyimpan koordinat yang telah dibuat
    glTranslatef(12.0, 5.0, 0.0);//untuk mentranslasi / memindahkan posisi objek
    glScalef(0.05, 0.5, 1.2);//untuk mengubah ukuran(memperbesar/memperkecil) objek
    glutSolidCube(6.0f);//membuat objek kubus dengan ukuran objek 6
    glPopMatrix();//untuk memanggil fungsi - fungsi yang telah dibuat pada glPushMatrix()

    //Kaca pinggir
    glColor3f(0.0, 0.0, 1.0);// menentukan warna biru untuk lampu bus
    glPushMatrix();//untuk menyimpan koordinat yang telah dibuat
    glTranslatef(-1.5, 5.0, 4.0);//untuk mentranslasi / memindahkan posisi objek
    glScalef(3.3, 0.5, 0.2);//untuk mengubah ukuran(memperbesar/memperkecil) objek
    glutSolidCube(6.0f);//membuat objek kubus dengan ukuran objek 6
    glPopMatrix();//untuk memanggil fungsi - fungsi yang telah dibuat pada glPushMatrix()

    glColor3f(0.0, 0.0, 1.0);// menentukan warna biru untuk lampu bus
    glPushMatrix();//untuk menyimpan koordinat yang telah dibuat
    glTranslatef(-1.5, 5.0, -4.0);//untuk mentranslasi / memindahkan posisi objek
    glScalef(3.3, 0.5, 0.2);//untuk mengubah ukuran(memperbesar/memperkecil) objek
    glutSolidCube(6.0f);//membuat objek kubus dengan ukuran objek 6
    glPopMatrix();//untuk memanggil fungsi - fungsi yang telah dibuat pada glPushMatrix()

    //Ban Belakang
    glPushMatrix();//untuk menyimpan koordinat yang telah dibuat
    glColor3f(1.0, 1.0, 1.0);// menentukan warna putih untuk lampu bus
    glScalef(0.35, 0.35, 0.25);//untuk mengubah ukuran(memperbesar/memperkecil) objek
    glTranslatef(25.0, 4.5, 19.3);//untuk mentranslasi / memindahkan posisi objek
    glutSolidTorus(2, 3, 20, 30);//membuat objek donat, 2 sebagai ukuran dari solidTorus
    glPopMatrix();//untuk memanggil fungsi - fungsi yang telah dibuat pada glPushMatrix()

    //Velg
    glPushMatrix();//untuk menyimpan koordinat yang telah dibuat
    glColor3f(0.0, 0.0, 0.0);// menentukan warna hitam untuk lampu bus
    glScalef(0.5, 0.5, 0.5);//untuk mengubah ukuran(memperbesar/memperkecil) objek
    glTranslatef(17.5, 2.5, 8.0);//untuk mentranslasi / memindahkan posisi objek
    glutSolidSphere(2, 10, 20);//membuat objek bola
    glPopMatrix();//untuk memanggil fungsi - fungsi yang telah dibuat pada glPushMatrix()

    //ban
    glPushMatrix();//untuk menyimpan koordinat yang telah dibuat
    glColor3f(1.0, 1.0, 1.0);// menentukan warna putih untuk lampu bus
    glScalef(0.35, 0.35, 0.25);//untuk mengubah ukuran(memperbesar/memperkecil) objek
    glTranslatef(25.0, 4.5, -19.3);//untuk mentranslasi / memindahkan posisi objek
    glutSolidTorus(2, 3, 20, 30);//membuat objek donat, 2 sebagai ukuran dari solidTorus
    glPopMatrix();//untuk memanggil fungsi - fungsi yang telah dibuat pada glPushMatrix()

    //Velg
    glPushMatrix();//untuk menyimpan koordinat yang telah dibuat
    glColor3f(0.0, 0.0, 0.0);// menentukan warna hitam untuk lampu bus
    glScalef(0.5, 0.5, 0.5);//untuk mengubah ukuran(memperbesar/memperkecil) objek
    glTranslatef(-17.5, 2.5, 8.0);//untuk mentranslasi / memindahkan posisi objek
    glutSolidSphere(2, 10, 20);//membuat objek bola
    glPopMatrix();//untuk memanggil fungsi - fungsi yang telah dibuat pada glPushMatrix()


    //Ban Depan
    glPushMatrix();//untuk menyimpan koordinat yang telah dibuat
    glColor3f(1.0, 1.0, 1.0);// menentukan warna putih untuk lampu bus
    glScalef(0.35, 0.35, 0.25);//untuk mengubah ukuran(memperbesar/memperkecil) objek
    glTranslatef(-25.0, 4.5, -19.3);//untuk mentranslasi / memindahkan posisi objek
    glutSolidTorus(2, 3, 20, 30);//membuat objek donat, 2 sebagai ukuran dari solidTorus
    glPopMatrix();//untuk memanggil fungsi - fungsi yang telah dibuat pada glPushMatrix()

    //Velg
    glPushMatrix();//untuk menyimpan koordinat yang telah dibuat
    glColor3f(0.0, 0.0, 0.0);// menentukan warna hitam untuk lampu bus
    glScalef(0.5, 0.5, 0.5);//untuk mengubah ukuran(memperbesar/memperkecil) objek
    glTranslatef(17.5, 2.5, -8.0);//untuk mentranslasi / memindahkan posisi objek
    glutSolidSphere(2, 10, 20);//membuat objek bola
    glPopMatrix();//untuk memanggil fungsi - fungsi yang telah dibuat pada glPushMatrix()

    //ban
    glPushMatrix();//untuk menyimpan koordinat yang telah dibuat
    glColor3f(1.0, 1.0, 1.0);// menentukan warna putih untuk lampu bus
    glScalef(0.35, 0.35, 0.25);//untuk mengubah ukuran(memperbesar/memperkecil) objek
    glTranslatef(-25.0, 4.5, 19.3);//untuk mentranslasi / memindahkan posisi objek
    glutSolidTorus(2, 3, 20, 30);//membuat objek donat, 2 sebagai ukuran dari solidTorus
    glPopMatrix();//untuk memanggil fungsi - fungsi yang telah dibuat pada glPushMatrix()

    //Velg
    glPushMatrix();//untuk menyimpan koordinat yang telah dibuat
    glColor3f(0.0, 0.0, 0.0);// menentukan warna hitam untuk lampu bus
    glScalef(0.5, 0.5, 0.5);//untuk mengubah ukuran(memperbesar/memperkecil) objek
    glTranslatef(-17.5, 2.5, -8.0);//untuk mentranslasi / memindahkan posisi objek
    glutSolidSphere(2, 10, 20);//membuat objek bola
    glPopMatrix();//untuk memanggil fungsi - fungsi yang telah dibuat pada glPushMatrix()
}


void display(void) {

	glClearStencil(0); //untuk membersihkan stencil buffer
	glClearDepth(1.0f);// untuk membersihkan depth buffer
	glClearColor(0.0, 0.6, 0.8, 1);//warna latar belakang dengan 4 komponen warna RGBAlpha dengan hasil kombinasi warna menjadi biru muda
	//GL_COLOR_BUFFER_BIT --> untuk menghapus window dan memberi warna yang telah kita definisikan sebelumnya dengan menggunakan glClearColor.
	//GL_DEPTH_BUFFER_BIT --> untuk menghapus window dan memberi kedalaman yang telah kita definisikan sebelumnya dengan menggunakan glClearColor.
    //GL_DEPTH_BUFFER_BIT --> untuk menghapus window dan memberi stencil yang telah kita definisikan sebelumnya dengan menggunakan glClearColor.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); //membersihkan semua piksel
	glLoadIdentity();//memproses pembuatan gambar
	gluLookAt(viewx, viewy, viewz, 0.0, 0.0, 5.0, 0.0, 1.0, 0.0);//untuk menentukan sudut pandang objek. terdiri 3 jenis sudut pandang yaitu posisi depan belakang, posisi samping kiri kanan dan rotasi.

	glPushMatrix();
	drawSceneTanah(_terrain, 0.3f, 0.9f, 0.0f);
    glPopMatrix();

	glPushMatrix();
	drawSceneTanah(_terrainStreet, 215.0f/255.0f, 208.0f/255.0f, 134.0f/255.0f);
	glPopMatrix();

    glPushMatrix();
	drawSceneTanah(_terrainKolam, 0.0f/255.0f, 128.0f/255.0f, 192.0f/255.0f);
	glPopMatrix();

	glPushMatrix();
	drawSceneTanah(_terrainAir, 0.4902f, 0.4683f,0.4594f);
	glPopMatrix();


// dis pohon
    for(int p=-200; p<=180; p=p+30)
    {
        glPushMatrix();//untuk menyimpan koordinat yang telah dibuat
        glTranslatef(p,0,-180);//untuk mentranslasi / memindahkan posisi objek
        glScalef(10, 20, 10);//untuk mengubah ukuran(memperbesar/memperkecil) objek
        pohon();
        glPopMatrix();
    }

    // dis pohon baris ke 2
    for(int p=-200; p<=180; p=p+30)
    {
        glPushMatrix();//untuk menyimpan koordinat yang telah dibuat
        glTranslatef(p,0,-160);//untuk mentranslasi / memindahkan posisi objek
        glScalef(10, 20, 10);//untuk mengubah ukuran(memperbesar/memperkecil) objek
        pohon();
        glPopMatrix();
    }
    // dis pohon baris ke 3
    for(int p=-200; p<=180; p=p+30)
    {
        glPushMatrix();//untuk menyimpan koordinat yang telah dibuat
        glTranslatef(p,0,-140);//untuk mentranslasi / memindahkan posisi objek
        glScalef(10, 20, 10);//untuk mengubah ukuran(memperbesar/memperkecil) objek
        pohon();
        glPopMatrix();
    }

    // dis pohon kolom 1
    for(int p2=-90; p2<=50; p2=p2+30)
    {
        glPushMatrix();//untuk menyimpan koordinat yang telah dibuat
        glTranslatef(100,0,p2);//untuk mentranslasi / memindahkan posisi objek
        glScalef(10, 20, 10);//untuk mengubah ukuran(memperbesar/memperkecil) objek
        pohon();
        glPopMatrix();
    }

    for(int p2=-90; p2<=50; p2=p2+30)
    {
        glPushMatrix();//untuk menyimpan koordinat yang telah dibuat
        glTranslatef(125,0,p2);//untuk mentranslasi / memindahkan posisi objek
        glScalef(10, 20, 10);//untuk mengubah ukuran(memperbesar/memperkecil) objek
        pohon();
        glPopMatrix();
    }

    for(int p2=-90; p2<=50; p2=p2+30)
    {
        glPushMatrix();//untuk menyimpan koordinat yang telah dibuat
        glTranslatef(150,0,p2);//untuk mentranslasi / memindahkan posisi objek
        glScalef(10, 20, 10);//untuk mengubah ukuran(memperbesar/memperkecil) objek
        pohon();
        glPopMatrix();
    }

    // dis pohon kolom 2
    for(int p2=80; p2<=150; p2=p2+30)
    {
        glPushMatrix();//untuk menyimpan koordinat yang telah dibuat
        glTranslatef(100,0,p2);//untuk mentranslasi / memindahkan posisi objek
        glScalef(10, 20, 10);//untuk mengubah ukuran(memperbesar/memperkecil) objek
        pohon();
        glPopMatrix();
    }

    for(int p2=80; p2<=150; p2=p2+30)
    {
        glPushMatrix();//untuk menyimpan koordinat yang telah dibuat
        glTranslatef(125,0,p2);//untuk mentranslasi / memindahkan posisi objek
        glScalef(10, 20, 10);//untuk mengubah ukuran(memperbesar/memperkecil) objek
        pohon();
        glPopMatrix();
    }

    for(int p2=80; p2<=150; p2=p2+30)
    {
        glPushMatrix();//untuk menyimpan koordinat yang telah dibuat
        glTranslatef(150,0,p2);//untuk mentranslasi / memindahkan posisi objek
        glScalef(10, 20, 10);//untuk mengubah ukuran(memperbesar/memperkecil) objek
        pohon();
        glPopMatrix();
    }

    //dis pohon paling kanan
    for(int p2=-180; p2<=180; p2=p2+30)
    {
        glPushMatrix();//untuk menyimpan koordinat yang telah dibuat
        glTranslatef(240,0,p2);//untuk mentranslasi / memindahkan posisi objek
        glScalef(10, 20, 10);//untuk mengubah ukuran(memperbesar/memperkecil) objek
        pohon();
        glPopMatrix();
    }
    //dis rumah hadap depan
	glPushMatrix();
	glTranslated(30,0,-10);//untuk mentranslasi / memindahkan posisi objek
    glScaled(20, 50, 20);//untuk mengubah ukuran(memperbesar/memperkecil) objek
	rumah();
	glPopMatrix();

    glPushMatrix();
    glTranslatef(-23,0,-80);//untuk mentranslasi / memindahkan posisi objek
    pagarjadi();
    glPopMatrix();

    //dis rumah 2
    glPushMatrix();
	glTranslated(30,0,80);
    glScaled(20, 50, 20);//untuk mengubah ukuran(memperbesar/memperkecil) objek
	rumah();
	glPopMatrix();

    glPushMatrix();
    glTranslatef(-23,0,5);//untuk mentranslasi / memindahkan posisi objek
    pagarjadi();
    glPopMatrix();

    //dis rumah hadap kanan
    glPushMatrix();
	glTranslated(-70,0,-160);
    glScaled(20, 50, 20);//untuk mengubah ukuran(memperbesar/memperkecil) objek
    glRotated(90,0,1,0);
	rumah();
	glPopMatrix();

	glPushMatrix();
    glTranslatef(-140,0,-110);//untuk mentranslasi / memindahkan posisi objek
    glRotated(90,0,1,0);
    pagarjadi();
    glPopMatrix();

    //dis rumah kanan 2
    glPushMatrix();
	glTranslated(-70,0,-90);//untuk mentranslasi / memindahkan posisi objek
    glScaled(20, 50, 20);//untuk mengubah ukuran(memperbesar/memperkecil) objek
    glRotated(90,0,1,0);
	rumah();
	glPopMatrix();

	glPushMatrix();
    glTranslatef(-140,0,-37);
    glRotated(90,0,1,0);
    pagarjadi();
    glPopMatrix();

    // dis rumah kanan 3
    glPushMatrix();
	glTranslated(105,0,-180);//untuk mentranslasi / memindahkan posisi objek
    glScaled(20, 50, 20);//untuk mengubah ukuran(memperbesar/memperkecil) objek
    glRotated(90,0,1,0);
	rumah();
	glPopMatrix();

	glPushMatrix();
    glTranslatef(35,0,-130);//untuk mentranslasi / memindahkan posisi objek
    glRotated(90,0,1,0);
    pagarjadi();
    glPopMatrix();

    // dis rumah kanan 3
    glPushMatrix();
	glTranslated(105,0,-85);//untuk mentranslasi / memindahkan posisi objek
    glScaled(20, 50, 20);//untuk mengubah ukuran(memperbesar/memperkecil) objek
    glRotated(90,0,1,0);
	rumah();
	glPopMatrix();

	glPushMatrix();
    glTranslatef(35,0,-35);//untuk mentranslasi / memindahkan posisi objek
    glRotated(90,0,1,0);
    pagarjadi();
    glPopMatrix();

//dis bendera
    glDisable(GL_CULL_FACE);
    glPushMatrix();
    glTranslated(-70,15,110);
    glScalef(1.5,1.5,1.5);//untuk mengubah ukuran(memperbesar/memperkecil) objek
    bendera();
    glPopMatrix();

//dis bus
    glPushMatrix();//untuk menyimpan koordinat yang telah dibuat
    //bus gerak
    if (gerak<=500)
    {
        glTranslatef(18, 0, gerak);
        gerak+=1;
        if (gerak==360)//batas jalan pojok
            {gerak=27.0;}// posisi awal mobil
    }
    glTranslated(190,0,-200);//untuk mentranslasi / memindahkan posisi objek
    glScaled(2,2,2);//untuk mengubah ukuran(memperbesar/memperkecil) objek
    glRotated(90,0,1,0);
    bus();
    glPopMatrix();

     //Penggambaran awan
    glPushMatrix();
    glTranslatef(0, 100, -150);//penentuan usulan
    glScalef(1.8, 1.0, 1.0);//penskalaan sesuai ukuran
    awan();//pemanggilan procedure awan
    glPopMatrix();

    glPushMatrix();
    glTranslatef(30, 100, -150);
    glScalef(1.8, 1.0, 1.0);
    awan();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-100, 100, -130);
    glScalef(1.8, 1.0, 1.0);
    awan();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-120, 100, -130);
    glScalef(1.8, 1.0, 1.0);
    awan();
    glPopMatrix();

    // dis bunga
// belakang rumah
    for(int b=-110; b<=20; b=b+50)
    {
        glPushMatrix();//untuk menyimpan koordinat yang telah dibuat
        glTranslated(-200,-20,b);//untuk mentranslasi / memindahkan posisi objek
        glScaled(2,2,2);//untuk mengubah ukuran(memperbesar/memperkecil) objek
        bunga();
        glPopMatrix();
    }

    for(int b=-100; b<=20; b=b+50)
    {
        glPushMatrix();//untuk menyimpan koordinat yang telah dibuat
        glTranslated(-200,-30,b);//untuk mentranslasi / memindahkan posisi objek
        glScaled(3,3,3);//untuk mengubah ukuran(memperbesar/memperkecil) objek
        bunga();
        glPopMatrix();
    }


    for(int b=-90; b<=20; b=b+50)
    {
        glPushMatrix();//untuk menyimpan koordinat yang telah dibuat
        glTranslated(-200,-20,b);//untuk mentranslasi / memindahkan posisi objek
        glScaled(2,2,2);//untuk mengubah ukuran(memperbesar/memperkecil) objek
        bunga();
        glPopMatrix();
    }

// belakang sungai
    glPushMatrix();//untuk menyimpan koordinat yang telah dibuat
    glTranslated(-200,-30,150);//untuk mentranslasi / memindahkan posisi objek
    glScaled(3,3,3);//untuk mengubah ukuran(memperbesar/memperkecil) objek
    bunga();
    glPopMatrix();

    glPushMatrix();//untuk menyimpan koordinat yang telah dibuat
    glTranslated(-50,-20,150);//untuk mentranslasi / memindahkan posisi objek
    glScaled(2,2,2);//untuk mengubah ukuran(memperbesar/memperkecil) objek
    bunga();
    glPopMatrix();

    glPushMatrix();//untuk menyimpan koordinat yang telah dibuat
    glTranslated(-40,-20,150);//untuk mentranslasi / memindahkan posisi objek
    glScaled(2,2,2);//untuk mengubah ukuran(memperbesar/memperkecil) objek
    bunga();
    glPopMatrix();

//deket jalan
    glPushMatrix();//untuk menyimpan koordinat yang telah dibuat
    glTranslated(160,-20,160);//untuk mentranslasi / memindahkan posisi objek
    glScaled(2,2,2);//untuk mengubah ukuran(memperbesar/memperkecil) objek
    bunga();
    glPopMatrix();

    glPushMatrix();//untuk menyimpan koordinat yang telah dibuat
    glTranslated(170,-30,160);//untuk mentranslasi / memindahkan posisi objek
    glScaled(3,3,3);//untuk mengubah ukuran(memperbesar/memperkecil) objek
    bunga();
    glPopMatrix();

    glPushMatrix();//untuk menyimpan koordinat yang telah dibuat
    glTranslated(180,-20,160);//untuk mentranslasi / memindahkan posisi objek
    glScaled(2,2,2);//untuk mengubah ukuran(memperbesar/memperkecil) objek
    bunga();
    glPopMatrix();

// depan sungai
    glPushMatrix();//untuk menyimpan koordinat yang telah dibuat
    glTranslated(60,-30,175);//untuk mentranslasi / memindahkan posisi objek
    glScaled(3,3,3);//untuk mengubah ukuran(memperbesar/memperkecil) objek
    bunga();
    glPopMatrix();

    glPushMatrix();//untuk menyimpan koordinat yang telah dibuat
    glTranslated(50,-20,170);//untuk mentranslasi / memindahkan posisi objek
    glScaled(2,2,2);//untuk mengubah ukuran(memperbesar/memperkecil) objek
    bunga();
    glPopMatrix();

	glutSwapBuffers();
	glFlush();
	rot++;
	angle++;

}





void init(void) {
    glEnable(GL_TEXTURE_2D);
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
	
	//Inisialisais terrain dengan mengambil gambar sesuai dengan nama file
	_terrain = loadTerrain("heightmap.bmp", 20);
	_terrainStreet = loadTerrain("heightmapstreet.bmp", 20);
	_terrainAir = loadTerrain("heightmapAir.bmp", 20);
    _terrainKolam = loadTerrain("heightmapkolam.bmp", 20);

	//binding texture
	Gambar *gambar1 = loadTexture();
	Gambar *gambar2 = loadTexture2();
    Gambar *gambar3 = loadTexture3();
    Gambar *gambar4 = loadTexture4();


	if (gambar1 == NULL) {
		printf("Gambar was not returned from loadTexture\n");
		exit(0);
	}

		if (gambar2 == NULL) {
		printf("Gambar was not returned from loadTexture\n");
		exit(0);
	}

		if (gambar3 == NULL) {
		printf("Gambar was not returned from loadTexture\n");
		exit(0);
    }
        if (gambar4 == NULL) {
		printf("Gambar was not returned from loadTexture\n");
		exit(0);
    }

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(3, texture);
	//binding texture untuk membuat texture 2D
	glBindTexture(GL_TEXTURE_2D, texture[0]);//menempelkan gambar dari texture ke objek, texture array ke 0 yaitu atap.bmp
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, gambar1->sizeX, gambar1->sizeY, 0, GL_RGB,
    GL_UNSIGNED_BYTE, gambar1->data);

	//binding texture untuk membuat texture 2D
	glBindTexture(GL_TEXTURE_2D, texture[1]);//menempelkan gambar dari texture ke objek, texture array ke 1 yaitu dinding.bmp
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, gambar2->sizeX, gambar2->sizeY, 0, GL_RGB,
    GL_UNSIGNED_BYTE, gambar2->data);

    glBindTexture(GL_TEXTURE_2D, texture[2]);//menempelkan gambar dari texture ke objek, texture array ke 2 yaitu pintu.bmp
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, gambar3->sizeX, gambar3->sizeY, 0, GL_RGB,
    GL_UNSIGNED_BYTE, gambar3->data);

	//binding texture untuk membuat texture 2D
	glBindTexture(GL_TEXTURE_2D, texture[3]);//menempelkan gambar dari texture ke objek, texture array ke 3 yaitu bunga.bmp
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, gambar4->sizeX, gambar4->sizeY, 0, GL_RGB,
	GL_UNSIGNED_BYTE, gambar4->data);

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
	gluPerspective(60, (GLfloat) w / (GLfloat) h, 0.1, 1000.0);//proyeksi sudut pandang 60 derajat
	glMatrixMode(GL_MODELVIEW);
}

int main(int argc, char **argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_STENCIL | GLUT_DEPTH); //add a stencil buffer to the window
	glutInitWindowSize(800, 600); // parameter ukuran window (panjang,lebar)
	glutInitWindowPosition(100, 100);//parameter posisi window (x,y)
	glutCreateWindow("TUGAS BESAR");//membuat window tugas besar
	init();

	glutDisplayFunc(display);//fungsi utama untuk menampilkan objek
	glutIdleFunc(display);//fungsi yang dijalankan dalam keadaan idle
	glutReshapeFunc(reshape);//untuk memanggil fungsi reshape
	glutSpecialFunc(kibor);

	glutKeyboardFunc(keyboard);//fungsi untuk mengatur arah kamera

	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);

	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, high_shininess);
	glColorMaterial(GL_FRONT, GL_DIFFUSE);

	glutMainLoop();
	return 0;
}
