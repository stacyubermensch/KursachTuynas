#include "Render.h"

#include <sstream>
#include <iostream>

#include <windows.h>
#include <GL\GL.h>
#include <GL\GLU.h>

#include "MyOGL.h"

#include "Camera.h"
#include "Light.h"
#include "Primitives.h"

#include "GUItextRectangle.h"

bool textureMode = true;
bool lightMode = true;

//����� ��� ��������� ������
class CustomCamera : public Camera
{
public:
	//��������� ������
	double camDist;
	//���� �������� ������
	double fi1, fi2;

	
	//������� ������ �� ���������
	CustomCamera()
	{
		camDist = 15;
		fi1 = 1;
		fi2 = 1;
	}

	
	//������� ������� ������, ������ �� ����� ��������, ���������� �������
	void SetUpCamera()
	{
		//�������� �� ������� ������ ������
		lookPoint.setCoords(0, 0, 0);

		pos.setCoords(camDist*cos(fi2)*cos(fi1),
			camDist*cos(fi2)*sin(fi1),
			camDist*sin(fi2));

		if (cos(fi2) <= 0)
			normal.setCoords(0, 0, -1);
		else
			normal.setCoords(0, 0, 1);

		LookAt();
	}

	void CustomCamera::LookAt()
	{
		//������� ��������� ������
		gluLookAt(pos.X(), pos.Y(), pos.Z(), lookPoint.X(), lookPoint.Y(), lookPoint.Z(), normal.X(), normal.Y(), normal.Z());
	}



}  camera;   //������� ������ ������


//����� ��� ��������� �����
class CustomLight : public Light
{
public:
	CustomLight()
	{
		//��������� ������� �����
		pos = Vector3(1, 1, 3);
	}

	
	//������ ����� � ����� ��� ���������� �����, ���������� �������
	void  DrawLightGhismo()
	{
		glDisable(GL_LIGHTING);

		
		glColor3d(0.9, 0.8, 0);
		Sphere s;
		s.pos = pos;
		s.scale = s.scale*0.08;
		s.Show();
		
		if (OpenGL::isKeyPressed('G'))
		{
			glColor3d(0, 0, 0);
			//����� �� ��������� ����� �� ����������
			glBegin(GL_LINES);
			glVertex3d(pos.X(), pos.Y(), pos.Z());
			glVertex3d(pos.X(), pos.Y(), 0);
			glEnd();

			//������ ���������
			Circle c;
			c.pos.setCoords(pos.X(), pos.Y(), 0);
			c.scale = c.scale*1.5;
			c.Show();
		}

	}

	void SetUpLight()
	{
		GLfloat amb[] = { 0.2, 0.2, 0.2, 0 };
		GLfloat dif[] = { 1.0, 1.0, 1.0, 0 };
		GLfloat spec[] = { .7, .7, .7, 0 };
		GLfloat position[] = { pos.X(), pos.Y(), pos.Z(), 1. };

		// ��������� ��������� �����
		glLightfv(GL_LIGHT0, GL_POSITION, position);
		// �������������� ����������� �����
		// ������� ��������� (���������� ����)
		glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
		// ��������� ������������ �����
		glLightfv(GL_LIGHT0, GL_DIFFUSE, dif);
		// ��������� ���������� ������������ �����
		glLightfv(GL_LIGHT0, GL_SPECULAR, spec);

		glEnable(GL_LIGHT0);
	}


} light;  //������� �������� �����




//������ ���������� ����
int mouseX = 0, mouseY = 0;

void mouseEvent(OpenGL *ogl, int mX, int mY)
{
	int dx = mouseX - mX;
	int dy = mouseY - mY;
	mouseX = mX;
	mouseY = mY;

	//������ ���� ������ ��� ������� ����� ������ ����
	if (OpenGL::isKeyPressed(VK_RBUTTON))
	{
		camera.fi1 += 0.01*dx;
		camera.fi2 += -0.01*dy;
	}

	
	//������� ���� �� ���������, � ����� ��� ����
	if (OpenGL::isKeyPressed('G') && !OpenGL::isKeyPressed(VK_LBUTTON))
	{
		LPPOINT POINT = new tagPOINT();
		GetCursorPos(POINT);
		ScreenToClient(ogl->getHwnd(), POINT);
		POINT->y = ogl->getHeight() - POINT->y;

		Ray r = camera.getLookRay(POINT->x, POINT->y);

		double z = light.pos.Z();

		double k = 0, x = 0, y = 0;
		if (r.direction.Z() == 0)
			k = 0;
		else
			k = (z - r.origin.Z()) / r.direction.Z();

		x = k*r.direction.X() + r.origin.X();
		y = k*r.direction.Y() + r.origin.Y();

		light.pos = Vector3(x, y, z);
	}

	if (OpenGL::isKeyPressed('G') && OpenGL::isKeyPressed(VK_LBUTTON))
	{
		light.pos = light.pos + Vector3(0, 0, 0.02*dy);
	}

	
}

void mouseWheelEvent(OpenGL *ogl, int delta)
{

	if (delta < 0 && camera.camDist <= 1)
		return;
	if (delta > 0 && camera.camDist >= 100)
		return;

	camera.camDist += 0.01*delta;

}

void keyDownEvent(OpenGL *ogl, int key)
{
	if (key == 'L')
	{
		lightMode = !lightMode;
	}

	if (key == 'T')
	{
		textureMode = !textureMode;
	}

	if (key == 'R')
	{
		camera.fi1 = 1;
		camera.fi2 = 1;
		camera.camDist = 15;

		light.pos = Vector3(1, 1, 3);
	}

	if (key == 'F')
	{
		light.pos = camera.pos;
	}
}

void keyUpEvent(OpenGL *ogl, int key)
{
	
}



GLuint texId;

//����������� ����� ������ ��������
void initRender(OpenGL *ogl)
{
	//��������� �������

	//4 ����� �� �������� �������
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	//��������� ������ ��������� �������
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	//�������� ��������
	glEnable(GL_TEXTURE_2D);
	

	//������ ����������� ���������  (R G B)
	RGBTRIPLE *texarray;

	//������ ��������, (������*������*4      4, ���������   ����, �� ������� ������������ �� 4 ����� �� ������� �������� - R G B A)
	char *texCharArray;
	int texW, texH;
	OpenGL::LoadBMP("buldog_10.bmp", &texW, &texH, &texarray);
	OpenGL::RGBtoChar(texarray, texW, texH, &texCharArray);

	
	
	//���������� �� ��� ��������
	glGenTextures(1, &texId);
	//������ ��������, ��� ��� ����� ����������� � ���������, ����� ����������� �� ����� ��
	glBindTexture(GL_TEXTURE_2D, texId);

	//��������� �������� � �����������, � ���������� ��� ������  ��� �� �����
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW, texH, 0, GL_RGBA, GL_UNSIGNED_BYTE, texCharArray);

	//�������� ������
	free(texCharArray);
	free(texarray);

	//������� ����
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);


	//������ � ���� ����������� � "������"
	ogl->mainCamera = &camera;
	ogl->mainLight = &light;

	// ������������ �������� : �� ����� ����� ����� 1
	glEnable(GL_NORMALIZE);

	// ���������� ������������� ��� �����
	glEnable(GL_LINE_SMOOTH); 


	//   ������ ��������� ���������
	//  �������� GL_LIGHT_MODEL_TWO_SIDE - 
	//                0 -  ������� � ���������� �������� ���������(�� ���������), 
	//                1 - ������� � ���������� �������������� ������� ��������       
	//                �������������� ������� � ���������� ��������� ����������.    
	//  �������� GL_LIGHT_MODEL_AMBIENT - ������ ������� ���������, 
	//                �� ��������� �� ���������
	// �� ��������� (0.2, 0.2, 0.2, 1.0)

	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 0);

	camera.fi1 = -1.3;
	camera.fi2 = 0.8;
}

double* normalno(double x1, double y1, double x2, double y2)
{

	struct vector
	{
		double x;
		double y;
		double z;
	} AC, AB, Normal;

	AC.x = x2 - x1; AC.y = y2 - y1; AC.z = 0;
	AB.x = 0; AB.y = 0; AB.z = 3;

	Normal.x = AC.y * AB.z - AB.y * AC.z;
	Normal.y = -(AC.x * AB.z - AB.x * AC.z);
	Normal.z = AC.x * AB.y - AB.x * AC.y;

	double length = sqrt(Normal.x * Normal.x + Normal.y * Normal.y + Normal.z * Normal.z);
	Normal.x = Normal.x / length;
	Normal.y = Normal.y / length;
	Normal.z = Normal.z / length;

	double finalNormal[3] = { Normal.x, Normal.y, Normal.z };
	return finalNormal;
}


void rybka()
{




	double A[] = { 6, 3, 0 };
	double B[] = { 5, 9, 0 };
	double C[] = { 1, 12, 0 };
	double D[] = { 6, 11, 0 };
	double E[] = { 11, 16, 0 };
	double F[] = { 19, 13, 0 };
	double G[] = { 15, 11, 0 };
	double H[] = { 20, 8, 0 };
	double I[] = { 7, 8, 0 };
	double M[] = { 9, 12, 0 };
	double N[] = { 9, 11, 0 };
	double A1[] = { 6, 3, 3 };
	double B1[] = { 5, 9, 3 };
	double C1[] = { 1, 12, 3 };
	double D1[] = { 6, 11, 3 };
	double E1[] = { 11, 16, 3 };
	double F1[] = { 19, 13, 3 };
	double G1[] = { 15, 11, 3 };
	double H1[] = { 20, 8, 3 };
	double I1[] = { 7, 8, 3 };
	double M1[] = { 9, 12, 3 };
	double N1[] = { 9, 11, 3 };

	//������ �����

	glBindTexture(GL_TEXTURE_2D, texId);
	glBegin(GL_TRIANGLES);
	glColor3d(1, 0, 1);
	//����������� ABI
	glNormal3d(0, 0, -1);
	glColor3d(1, 0, 1);
	glTexCoord2d(0.3, 0.15);
	glVertex3dv(A);
	glTexCoord2d(0.25, 0.45);
	glVertex3dv(B);
	glTexCoord2d(0.35, 0.4);
	glVertex3dv(I);

	//����������� BCD
	glNormal3d(0, 0, -1);
	glColor3d(1, 0, 1);
	glTexCoord2d(0.25, 0.45);
	glVertex3dv(B);
	glTexCoord2d(0.05, 0.6);
	glVertex3dv(C);
	glTexCoord2d(0.3, 0.55);
	glVertex3dv(D);


	//����������� BDI
	glNormal3d(0, 0, -1);
	glColor3d(1, 0, 1);
	glTexCoord2d(0.25, 0.45);
	glVertex3dv(B);
	glTexCoord2d(0.3, 0.55);
	glVertex3dv(D);
	glTexCoord2d(0.35, 0.4);
	glVertex3dv(I);


	//����������� DEG
	glNormal3d(0, 0, -1);
	glColor3d(1, 0, 1);
	glTexCoord2d(0.3, 0.55);
	glVertex3dv(D);
	glTexCoord2d(0.55, 0.8);
	glVertex3dv(E);
	glTexCoord2d(0.75, 0.55);
	glVertex3dv(G);

	//����������� EGF
	glNormal3d(0, 0, -1);
	glColor3d(1, 0, 1);
	glTexCoord2d(0.55, 0.8);
	glVertex3dv(E);
	glTexCoord2d(0.75, 0.55);
	glVertex3dv(G);
	glTexCoord2d(0.95, 0.65);
	glVertex3dv(F);

	//����������� IGH
	glNormal3d(0, 0, -1);
	glColor3d(1, 0, 1);
	glTexCoord2d(0.35, 0.4);
	glVertex3dv(I);
	glTexCoord2d(0.75, 0.55);
	glVertex3dv(G);
	glTexCoord2d(1, 0.4);
	glVertex3dv(H);

	//����������� DIG
	glNormal3d(0, 0, -1);
	glColor3d(1, 0, 1);
	glTexCoord2d(0.3, 0.55);
	glVertex3dv(D);
	glTexCoord2d(0.35, 0.4);
	glVertex3dv(I);
	glTexCoord2d(0.75, 0.55);
	glVertex3dv(G);

	



	////////////������� �����////////////////////

		//����������� ABI
	glNormal3d(0, 0, 1);
	glColor3d(1, 0, 1);
	glTexCoord2d(0.3, 0.15);
	glVertex3dv(A1);
	glTexCoord2d(0.25, 0.45);
	glVertex3dv(B1);
	glTexCoord2d(0.35, 0.4);
	glVertex3dv(I1);

	//����������� BCD
	glNormal3d(0, 0, 1);
	glColor3d(1, 0, 1);
	glTexCoord2d(0.25, 0.45);
	glVertex3dv(B1);
	glTexCoord2d(0.05, 0.6);
	glVertex3dv(C1);
	glTexCoord2d(0.3, 0.55);
	glVertex3dv(D1);


	//����������� BDI
	glNormal3d(0, 0, 1);
	glColor3d(1, 0, 1);
	glTexCoord2d(0.25, 0.45);
	glVertex3dv(B1);
	glTexCoord2d(0.3, 0.55);
	glVertex3dv(D1);
	glTexCoord2d(0.35, 0.4);
	glVertex3dv(I1);


	//����������� DEG
	glNormal3d(0, 0, 1);
	glColor3d(1, 0, 1);
	glTexCoord2d(0.3, 0.55);
	glVertex3dv(D1);
	glTexCoord2d(0.55, 0.8);
	glVertex3dv(E1);
	glTexCoord2d(0.75, 0.55);
	glVertex3dv(G1);

	//����������� EGF
	glNormal3d(0, 0, 1);
	glColor3d(1, 0, 1);
	glTexCoord2d(0.55, 0.8);
	glVertex3dv(E1);
	glTexCoord2d(0.75, 0.55);
	glVertex3dv(G1);
	glTexCoord2d(0.95, 0.65);
	glVertex3dv(F1);

	//����������� IGH
	glNormal3d(0, 0, 1);
	glColor3d(1, 0, 1);
	glTexCoord2d(0.35, 0.4);
	glVertex3dv(I1);
	glTexCoord2d(0.75, 0.55);
	glVertex3dv(G1);
	glTexCoord2d(1, 0.4);
	glVertex3dv(H1);

	//����������� DIG
	glNormal3d(0, 0, 1);
	glColor3d(1, 0, 1);
	glTexCoord2d(0.3, 0.55);
	glVertex3dv(D1);
	glTexCoord2d(0.35, 0.4);
	glVertex3dv(I1);
	glTexCoord2d(0.75, 0.55);
	glVertex3dv(G1);

	glEnd();



	//��������



	glBegin(GL_QUADS);


	//������ AI
	glNormal3dv(normalno(6, 3, 7, 8));
	glColor3d(0, 0, 0);
	glTexCoord2d(0, 0);
	glVertex3dv(A);
	glTexCoord2d(0, 1);
	glVertex3dv(A1);
	glTexCoord2d(1, 1);
	glVertex3dv(I1);
	glTexCoord2d(1, 0);
	glVertex3dv(I);


	//������ IH
	glNormal3dv(normalno(7, 8, 20, 8));
	glColor3d(0, 0, 0);
	glTexCoord2d(0, 0);
	glVertex3dv(I);
	glTexCoord2d(0, 1);
	glVertex3dv(I1);
	glTexCoord2d(1, 1);
	glVertex3dv(H1);
	glTexCoord2d(1, 0);
	glVertex3dv(H);


	//������ HG
	glNormal3dv(normalno(20, 8, 15, 11));
	glColor3d(0, 0, 0);
	glTexCoord2d(0, 0);
	glVertex3dv(H);
	glTexCoord2d(0, 1);
	glVertex3dv(H1);
	glTexCoord2d(1, 1);
	glVertex3dv(G1);
	glTexCoord2d(1, 0);
	glVertex3dv(G);

	//������ GF
	glNormal3dv(normalno(15, 11, 19, 13));
	glColor3d(0, 0, 0);
	glTexCoord2d(0, 0);
	glVertex3dv(G);
	glTexCoord2d(0, 1);
	glVertex3dv(G1);
	glTexCoord2d(1, 1);
	glVertex3dv(F1);
	glTexCoord2d(1, 0);
	glVertex3dv(F);


	//������ FE
	glNormal3dv(normalno(19, 13, 11, 16));
	glTexCoord2d(0, 0);
	glVertex3dv(F);
	glTexCoord2d(0, 1);
	glVertex3dv(F1);
	glTexCoord2d(1, 1);
	glVertex3dv(E1);
	glTexCoord2d(1, 0);
	glVertex3dv(E);

	//������ ED
	glNormal3dv(normalno(11, 16, 6, 11));
	glColor3d(0, 0, 0);
	glTexCoord2d(0, 0);
	glVertex3dv(E);
	glTexCoord2d(0, 1);
	glVertex3dv(E1);
	glTexCoord2d(1, 1);
	glVertex3dv(D1);
	glTexCoord2d(1, 0);
	glVertex3dv(D);

	//������ DC
	glNormal3dv(normalno(6, 11, 1, 12));
	glColor3d(0, 0, 0);
	glTexCoord2d(0, 0);
	glVertex3dv(D);
	glTexCoord2d(0, 1);
	glVertex3dv(D1);
	glTexCoord2d(1, 1);
	glVertex3dv(C1);
	glTexCoord2d(1, 0);
	glVertex3dv(C);

	//������ CB
	glNormal3dv(normalno(1, 12, 5, 9));
	glColor3d(0, 0, 0);
	glTexCoord2d(0, 0);
	glVertex3dv(C);
	glTexCoord2d(0, 1);
	glVertex3dv(C1);
	glTexCoord2d(1, 1);
	glVertex3dv(B1);
	glTexCoord2d(1, 0);
	glVertex3dv(B);


	//������ BA
	glNormal3dv(normalno(5, 9, 6, 3));
	glColor3d(0, 0, 0);
	glTexCoord2d(0, 0);
	glVertex3dv(B);
	glTexCoord2d(0, 1);
	glVertex3dv(B1);
	glTexCoord2d(1, 1);
	glVertex3dv(A1);
	glTexCoord2d(1, 0);
	glVertex3dv(A);



	glEnd();

}

//���� � ���������� ���
void circle(double x, double y, double r)

{
	double rad = 0;
	double x1 = 0;
	double y1 = 0;

	double x_1 = 5;		//���������� ��������� �����
	double y_1 = 0;

	double x_text = 1;			//��� �������� ��������� �����
	double y_text = 0.5;

	double x_text1 = 0;
	double y_text1 = 0;

	double O[] = { x, y };
	double O1[] = { 0.5, 0.5 };





	for (int phi = 0; phi < 360; phi++)
	{
		rad = phi * (3.14 / 180);
		x1 = r * cos(rad) + x;	//����� �����
		y1 = r * sin(rad) + y;

		x_text1 = 0.5 * cos(rad) + 0.5;	//���������� ��������
		y_text1 = 0.5 * sin(rad) + 0.5;
		double temp[] = { x1, y1, };		//����� �����
		double temp1[] = { x_1, y_1 };			//������ �����

		double temp_text[] = { x_text, y_text, };		//������ �����
		double temp1_text[] = { x_text1, y_text1 };		//����� �����

		glBindTexture(GL_TEXTURE_2D, texId);
		glBegin(GL_TRIANGLES);

		glNormal3d(0, 0, 1);

		glTexCoord2d(0.5, 0.5);
		glVertex2dv(O);

		glTexCoord2d(x_text, y_text);
		glVertex2dv(temp1);

		glTexCoord2d(x_text1, y_text1);
		glVertex2dv(temp);


		glEnd();
		x_1 = x1;
		y_1 = y1;
		x_text = x_text1;
		y_text = y_text1;
	}
}

void rybkaalfa()

{



	double A[] = { 6, 3, 0 };
	double B[] = { 5, 9, 0 };
	double C[] = { 1, 12, 0 };
	double D[] = { 6, 11, 0 };
	double E[] = { 11, 16, 0 };
	double F[] = { 19, 13, 0 };
	double G[] = { 15, 11, 0 };
	double H[] = { 20, 8, 0 };
	double I[] = { 7, 8, 0 };
	double M[] = { 9, 12, 0 };
	double N[] = { 9, 11, 0 };
	double A1[] = { 6, 3, 3 };
	double B1[] = { 5, 9, 3 };
	double C1[] = { 1, 12, 3 };
	double D1[] = { 6, 11, 3 };
	double E1[] = { 11, 16, 3 };
	double F1[] = { 19, 13, 3 };
	double G1[] = { 15, 11, 3 };
	double H1[] = { 20, 8, 3 };
	double I1[] = { 7, 8, 3 };
	double M1[] = { 9, 12, 3 };
	double N1[] = { 9, 11, 3 };

	//������ �����

	glBegin(GL_TRIANGLES);
	
	glColor3d(1, 0, 1);
	glNormal3d(0, 0, -1);
	glVertex3dv(A);
	glVertex3dv(B);
	glVertex3dv(I);

	glNormal3d(0, 0, -1);
	glColor3d(1, 0, 1);
	glVertex3dv(B);
	glVertex3dv(C);
	glVertex3dv(D);

	glNormal3d(0, 0, -1);
	glColor3d(1, 0, 1);
	glVertex3dv(B);
	glVertex3dv(D);
	glVertex3dv(I);

	glNormal3d(0, 0, -1);
	glColor3d(1, 0, 1);
	glVertex3dv(M);
	glVertex3dv(N);
	glVertex3dv(G);

	glNormal3d(0, 0, -1);
	glColor3d(1, 0, 1);
	glVertex3dv(E);
	glVertex3dv(G);
	glVertex3dv(F);

	glNormal3d(0, 0, -1);
	glColor3d(1, 0, 1);
	glVertex3dv(I);
	glVertex3dv(G);
	glVertex3dv(H);

	glNormal3d(0, 0, -1);
	glColor3d(1, 0, 1);
	glVertex3dv(D);
	glVertex3dv(I);
	glVertex3dv(G);

	//������� �����

	glNormal3d(0, 0, 1);
	glColor4d(1, 0, 1, 0.9);
	glVertex3dv(A1);
	glVertex3dv(B1);
	glVertex3dv(I1);

	glNormal3d(0, 0, 1);
	glColor4d(1, 0, 1, 0.9);
	glVertex3dv(B1);
	glVertex3dv(C1);
	glVertex3dv(D1);

	glNormal3d(0, 0, 1);
	glColor4d(1, 0, 1, 0.9);
	glVertex3dv(B1);
	glVertex3dv(D1);
	glVertex3dv(I1);

	glNormal3d(0, 0, 1);
	glColor4d(1, 0, 1, 0.9);
	glVertex3dv(M1);
	glVertex3dv(N1);
	glVertex3dv(G1);

	glNormal3d(0, 0, 1);
	glColor4d(1, 0, 1, 0.9);
	glVertex3dv(E1);
	glVertex3dv(G1);
	glVertex3dv(F1);

	glNormal3d(0, 0, 1);
	glColor4d(1, 0, 1, 0.9);
	glVertex3dv(I1);
	glVertex3dv(G1);
	glVertex3dv(H1);

	glNormal3d(0, 0, 1);
	glColor4d(1, 0, 1, 0.9);
	glVertex3dv(D1);
	glVertex3dv(I1);
	glVertex3dv(G1);

	glEnd();



	//��������



	glBegin(GL_QUADS);

	glNormal3dv(normalno(6, 3, 7, 8));
	glColor3d(0, 0, 0);
	glVertex3dv(A);
	glVertex3dv(A1);
	glVertex3dv(I1);
	glVertex3dv(I);

	glNormal3dv(normalno(7, 8, 20, 8));
	glColor3d(0, 0, 0);
	glVertex3dv(I);
	glVertex3dv(I1);
	glVertex3dv(H1);
	glVertex3dv(H);

	glNormal3dv(normalno(20, 8, 15, 11));
	glColor3d(0, 0, 0);
	glVertex3dv(H);
	glVertex3dv(H1);
	glVertex3dv(G1);
	glVertex3dv(G);

	glNormal3dv(normalno(15, 11, 19, 13));
	glColor3d(0, 0, 0);
	glVertex3dv(G);
	glVertex3dv(G1);
	glVertex3dv(F1);
	glVertex3dv(F);

	glTexCoord2d(0, 0);
	glVertex3dv(F);
	glVertex3dv(F1);
	glVertex3dv(E1);
	glVertex3dv(E);


	glNormal3dv(normalno(6, 11, 1, 12));
	glColor3d(0, 0, 0);
	glVertex3dv(D);
	glVertex3dv(D1);
	glVertex3dv(C1);
	glVertex3dv(C);

	glNormal3dv(normalno(1, 12, 5, 9));
	glColor3d(0, 0, 0);
	glVertex3dv(C);
	glVertex3dv(C1);
	glVertex3dv(B1);
	glVertex3dv(B);

	glNormal3dv(normalno(5, 9, 6, 3));
	glColor3d(0, 0, 0);
	glVertex3dv(B);
	glVertex3dv(B1);
	glVertex3dv(A1);
	glVertex3dv(A);


	glEnd();

	//�������� �����


	double x = 11;
	double y = 16;
	double z = 0;
	double cR = 5;
	double rad = 0;
	double x1 = 0;
	double y1 = 0;
	double z1 = 3;


	//����� ������
	for (int phi = -1; phi > -54; phi--)
	{
		rad = phi * (3.14 / 180);
		x1 = cR * cos(rad) + 6;
		y1 = cR * sin(rad) + 16;
		double O[] = { x, y, z };
		double O1[] = { x,y, z1 };
		double temp[] = { x1, y1, z };
		double temp1[] = { x1, y1, z1 };


		glBegin(GL_TRIANGLES);

		glNormal3d(0, 0, -1);
		glColor3d(1, 0, 1);
		glVertex3dv(O);
		glVertex3dv(temp);
		glVertex3dv(G);

		glNormal3d(0, 0, 1);
		glColor4d(1, 0, 1, 0.9);
		glVertex3dv(O1);
		glVertex3dv(temp1);
		glVertex3dv(G1);

		glEnd();

		glBegin(GL_QUADS);

		glNormal3dv(normalno(x, y, x1, y1));
		glColor3d(0, 0, 0);
		glVertex3dv(temp);
		glVertex3dv(temp1);
		glVertex3dv(O1);
		glVertex3dv(O);

		glEnd();
		x = x1;
		y = y1;

	}

	x = 9;
	y = 12;
	x1 = 0;
	y1 = 0;

	for (int phi = -53; phi > -91; phi--)
	{
		rad = phi * (3.14 / 180);
		x1 = cR * cos(rad) + 6;
		y1 = cR * sin(rad) + 16;
		double O[] = { x, y, z };
		double O1[] = { x,y,z1 };
		double temp[] = { x1, y1, z };
		double temp1[] = { x1, y1, z1 };


		glBegin(GL_TRIANGLES);

		glNormal3d(0, 0, -1);
		glColor3d(1, 0, 1);
		glVertex3dv(O);
		glVertex3dv(temp);
		glVertex3dv(N);

		glNormal3d(0, 0, 1);
		glColor3d(1, 0, 1);
		glVertex3dv(O1);
		glVertex3dv(temp1);
		glVertex3dv(N1);

		glEnd();

		glBegin(GL_QUADS);

		glNormal3dv(normalno(x, y, x1, y1));
		glColor3d(0, 0, 0);
		glVertex3dv(temp);
		glVertex3dv(temp1);
		glVertex3dv(O1);
		glVertex3dv(O);

		glEnd();
		x = x1;
		y = y1;

	}


	double nx = 0;
	double ny = 0;
	double nz = 0;
	double leg = 0;
	double leg1 = 3;

	double O[] = { 15, 14.5, 0 };
	double O12[] = { 15, 14.5, 3 };
	double r = 4.27;
	x = 0;
	y = 0;
	z = 0;
	x1 = 19;
	y1 = 13;
	z1 = 3;

	rad = 0;

	//vypuklaya
	for (double phi = -20; phi < 160; phi += 0.5)
	{

		rad = phi * (3.14 / 180);
		x = r * cos(rad) + 15;
		y = r * sin(rad) + 14.5;
		double O1[] = { x, y, z };
		double O11[] = { x,y,z1 };
		double temp[] = { x1, y1, z };
		double temp1[] = { x1, y1, z1 };

		//nizhnyaya
		glBegin(GL_TRIANGLES);

		glNormal3d(0, 0, -1);
		glColor3d(0, 1, 0);
		glVertex3dv(temp);
		glVertex3dv(O1);
		glVertex3dv(O);

		//verhnyaya
		glNormal3d(0, 0, 1);
		glColor4d(0, 1, 0, 0.9);
		glVertex3dv(temp1);
		glVertex3dv(O11);
		glVertex3dv(O12);

		glEnd();

		glBegin(GL_QUADS);

		glNormal3dv(normalno(x1, y1, x, y));
		glColor3d(0, 0, 0);
		glVertex3dv(temp);
		glVertex3dv(temp1);
		glVertex3dv(O11);
		glVertex3dv(O1);

		glEnd();


		x1 = x;
		y1 = y;
	}
}


void Render(OpenGL *ogl)
{



	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);

	glEnable(GL_DEPTH_TEST);
	if (textureMode)
		glEnable(GL_TEXTURE_2D);

	if (lightMode)
		glEnable(GL_LIGHTING);


	//��������������
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	//��������� ���������
	GLfloat amb[] = { 0.2, 0.2, 0.1, 1. };
	GLfloat dif[] = { 0.4, 0.65, 0.5, 1. };
	GLfloat spec[] = { 0.9, 0.8, 0.3, 1. };
	GLfloat sh = 0.1f * 256;


	//�������
	glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
	//��������
	glMaterialfv(GL_FRONT, GL_DIFFUSE, dif);
	//����������
	glMaterialfv(GL_FRONT, GL_SPECULAR, spec); \
		//������ �����
		glMaterialf(GL_FRONT, GL_SHININESS, sh);

	//���� ���� �������, ��� ����������� (����������� ���������)
	glShadeModel(GL_SMOOTH);
	//===================================
	//������� ���  

	rybka();
	//circle(0, 0, 5);
	//rybkaalfa(); //����������������� ����� ���������

	/*//������ ��������� ���������� ��������
	double A[2] = { -4, -4 };
	double B[2] = { 4, -4 };
	double C[2] = { 4, 4 };
	double D[2] = { -4, 4 };

	glBindTexture(GL_TEXTURE_2D, texId);

	glColor3d(0.6, 0.6, 0.6);
	glBegin(GL_QUADS);

	glNormal3d(0, 0, 1);
	glTexCoord2d(0, 0);
	glVertex2dv(A);
	glTexCoord2d(1, 0);
	glVertex2dv(B);
	glTexCoord2d(1, 1);
	glVertex2dv(C);
	glTexCoord2d(0, 1);
	glVertex2dv(D);

	glEnd();
	//����� ��������� ���������� ��������
	*/

   //��������� ������ ������

	
	glMatrixMode(GL_PROJECTION);	//������ �������� ������� ��������. 
	                                //(���� ��������� ��������, ����� �� ������������.)
	glPushMatrix();   //��������� ������� ������� ������������� (������� ��������� ������������� ��������) � ���� 				    
	glLoadIdentity();	  //��������� ��������� �������
	glOrtho(0, ogl->getWidth(), 0, ogl->getHeight(), 0, 1);	 //������� ����� ������������� ��������

	glMatrixMode(GL_MODELVIEW);		//������������� �� �����-��� �������
	glPushMatrix();			  //��������� ������� ������� � ���� (��������� ������, ����������)
	glLoadIdentity();		  //���������� �� � ������

	glDisable(GL_LIGHTING);



	GuiTextRectangle rec;		   //������� ����� ��������� ��� ������� ������ � �������� ������.
	rec.setSize(300, 150);
	rec.setPosition(10, ogl->getHeight() - 150 - 10);


	std::stringstream ss;
	ss << "T - ���/���� �������" << std::endl;
	ss << "L - ���/���� ���������" << std::endl;
	ss << "F - ���� �� ������" << std::endl;
	ss << "G - ������� ���� �� �����������" << std::endl;
	ss << "G+��� ������� ���� �� ���������" << std::endl;
	ss << "�����. �����: (" << light.pos.X() << ", " << light.pos.Y() << ", " << light.pos.Z() << ")" << std::endl;
	ss << "�����. ������: (" << camera.pos.X() << ", " << camera.pos.Y() << ", " << camera.pos.Z() << ")" << std::endl;
	ss << "��������� ������: R="  << camera.camDist << ", fi1=" << camera.fi1 << ", fi2=" << camera.fi2 << std::endl;
	
	rec.setText(ss.str().c_str());
	rec.Draw();
	
	glMatrixMode(GL_PROJECTION);	  //��������������� ������� �������� � �����-��� �������� �� �����.
	glPopMatrix();


	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

}