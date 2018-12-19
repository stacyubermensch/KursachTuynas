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

//класс для настройки камеры
class CustomCamera : public Camera
{
public:
	//дистанция камеры
	double camDist;
	//углы поворота камеры
	double fi1, fi2;

	
	//значния масеры по умолчанию
	CustomCamera()
	{
		camDist = 15;
		fi1 = 1;
		fi2 = 1;
	}

	
	//считает позицию камеры, исходя из углов поворота, вызывается движком
	void SetUpCamera()
	{
		//отвечает за поворот камеры мышкой
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
		//функция настройки камеры
		gluLookAt(pos.X(), pos.Y(), pos.Z(), lookPoint.X(), lookPoint.Y(), lookPoint.Z(), normal.X(), normal.Y(), normal.Z());
	}



}  camera;   //создаем объект камеры


//Класс для настройки света
class CustomLight : public Light
{
public:
	CustomLight()
	{
		//начальная позиция света
		pos = Vector3(1, 1, 3);
	}

	
	//рисует сферу и линии под источником света, вызывается движком
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
			//линия от источника света до окружности
			glBegin(GL_LINES);
			glVertex3d(pos.X(), pos.Y(), pos.Z());
			glVertex3d(pos.X(), pos.Y(), 0);
			glEnd();

			//рисуем окруность
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

		// параметры источника света
		glLightfv(GL_LIGHT0, GL_POSITION, position);
		// характеристики излучаемого света
		// фоновое освещение (рассеянный свет)
		glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
		// диффузная составляющая света
		glLightfv(GL_LIGHT0, GL_DIFFUSE, dif);
		// зеркально отражаемая составляющая света
		glLightfv(GL_LIGHT0, GL_SPECULAR, spec);

		glEnable(GL_LIGHT0);
	}


} light;  //создаем источник света




//старые координаты мыши
int mouseX = 0, mouseY = 0;

void mouseEvent(OpenGL *ogl, int mX, int mY)
{
	int dx = mouseX - mX;
	int dy = mouseY - mY;
	mouseX = mX;
	mouseY = mY;

	//меняем углы камеры при нажатой левой кнопке мыши
	if (OpenGL::isKeyPressed(VK_RBUTTON))
	{
		camera.fi1 += 0.01*dx;
		camera.fi2 += -0.01*dy;
	}

	
	//двигаем свет по плоскости, в точку где мышь
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

//выполняется перед первым рендером
void initRender(OpenGL *ogl)
{
	//настройка текстур

	//4 байта на хранение пикселя
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	//настройка режима наложения текстур
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	//включаем текстуры
	glEnable(GL_TEXTURE_2D);
	

	//массив трехбайтных элементов  (R G B)
	RGBTRIPLE *texarray;

	//массив символов, (высота*ширина*4      4, потомучто   выше, мы указали использовать по 4 байта на пиксель текстуры - R G B A)
	char *texCharArray;
	int texW, texH;
	OpenGL::LoadBMP("buldog_10.bmp", &texW, &texH, &texarray);
	OpenGL::RGBtoChar(texarray, texW, texH, &texCharArray);

	
	
	//генерируем ИД для текстуры
	glGenTextures(1, &texId);
	//биндим айдишник, все что будет происходить с текстурой, будте происходить по этому ИД
	glBindTexture(GL_TEXTURE_2D, texId);

	//загружаем текстуру в видеопямять, в оперативке нам больше  она не нужна
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW, texH, 0, GL_RGBA, GL_UNSIGNED_BYTE, texCharArray);

	//отчистка памяти
	free(texCharArray);
	free(texarray);

	//наводим шмон
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);


	//камеру и свет привязываем к "движку"
	ogl->mainCamera = &camera;
	ogl->mainLight = &light;

	// нормализация нормалей : их длины будет равна 1
	glEnable(GL_NORMALIZE);

	// устранение ступенчатости для линий
	glEnable(GL_LINE_SMOOTH); 


	//   задать параметры освещения
	//  параметр GL_LIGHT_MODEL_TWO_SIDE - 
	//                0 -  лицевые и изнаночные рисуются одинаково(по умолчанию), 
	//                1 - лицевые и изнаночные обрабатываются разными режимами       
	//                соответственно лицевым и изнаночным свойствам материалов.    
	//  параметр GL_LIGHT_MODEL_AMBIENT - задать фоновое освещение, 
	//                не зависящее от сточников
	// по умолчанию (0.2, 0.2, 0.2, 1.0)

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

	//Нижняя грань

	glBindTexture(GL_TEXTURE_2D, texId);
	glBegin(GL_TRIANGLES);
	glColor3d(1, 0, 1);
	//треугольник ABI
	glNormal3d(0, 0, -1);
	glColor3d(1, 0, 1);
	glTexCoord2d(0.3, 0.15);
	glVertex3dv(A);
	glTexCoord2d(0.25, 0.45);
	glVertex3dv(B);
	glTexCoord2d(0.35, 0.4);
	glVertex3dv(I);

	//треугольник BCD
	glNormal3d(0, 0, -1);
	glColor3d(1, 0, 1);
	glTexCoord2d(0.25, 0.45);
	glVertex3dv(B);
	glTexCoord2d(0.05, 0.6);
	glVertex3dv(C);
	glTexCoord2d(0.3, 0.55);
	glVertex3dv(D);


	//треугольник BDI
	glNormal3d(0, 0, -1);
	glColor3d(1, 0, 1);
	glTexCoord2d(0.25, 0.45);
	glVertex3dv(B);
	glTexCoord2d(0.3, 0.55);
	glVertex3dv(D);
	glTexCoord2d(0.35, 0.4);
	glVertex3dv(I);


	//треугольник DEG
	glNormal3d(0, 0, -1);
	glColor3d(1, 0, 1);
	glTexCoord2d(0.3, 0.55);
	glVertex3dv(D);
	glTexCoord2d(0.55, 0.8);
	glVertex3dv(E);
	glTexCoord2d(0.75, 0.55);
	glVertex3dv(G);

	//треугольник EGF
	glNormal3d(0, 0, -1);
	glColor3d(1, 0, 1);
	glTexCoord2d(0.55, 0.8);
	glVertex3dv(E);
	glTexCoord2d(0.75, 0.55);
	glVertex3dv(G);
	glTexCoord2d(0.95, 0.65);
	glVertex3dv(F);

	//треугольник IGH
	glNormal3d(0, 0, -1);
	glColor3d(1, 0, 1);
	glTexCoord2d(0.35, 0.4);
	glVertex3dv(I);
	glTexCoord2d(0.75, 0.55);
	glVertex3dv(G);
	glTexCoord2d(1, 0.4);
	glVertex3dv(H);

	//треугольник DIG
	glNormal3d(0, 0, -1);
	glColor3d(1, 0, 1);
	glTexCoord2d(0.3, 0.55);
	glVertex3dv(D);
	glTexCoord2d(0.35, 0.4);
	glVertex3dv(I);
	glTexCoord2d(0.75, 0.55);
	glVertex3dv(G);

	



	////////////Верхняя грань////////////////////

		//треугольник ABI
	glNormal3d(0, 0, 1);
	glColor3d(1, 0, 1);
	glTexCoord2d(0.3, 0.15);
	glVertex3dv(A1);
	glTexCoord2d(0.25, 0.45);
	glVertex3dv(B1);
	glTexCoord2d(0.35, 0.4);
	glVertex3dv(I1);

	//треугольник BCD
	glNormal3d(0, 0, 1);
	glColor3d(1, 0, 1);
	glTexCoord2d(0.25, 0.45);
	glVertex3dv(B1);
	glTexCoord2d(0.05, 0.6);
	glVertex3dv(C1);
	glTexCoord2d(0.3, 0.55);
	glVertex3dv(D1);


	//треугольник BDI
	glNormal3d(0, 0, 1);
	glColor3d(1, 0, 1);
	glTexCoord2d(0.25, 0.45);
	glVertex3dv(B1);
	glTexCoord2d(0.3, 0.55);
	glVertex3dv(D1);
	glTexCoord2d(0.35, 0.4);
	glVertex3dv(I1);


	//треугольник DEG
	glNormal3d(0, 0, 1);
	glColor3d(1, 0, 1);
	glTexCoord2d(0.3, 0.55);
	glVertex3dv(D1);
	glTexCoord2d(0.55, 0.8);
	glVertex3dv(E1);
	glTexCoord2d(0.75, 0.55);
	glVertex3dv(G1);

	//треугольник EGF
	glNormal3d(0, 0, 1);
	glColor3d(1, 0, 1);
	glTexCoord2d(0.55, 0.8);
	glVertex3dv(E1);
	glTexCoord2d(0.75, 0.55);
	glVertex3dv(G1);
	glTexCoord2d(0.95, 0.65);
	glVertex3dv(F1);

	//треугольник IGH
	glNormal3d(0, 0, 1);
	glColor3d(1, 0, 1);
	glTexCoord2d(0.35, 0.4);
	glVertex3dv(I1);
	glTexCoord2d(0.75, 0.55);
	glVertex3dv(G1);
	glTexCoord2d(1, 0.4);
	glVertex3dv(H1);

	//треугольник DIG
	glNormal3d(0, 0, 1);
	glColor3d(1, 0, 1);
	glTexCoord2d(0.3, 0.55);
	glVertex3dv(D1);
	glTexCoord2d(0.35, 0.4);
	glVertex3dv(I1);
	glTexCoord2d(0.75, 0.55);
	glVertex3dv(G1);

	glEnd();



	//СТЕНОЧКИ



	glBegin(GL_QUADS);


	//стенка AI
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


	//стенка IH
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


	//стенка HG
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

	//стенка GF
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


	//стенка FE
	glNormal3dv(normalno(19, 13, 11, 16));
	glTexCoord2d(0, 0);
	glVertex3dv(F);
	glTexCoord2d(0, 1);
	glVertex3dv(F1);
	glTexCoord2d(1, 1);
	glVertex3dv(E1);
	glTexCoord2d(1, 0);
	glVertex3dv(E);

	//стенка ED
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

	//стенка DC
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

	//стенка CB
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


	//стенка BA
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

//КРУГ С ТЕКСТУРКОЙ УЖЕ
void circle(double x, double y, double r)

{
	double rad = 0;
	double x1 = 0;
	double y1 = 0;

	double x_1 = 5;		//координата начальной точки
	double y_1 = 0;

	double x_text = 1;			//для текстуры начальная точка
	double y_text = 0.5;

	double x_text1 = 0;
	double y_text1 = 0;

	double O[] = { x, y };
	double O1[] = { 0.5, 0.5 };





	for (int phi = 0; phi < 360; phi++)
	{
		rad = phi * (3.14 / 180);
		x1 = r * cos(rad) + x;	//новая точка
		y1 = r * sin(rad) + y;

		x_text1 = 0.5 * cos(rad) + 0.5;	//координата текстуры
		y_text1 = 0.5 * sin(rad) + 0.5;
		double temp[] = { x1, y1, };		//новая точка
		double temp1[] = { x_1, y_1 };			//старая точка

		double temp_text[] = { x_text, y_text, };		//старая точка
		double temp1_text[] = { x_text1, y_text1 };		//новая точка

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

	//Нижняя грань

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

	//Верхняя грань

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



	//СТЕНОЧКИ



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

	//ВОГНУТАЯ ЧАСТЬ


	double x = 11;
	double y = 16;
	double z = 0;
	double cR = 5;
	double rad = 0;
	double x1 = 0;
	double y1 = 0;
	double z1 = 3;


	//Часть справа
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


	//альфаналожение
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	//настройка материала
	GLfloat amb[] = { 0.2, 0.2, 0.1, 1. };
	GLfloat dif[] = { 0.4, 0.65, 0.5, 1. };
	GLfloat spec[] = { 0.9, 0.8, 0.3, 1. };
	GLfloat sh = 0.1f * 256;


	//фоновая
	glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
	//дифузная
	glMaterialfv(GL_FRONT, GL_DIFFUSE, dif);
	//зеркальная
	glMaterialfv(GL_FRONT, GL_SPECULAR, spec); \
		//размер блика
		glMaterialf(GL_FRONT, GL_SHININESS, sh);

	//чтоб было красиво, без квадратиков (сглаживание освещения)
	glShadeModel(GL_SMOOTH);
	//===================================
	//Прогать тут  

	rybka();
	//circle(0, 0, 5);
	//rybkaalfa(); //раскомментировать альфа наложение

	/*//Начало рисования квадратика станкина
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
	//конец рисования квадратика станкина
	*/

   //Сообщение вверху экрана

	
	glMatrixMode(GL_PROJECTION);	//Делаем активной матрицу проекций. 
	                                //(всек матричные операции, будут ее видоизменять.)
	glPushMatrix();   //сохраняем текущую матрицу проецирования (которая описывает перспективную проекцию) в стек 				    
	glLoadIdentity();	  //Загружаем единичную матрицу
	glOrtho(0, ogl->getWidth(), 0, ogl->getHeight(), 0, 1);	 //врубаем режим ортогональной проекции

	glMatrixMode(GL_MODELVIEW);		//переключаемся на модел-вью матрицу
	glPushMatrix();			  //сохраняем текущую матрицу в стек (положение камеры, фактически)
	glLoadIdentity();		  //сбрасываем ее в дефолт

	glDisable(GL_LIGHTING);



	GuiTextRectangle rec;		   //классик моего авторства для удобной работы с рендером текста.
	rec.setSize(300, 150);
	rec.setPosition(10, ogl->getHeight() - 150 - 10);


	std::stringstream ss;
	ss << "T - вкл/выкл текстур" << std::endl;
	ss << "L - вкл/выкл освещение" << std::endl;
	ss << "F - Свет из камеры" << std::endl;
	ss << "G - двигать свет по горизонтали" << std::endl;
	ss << "G+ЛКМ двигать свет по вертекали" << std::endl;
	ss << "Коорд. света: (" << light.pos.X() << ", " << light.pos.Y() << ", " << light.pos.Z() << ")" << std::endl;
	ss << "Коорд. камеры: (" << camera.pos.X() << ", " << camera.pos.Y() << ", " << camera.pos.Z() << ")" << std::endl;
	ss << "Параметры камеры: R="  << camera.camDist << ", fi1=" << camera.fi1 << ", fi2=" << camera.fi2 << std::endl;
	
	rec.setText(ss.str().c_str());
	rec.Draw();
	
	glMatrixMode(GL_PROJECTION);	  //восстанавливаем матрицы проекции и модел-вью обратьно из стека.
	glPopMatrix();


	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

}