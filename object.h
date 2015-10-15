#include "patch.h"
#include "structs.h"
#include <math.h>
#include <stdio.h>

class pawn
{
public:
	static int xx, yy, zz; //для инициализиции координат
	static int counter; //счетчик для цвета

	int state; //0-нет, 1-шашка, 2-дамка
	int player; //0-1 игрок, 1-2 игрок
	int color; //уникальный цвет
	float x, y, z; //координаты
	int angle_x; //угол поворота относительно оси X
	way dl, dr, ul, ur;
	GLUquadricObj* obj;

	pawn()
	{
		state=1;
		if (counter<13) player=0;
		else player=1;
		color=counter;
		init_coord();
		angle_x=0;
		obj=gluNewQuadric();
		counter++;
	}
	~pawn()
	{
		gluDeleteQuadric(obj);
	}
	void draw() const
	{
		if (state==0) return;
		glPushMatrix();
		switch (player)
		{
		case 0:
			glTranslatef(x, y, z);
			if (angle_x!=0) glRotatef(angle_x, 1, 0, 0);
			//тестовая версия
			glColor3ub(250, 250, 250);
			draw_object();
			//конец
			break;
		case 1:
			glTranslatef(x, y, z);
			if (angle_x!=0) glRotatef(angle_x, 1, 0, 0);
			//тестовая версия
			glColor3ub(70, 70, 70);
			draw_object();
			//конец
			break;
		}
		glPopMatrix();
	}
	void draw_clr(int f) const
	{
		if (state==0) return;
		glPushMatrix();
		switch (f)
		{
		case 0:
			glTranslatef(x, y, z);
			if (angle_x!=0) glRotatef(angle_x, 1, 0, 0);
			//тестовая версия
			glColor3ub(0, 0, 0);
			draw_object();
			//конец
			break;
		case 1:
			glTranslatef(x, y, z);
			if (angle_x!=0) glRotatef(angle_x, 1, 0, 0);
			glColor3ub(color, 0, 0);
			draw_object();
			//конец
			break;
		}
		glPopMatrix();
	}
	void null_way()
	{
		dl.first=0;
		dl.steps=0;
		dr.first=0;
		dr.steps=0;
		ul.first=0;
		ul.steps=0;
		ur.first=0;
		ur.steps=0;
	}
	void refresh()
	{
		state=1;
		init_coord();
		angle_x=0;
	}
private:
	void init_coord()
	{
		x=xx;
		y=yy;
		z=0;
		yy-=16;
		if (yy<-32)
		{
			yy=28-zz*8;
			xx+=8;
			zz=1-zz;
		}
		if (xx==-4) xx=12;
	}
	void draw_object() const
	{
		gluCylinder(obj, 3, 3, 2, 20, 1);
		gluQuadricOrientation(obj, GLU_INSIDE);
		gluCylinder(obj, 2.6, 2.6, 2, 20, 1);
		gluDisk(obj, 2.6, 3, 20, 1);
		glTranslatef(0, 0, 1.7);
		gluDisk(obj, 0, 2.6, 20, 1);
		gluQuadricOrientation(obj, GLU_OUTSIDE);
		glTranslatef(0, 0, 0.3);
		gluDisk(obj, 0, 3, 20, 1);
		glutSolidTorus(0.4, 2.6, 10, 20);
		glutSolidTorus(0.3, 1.3, 10, 20);
	}
};
int pawn::xx=-28;
int pawn::yy=28;
int pawn::zz=1;
int pawn::counter=1;
class game
{
public:
	pawn p[24]; //шашки
	int field[8][8]; //поле с индексами шашек
	int vrtx[384]; //массив вершин
	unsigned char pxlclr[3]; //для буфера цвета
	GLuint id; //индекс для текстурки
	int played; //активный игрок 
	int moved; //движение шашек
	select s; //выбранная фишка (-1 - ничего не выбрано)
	select d;
	walking w; //проверка шашек перед началом хода
	int *move, *angleZ, *distX; //указатели на движение и угол сцены в общем
	int r; //дополнительная переменная
	int flag;

	game(int *m, int *a, int* d)
	{
		init_field();
		init_vrtx();
		for (int i=0;i<3;i++) pxlclr[i]=0;
		id=0;
		played=1;
		moved=0;
		move=m;
		angleZ=a;
		distX=d;
		r=0;
		flag=0;
	}
	~game()
	{
		if (glIsTexture(id)) glDeleteTextures(1, &id);
	}
	void draw()
	{
		int f=0;
		switch (moved)
		{
		case 0:
			p[s.ix].z++;
			p[s.ix].angle_x+=r;
			f=1;
			break;
		case 1:
			if (p[s.ix].x>w.x) p[s.ix].x--;
			else p[s.ix].x++;
			if (p[s.ix].y>w.y) p[s.ix].y--;
			else p[s.ix].y++;
			if (p[s.ix].x==w.x)
				if (s.j==7*(1-played) && p[s.ix].state==1)
				{
					w.z=1;
					moved=4;
				}
				else replace_player();
			break;
		case 2:
			if (p[s.ix].x>w.x) p[s.ix].x--;
			else p[s.ix].x++;
			if (p[s.ix].y>w.y) p[s.ix].y--;
			else p[s.ix].y++;
			r=abs(p[s.ix].x-p[d.ix].x);
			if (r>3 && r<8) p[s.ix].z+=w.z;
			if (r==0) w.z=-1;
			if (p[s.ix].x==w.x)
			{
				moved=3;
				w.z=1;
				r=0;
			}
			break;
		case 3:
			p[d.ix].z++;
			p[d.ix].angle_x+=5;
			if (p[d.ix].z>30)
			{
				p[d.ix].state=0;
				if (s.j==7*(1-played) && p[s.ix].state==1)
				{
					w.z=1;
					moved=4;
					break;
				}
				if (w.fight==0) replace_player();
				else moved=0;
			}
			break;
		case 4:
			p[s.ix].z+=w.z;
			p[s.ix].angle_x+=9;
			if (p[s.ix].z==11) w.z=-1;
			if (p[s.ix].z==2 && w.z==-1)
			{
				p[s.ix].z=2.4;
				p[s.ix].state=2;
				w.z=1;
				if (w.fight==0) replace_player();
				else moved=0;
			}
			break;
		case 5:
			*distX-=5;
			if (*distX==-100)
			{
				*distX=100;
				for (int i=0;i<24;i++) p[i].refresh();
			}
			if (*distX==0) replace_player();
            break;
		}
		if (flag==0) draw_table();
		else draw_table_clr();
		for (int i=0;i<24;i++) p[i].draw();
		if (f==1)
		{
			if (r==30 || r==-30) w.z*=-1; 
			p[s.ix].z--;
			p[s.ix].angle_x-=r;
			r+=5*w.z;
			return;
		}
		if (moved==6)
		{
			char sz[128]="";
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			glOrtho(0,1024, 0,768, -1,1);
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();
			glColor3f(1,0,0);
			sprintf(sz, "Win is %i", 2-played);
			draw_string(GLUT_BITMAP_TIMES_ROMAN_24, sz, 480, 30);
			*move=3;
		}
	}
	void rotate_table()
	{
		int k;
		switch (played)
		{
		case 0:
			k=MIN(abs(*angleZ-90), 5);
			if (*angleZ>90) k*=-1;
			*angleZ+=k;
		    if (*angleZ==90) *move=0;
		    break;
		case 1:
			k=MIN(abs(*angleZ+90), 5);
			if (*angleZ>-90) k*=-1;
			*angleZ+=k;
			if (*angleZ==-90) *move=0;
			break;
		}
	}
	void replace_player()
	{
		int f=0;
		played=1-played;
		moved=0;
		s.ix=-1;
		d.ix=-1;
		for (int k=0;k<12;k++) w.el[k]=0;
		w.fight=0;
		w.z=1;
		r=0;
		*move=2;
		int ip=0, jp=0;
		int end=(played+1)*12;
		for (int i=played*12;i<end;i++)
		{
			if (p[i].state==0) continue;
			ip=(28-p[i].y)/8;
			jp=(28+p[i].x)/8;
			if (w.fight==0)
			{
				if (check_fight_walk_begin(ip, jp, i)==1)
				{
					w.fight=1;
					continue;
				}
				check_walk(ip, jp, i);
			}
			else
			{
				check_fight_begin(ip, jp, i);
			}
		}
		for (int i=0;i<12;i++)
			if (w.el[i]==1)
			{
				f=1;
				s.ix=12*played+i;
				s.i=(28-p[s.ix].y)/8;
				s.j=(28+p[s.ix].x)/8;
				break;
			}
		if (f==0) moved=6;
	}
	void on_mouse(int x, int y)
	{
		if (moved!=0) return;
		//отрисовка полигонов для шашки
		draw_table_clr();
		//отрисовка шашек, которые могут ходить
		if (played==0)
		{
			for (int i=0;i<12;i++) p[i].draw_clr(w.el[i]);
			for (int i=12;i<24;i++) p[i].draw_clr(0);
		}
		else
		{
			for (int i=0;i<12;i++) p[i].draw_clr(0);
			for (int i=12;i<24;i++) p[i].draw_clr(w.el[i-12]);
		}
		glReadPixels(x, y, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, &pxlclr);
		if (pxlclr[0]!=0 && d.ix==-1)
		{
			if (s.ix==pxlclr[0]-1) return;
			s.ix=pxlclr[0]-1;
			s.i=(28-p[s.ix].y)/8;
			s.j=(28+p[s.ix].x)/8;
			w.z=1;
			r=0;
			return;
		}
		if (pxlclr[1]!=0)
		{
			int i=pxlclr[1]-1;
			int j=pxlclr[2]-1;
			int v=0;
			if (w.fight==0)
			{
				w.x=p[s.ix].x+(j-s.j)*8;
				w.y=p[s.ix].y+(s.i-i)*8;
				field[s.i][s.j]=0;
				field[i][j]=s.ix+1;
				moved=1;
			}
			else
			{
				w.x=p[s.ix].x+(j-s.j)*8;
				w.y=p[s.ix].y+(s.i-i)*8;
				field[s.i][s.j]=0;
				field[i][j]=s.ix+1;
				if (i-s.i>0)
					if (j-s.j<0)
					{
						d.i=s.i+p[s.ix].dl.first;
						d.j=s.j-p[s.ix].dl.first;
						v=1;
					}
					else
					{
						d.i=s.i+p[s.ix].dr.first;
						d.j=s.j+p[s.ix].dr.first;
						v=2;
					}
				else
					if (j-s.j<0)
					{
						d.i=s.i-p[s.ix].ul.first;
						d.j=s.j-p[s.ix].ul.first;
						v=3;
					}
					else
					{
						d.i=s.i-p[s.ix].ur.first;
						d.j=s.j+p[s.ix].ur.first;
						v=4;
					}
				d.ix=field[d.i][d.j]-1;
				field[d.i][d.j]=0;
				if (j==(1-played)*7) w.fight=check_fight(i, j, s.ix, v, 2);
				else w.fight=check_fight(i, j, s.ix, v, 1);
				moved=2;
				w.z=1;
			}
			s.i=i;
			s.j=j;
		}
	}
	void create_tex()
	{
		const int size=288;
		unsigned char data[size*size*3]={0};
		int i, j;
		for (i=0;i<16;i++)
			for (j=0;j<size;j++)
			{
				data[(i*size+j)*3+0]=97;
				data[(i*size+j)*3+1]=55;
				data[(i*size+j)*3+2]=0;
			}
		for (i=16;i<size-16;i++)
		{
			for (j=0;j<16;j++)
			{
				data[(i*size+j)*3+0]=97;
				data[(i*size+j)*3+1]=55;
				data[(i*size+j)*3+2]=0;
			}
			for (j=16;j<size-16;j++)
			{
				if (((j-16)/32+(i-16)/32)%2==0)
				{
					data[(i*size+j)*3+0] = 147;
					data[(i*size+j)*3+1] = 74;
					data[(i*size+j)*3+2] = 0;
				}
				else
				{
					data[(i*size+j)*3+0] = 255;
					data[(i*size+j)*3+1] = 203;
					data[(i*size+j)*3+2] = 0;
				}
			}
			for (j=size-16;j<size;j++)
			{
				data[(i*size+j)*3+0]=97;
				data[(i*size+j)*3+1]=55;
				data[(i*size+j)*3+2]=0;
			}
		}
		for (i=size-16;i<size;i++)
			for (j=0;j<size;j++)
			{
				data[(i*size+j)*3+0]=97;
				data[(i*size+j)*3+1]=55;
				data[(i*size+j)*3+2]=0;
			}
		glGenTextures(1, &id);
		glBindTexture(GL_TEXTURE_2D, id);
		gluBuild2DMipmaps(GL_TEXTURE_2D, 3, size, size, GL_RGB, GL_UNSIGNED_BYTE, data);
	}
	void new_game()
	{
		r=0;
		*move=1;
		moved=5;
		pawn::xx=-28;
		pawn::yy=28;
		pawn::zz=1;
		init_field();
		played=1;
	}
private:
	void draw_string(void *font, const char* sz, float x, float y)
	{
		if (!sz) return;										// если пихнули 0, вместо строки - вылетим
		glRasterPos2f(x,y);										// установка позиции текста
		while(*sz) { glutBitmapCharacter(font, *sz); sz++; }	// строка выводится посимвольно
	}
	void init_field()
	{
		int k=1;
		for (int i=0;i<3;i++)
		{
			for (int j=i%2;j<8;j+=2)
			{
				field[j][i]=k;
				k++;
			}
			for (int j=1-i%2;j<8;j+=2) field[j][i]=0;
		}
		for (int i=3;i<5;i++)
			for (int j=0;j<8;j++) field[j][i]=0;
		for (int i=5;i<8;i++)
		{
			for (int j=i%2;j<8;j+=2)
			{
				field[j][i]=k;
				k++;
			}
			for (int j=1-i%2;j<8;j+=2) field[j][i]=0;
		}
	}
	void init_vrtx() //инициализация массива вершина и поля с шашками
	{
		int x=-32, y=32, k=1;
		for (int i=0;i<384;i+=12)
		{
			vrtx[i]=x;
			vrtx[i+1]=y;
			vrtx[i+2]=0;
			vrtx[i+3]=x;
			vrtx[i+4]=y-8;
			vrtx[i+5]=0;
			vrtx[i+6]=x+8;
			vrtx[i+7]=y-8;
			vrtx[i+8]=0;
			vrtx[i+9]=x+8;
			vrtx[i+10]=y;
			vrtx[i+11]=0;
			x+=16;
			if (x>=32)
			{
				x=-32+8*k;
				y-=8;
				k=1-k;
			}
		}
	}
	void draw_table() const
	{
		glBindTexture(GL_TEXTURE_2D, id);
		glBegin(GL_QUADS);
			glTexCoord2f(0, 0);	glVertex2f(-36, -36);
			glTexCoord2f(1, 0);	glVertex2f(36, -36);
			glTexCoord2f(1, 1);	glVertex2f(36, 36);
			glTexCoord2f(0, 1);	glVertex2f(-36, 36);
		glEnd();
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	void draw_poligon(int i, int j) const
	{
		glColor3ub(0, i+1, j+1);
		glDrawArrays(GL_QUADS, (i*4+(j-i%2)/2)*4, 4);
	}
	void draw_table_clr() const
	{
		if (moved!=0) return;
		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(3, GL_INT, 0, vrtx);
		if (p[s.ix].dl.steps!=0) for (int i=1;i<=p[s.ix].dl.steps;i++) draw_poligon(s.i+i+p[s.ix].dl.first, s.j-i-p[s.ix].dl.first);
		if (p[s.ix].dr.steps!=0) for (int i=1;i<=p[s.ix].dr.steps;i++) draw_poligon(s.i+i+p[s.ix].dr.first, s.j+i+p[s.ix].dr.first);
		if (p[s.ix].ul.steps!=0) for (int i=1;i<=p[s.ix].ul.steps;i++) draw_poligon(s.i-i-p[s.ix].ul.first, s.j-i-p[s.ix].ul.first);
		if (p[s.ix].ur.steps!=0) for (int i=1;i<=p[s.ix].ur.steps;i++) draw_poligon(s.i-i-p[s.ix].ur.first, s.j+i+p[s.ix].ur.first);
		glDisableClientState(GL_VERTEX_ARRAY);
	}
	void check_walk(int i, int j, int index)
	{
		int ind, q=0;
		if (played==0) ind=index;
		else ind=index-12;
		switch (p[index].state)
		{
		case 1:
			if (played==0)
			{
				if (i+1<8)
					if (field[i+1][j+1]==0)
					{
						w.el[ind]=1;
						p[index].dr.steps=1;
					}
				if (i-1>-1)
					if (field[i-1][j+1]==0)
					{
						w.el[ind]=1;
						p[index].ur.steps=1;
					}
			}
			else
			{
				if (i+1<8)
					if (field[i+1][j-1]==0)
					{
						w.el[ind]=1;
						p[index].dl.steps=1;
					}
				if (i-1>-1)
					if (field[i-1][j-1]==0)
					{
						w.el[ind]=1;
						p[index].ul.steps=1;
					}
			}
			break;
		case 2:
			for (q=1;i+q<8 && j+q<8 && field[i+q][j+q]==0;q++);
			if (q!=1)
			{
				w.el[ind]=1;
				p[index].dr.steps=q-1;
			}
			for (q=1;i-q>-1 && j+q<8 && field[i-q][j+q]==0;q++);
			if (q!=1)
			{
				w.el[ind]=1;
				p[index].ur.steps=q-1;
			}
			for (q=1;i+q<8 && j-q>-1 && field[i+q][j-q]==0;q++);
			if (q!=1)
			{
				w.el[ind]=1;
				p[index].dl.steps=q-1;
			}
			for (q=1;i-q>-1 && j-q>-1 && field[i-q][j-q]==0;q++);
			if (q!=1)
			{
				w.el[ind]=1;
				p[index].ul.steps=q-1;
			}
			break;
		}
	}
	int check_fight_walk_begin(int i, int j, int index)
	{
		int ind;
		if (played==0) ind=index;
		else ind=index-12;
		int b=(1-played)*12, e=(2-played)*12, bo=played*12, eo=(1+played)*12;
		int f=0, h;
		p[index].null_way();
		switch (p[index].state)
		{
		case 1:
			if (played==0)
			{
				if (j+2<8)
				{
					if (i+2<8)
						if (field[i+2][j+2]==0 && field[i+1][j+1]>12)
						{
							for (int k=0;k<ind;k++) w.el[k]=0;
							w.el[ind]=1;
							p[index].dr.first=1;
							p[index].dr.steps=1;
							f=1;
						}
					if (i-2>-1)
						if (field[i-2][j+2]==0 && field[i-1][j+1]>12)
						{
							if (f==0)
							{
								for (int k=0;k<ind;k++) w.el[k]=0;
								w.el[ind]=1;
								f=1;
							}
							p[index].ur.first=1;
							p[index].ur.steps=1;
						}
				}
			}
			else
			{
				if (j-2>-1)
				{
					if (i+2<8)
						if (field[i+2][j-2]==0 && field[i+1][j-1]>0 && field[i+1][j-1]<13)
						{
							for (int k=0;k<ind;k++) w.el[k]=0;
							w.el[ind]=1;
							p[index].dl.first=1;
							p[index].dl.steps=1;
							f=1;
						}
					if (i-2>-1)
						if (field[i-2][j-2]==0 && field[i-1][j-1]>0 && field[i-1][j-1]<13)
						{
							if (f==0)
							{
								for (int k=0;k<ind;k++) w.el[k]=0;
								w.el[ind]=1;
								f=1;
							}
							p[index].ul.first=1;
							p[index].ul.steps=1;
						}
				}
			}
			break;
		case 2:
			for (int q=1;i+q<7 && j+q<7;q++)
			{
				if (field[i+q][j+q]>bo && field[i+q][j+q]<=eo) break;
				if (field[i+q][j+q]>b && field[i+q][j+q]<=e)
				{
					for (h=q;i+h<7 && j+h<7 && field[i+h+1][j+h+1]==0;h++);
					if (q!=h)
					{
						for (int l=0;l<ind;l++) w.el[l]=0;
						w.el[ind]=1;
						p[index].dr.first=q;
						p[index].dr.steps=h-q;
						f=1;
					}
					break;
				}
			}
			for (int q=1;i-q>0 && j+q<7;q++)
			{
				if (field[i-q][j+q]>bo && field[i-q][j+q]<=eo) break;
				if (field[i-q][j+q]>b && field[i-q][j+q]<=e)
				{
					for (h=q;i-h>0 && j+h<7 && field[i-h-1][j+h+1]==0;h++);
					if (q!=h)
					{
						if (f==0)
						{
							for (int l=0;l<ind;l++) w.el[l]=0;
							w.el[ind]=1;
							f=1;
						}
						p[index].ur.first=q;
						p[index].ur.steps=h-q;
					}
					break;
				}
			}
			for (int q=1;i+q<7 && j-q>0;q++)
			{
				if (field[i+q][j-q]>bo && field[i+q][j-q]<=eo) break;
				if (field[i+q][j-q]>b && field[i+q][j-q]<=e)
				{
					for (h=q;i+h<7 && j-h>0 && field[i+h+1][j-h-1]==0;h++);
					if (q!=h)
					{
						if (f==0)
						{
							for (int l=0;l<ind;l++) w.el[l]=0;
							w.el[ind]=1;
							f=1;
						}
						p[index].dl.first=q;
						p[index].dl.steps=h-q;
					}
					break;
				}
			}
			for (int q=1;i-q>0 && j-q>0;q++)
			{
				if (field[i-q][j-q]>bo && field[i-q][j-q]<=eo) break;
				if (field[i-q][j-q]>b && field[i-q][j-q]<=e)
				{
					for (h=q;i-h>0 && j-h>0 && field[i-h-1][j-h-1]==0;h++);
					if (q!=h)
					{
						if (f==0)
						{
							for (int l=0;l<ind;l++) w.el[l]=0;
							w.el[ind]=1;
							f=1;
						}
						p[index].ul.first=q;
						p[index].ul.steps=h-q;
					}
					break;
				}
			}
			break;
		}
		return f;
	}
	void check_fight_begin(int i, int j, int index)
	{
		int ind=0;
		if (played==0) ind=index;
		else ind=index-12;
		int b=(1-played)*12, e=(2-played)*12, bo=played*12, eo=(1+played)*12;
		int f=0, h;
		p[index].null_way();
		switch (p[index].state)
		{
		case 1:
			if (played==0)
			{
				if (j+2<8)
				{
					if (i+2<8)
						if (field[i+2][j+2]==0 && field[i+1][j+1]>12)
						{
							w.el[ind]=1;
							p[index].dr.first=1;
							p[index].dr.steps=1;
							f=1;
						}
					if (i-2>-1)
						if (field[i-2][j+2]==0 && field[i-1][j+1]>12)
						{
							if (f==0) w.el[ind]=1;
							p[index].ur.first=1;
							p[index].ur.steps=1;
						}
				}
			}
			else
			{
				if (j-2>-1)
				{
					if (i+2<8)
						if (field[i+2][j-2]==0 && field[i+1][j-1]>0 && field[i+1][j-1]<13)
						{
							w.el[ind]=1;
							p[index].dl.first=1;
							p[index].dl.steps=1;
							f=1;
						}
					if (i-2>-1)
						if (field[i-2][j-2]==0 && field[i-1][j-1]>0 && field[i-1][j-1]<13)
						{
							if (f==0) w.el[ind]=1;
							p[index].ul.first=1;
							p[index].ul.steps=1;
						}
				}
			}
			break;
		case 2:
			for (int q=1;i+q<7 && j+q<7;q++)
			{
				if (field[i+q][j+q]>bo && field[i+q][j+q]<=eo) break;
				if (field[i+q][j+q]>b && field[i+q][j+q]<=e)
				{
					for (h=q;i+h<7 && j+h<7 && field[i+h+1][j+h+1]==0;h++);
					if (q!=h)
					{
						w.el[ind]=1;
						p[index].dr.first=q;
						p[index].dr.steps=h-q;
						f=1;
					}
					break;
				}
			}
			for (int q=1;i-q>0 && j+q<7;q++)
			{
				if (field[i-q][j+q]>bo && field[i-q][j+q]<=eo) break;
				if (field[i-q][j+q]>b && field[i-q][j+q]<=e)
				{
					for (h=q;i-h>0 && j+h<7 && field[i-h-1][j+h+1]==0;h++);
					if (q!=h)
					{
						if (f==0)
						{
							w.el[ind]=1;
							f=1;
						}
						p[index].ur.first=q;
						p[index].ur.steps=h-q;
					}
					break;
				}
			}
			for (int q=1;i+q<7 && j-q>0;q++)
			{
				if (field[i+q][j-q]>bo && field[i+q][j-q]<=eo) break;
				if (field[i+q][j-q]>b && field[i+q][j-q]<=e)
				{
					for (h=q;i+h<7 && j-h>0 && field[i+h+1][j-h-1]==0;h++);
					if (q!=h)
					{
						if (f==0)
						{
							w.el[ind]=1;
							f=1;
						}
						p[index].dl.first=q;
						p[index].dl.steps=h-q;
					}
					break;
				}
			}
			for (int q=1;i-q>0 && j-q>0;q++)
			{
				if (field[i-q][j-q]>bo && field[i-q][j-q]<=eo) break;
				if (field[i-q][j-q]>b && field[i-q][j-q]<=e)
				{
					for (h=q;i-h>0 && j-h>0 && field[i-h-1][j-h-1]==0;h++);
					if (q!=h)
					{
						if (f==0)
						{
							w.el[ind]=1;
							f=1;
						}
						p[index].ul.first=q;
						p[index].ul.steps=h-q;
					}
					break;
				}
			}
			break;
		}
	}
	int check_fight(int i, int j, int index, int v, int s)
	{
		int b=(1-played)*12, e=(2-played)*12, bo=played*12, eo=(1+played)*12;
		int f=0, h;
		int state=MAX(s, p[index].state);
		p[index].null_way();
		switch (state)
		{
		case 1:
			if (j+2<8)
			{
				if (i+2<8 && v!=3)
					if (field[i+2][j+2]==0 && field[i+1][j+1]>b && field[i+1][j+1]<=e)
					{
						p[index].dr.first=1;
						p[index].dr.steps=1;
						f=1;
					}
				if (i-2>-1 && v!=1)
					if (field[i-2][j+2]==0 && field[i-1][j+1]>b && field[i-1][j+1]<=e)
					{
						p[index].ur.first=1;
						p[index].ur.steps=1;
						f=1;
					}
			}
			if (j-2>-1)
			{
				if (i+2<8 && v!=4)
					if (field[i+2][j-2]==0 && field[i+1][j-1]>b && field[i+1][j-1]<=e)
					{
						p[index].dl.first=1;
						p[index].dl.steps=1;
						f=1;
					}
				if (i-2>-1 && v!=2)
					if (field[i-2][j-2]==0 && field[i-1][j-1]>b && field[i-1][j-1]<=e)
					{
						p[index].ul.first=1;
						p[index].ul.steps=1;
						f=1;
					}
			}
			break;
		case 2:
			if (v!=3)
				for (int q=1;i+q<7 && j+q<7;q++)
				{
					if (field[i+q][j+q]>bo && field[i+q][j+q]<=eo) break;
					if (field[i+q][j+q]>b && field[i+q][j+q]<=e)
					{
						for (h=q;i+h<7 && j+h<7 && field[i+h+1][j+h+1]==0;h++);
						if (q!=h)
						{
							p[index].dr.first=q;
							p[index].dr.steps=h-q;
							f=1;
						}
						break;
					}
				}
			if (v!=1)
				for (int q=1;i-q>0 && j+q<7;q++)
				{
					if (field[i-q][j+q]>bo && field[i-q][j+q]<=eo) break;
					if (field[i-q][j+q]>b && field[i-q][j+q]<=e)
					{
						for (h=q;i-h>0 && j+h<7 && field[i-h-1][j+h+1]==0;h++);
						if (q!=h)
						{
							p[index].ur.first=q;
							p[index].ur.steps=h-q;
							f=1;
						}
						break;
					}
				}
			if (v!=4)
				for (int q=1;i+q<7 && j-q>0;q++)
				{
					if (field[i+q][j-q]>bo && field[i+q][j-q]<=eo) break;
					if (field[i+q][j-q]>b && field[i+q][j-q]<=e)
					{
						for (h=q;i+h<7 && j-h>0 && field[i+h+1][j-h-1]==0;h++);
						if (q!=h)
						{
							p[index].dl.first=q;
							p[index].dl.steps=h-q;
							f=1;
						}
						break;
					}
				}
			if (v!=2)
				for (int q=1;i-q>0 && j-q>0;q++)
				{
					if (field[i-q][j-q]>bo && field[i-q][j-q]<=eo) break;
					if (field[i-q][j-q]>b && field[i-q][j-q]<=e)
					{
						for (h=q;i-h>0 && j-h>0 && field[i-h-1][j-h-1]==0;h++);
						if (q!=h)
						{
							p[index].ul.first=q;
							p[index].ul.steps=h-q;
							f=1;
						}
						break;
					}
				}
			break;
		}
		return f;
	}
};