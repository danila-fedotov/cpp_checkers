#include "glut.h" //подключение glut
#include "object.h"
int angleX, angleZ; //отвечают за поворот доски относительно осей X и Z
int mouseX, mouseY; //координаты курсора
int distX, distZ=-100, width=1024, height=768;
int on_button; //последн€€ нажата€ кнопка мыши
int move;
game gamer(&move, &angleZ, &distX);

void draw()
{
	glViewport(0, 0, width, height);
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	//настройка камеры
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45, (GLfloat)width/height, 1, 300);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	//отрисовка экрана
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_TEXTURE_2D);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, (float)GL_REPLACE);
	glTranslatef(distX, 0, distZ);
	glRotatef(angleX, 1, 0, 0);
	if (move==2) gamer.rotate_table();
	glRotatef(angleZ, 0, 0, 1);
	gamer.draw();
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_COLOR_MATERIAL);
	glDisable(GL_LIGHT0);
	glDisable(GL_LIGHTING);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glFlush();
	glutSwapBuffers(); //типа return
}
void on_keyboard(unsigned char key, int x, int y)
{
	if (key==27) exit(0);
	if (move==3)
	{
		gamer.new_game();
		return;
	}
	if (move!=0) return;
	switch (key)
	{
	case 'r':
		gamer.replace_player();
		break;
	case 'n':
		gamer.new_game();
		break;
	case 't':
		gamer.flag=1-gamer.flag;
		break;
	}
}
void on_mouse(int button, int state, int x, int y)
{
	if (move!=0) return;
	if (state==0)
	{
		on_button=button;
		mouseX=x;
		mouseY=y;
	}
	if (button==0 && state==0)
	{
		glViewport(0, 0, width, height);
		glClearColor(0, 0, 0, 0);
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(45, (GLfloat)width/height, 1, 300);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glTranslatef(0, 0, distZ);
		glRotatef(angleX, 1, 0, 0);
		glRotatef(angleZ, 0, 0, 1);
		gamer.on_mouse(x, height-y);
	}
}
void motion_mouse(int x, int y)
{
	if (move!=0) return;
	if (on_button==1)
	{
		distZ+=mouseY-y;
		distZ=MAX(MIN(distZ,-50),-200);
		mouseY=y;
	}
	if (on_button==2)
	{
        angleX-=mouseY-y;
		angleX=MAX(MIN(angleX,0),-80);
        angleZ+=mouseX-x;
		angleZ=MAX(MIN(angleZ,180),-180);
		mouseX=x;
		mouseY=y;
	}
}
void on_timer(int value)
{
	draw(); // перерисуем экран
	glutTimerFunc(33, on_timer, 0); // через 33мс вызоветс€ эта функци€
}
int main(int argc, char* argv[])
{
	//инициализаци€ окна
	glutInit(&argc,argv);
	glutInitWindowSize(width,height);
	glutInitDisplayMode(GLUT_RGBA|GLUT_DOUBLE|GLUT_DEPTH);
	glutCreateWindow("игра Ўашки v5.1");
	glutFullScreen();
	gamer.create_tex();
	gamer.replace_player();
	glutDisplayFunc(draw);
	//glutIdleFunc(draw);
	glutKeyboardFunc(on_keyboard); //--//-- за нажатие клавишы escape
	glutMouseFunc(on_mouse); //--//-- за нажатие кнопок мыши
	glutMotionFunc(motion_mouse);
	glutTimerFunc(33, on_timer, 0);
	glutMainLoop();
	return 0;
}