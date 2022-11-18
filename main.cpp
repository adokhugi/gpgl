//#define DEBUG

#include <stdio.h>
#include <conio.h>
#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <math.h>

#include "window.h"

GLint winWidth = 256, winHeight = 256;

Window win;

int OPENGL_XRES = winWidth;
int OPENGL_YRES = winHeight;
int OPENGL_FULLSCREEN = false;
int keyDown [256];

const int numProgs = 10;

int variable [numProgs];

class Operator
{
public:
	unsigned char opcode;
	Operator *param [2];
	unsigned char constant;

	Operator ()
	{
	}

	Operator (char _opcode)
	{
		opcode = _opcode;
	}

	void setParam (int n, Operator *program)
	{
		param [n] = program;
	}

	void setConstant (int c)
	{
		constant = c;
	}

	unsigned char evaluate ()
	{
		int i, j;

#ifdef DEBUG
		if (opcode == 'v')
			printf ("v%d ", constant);
		else
			printf ("%c ", opcode);
#endif

		switch (opcode)
		{
		case '+': return param [0]->evaluate () + param [1]->evaluate ();
		case '-': return param [0]->evaluate () - param [1]->evaluate ();
		case '*': return param [0]->evaluate () * param [1]->evaluate ();
		case '/': i = param [1]->evaluate (); if (i) return param [0]->evaluate () / i; else return 0;
		case '&': return param [0]->evaluate () & param [1]->evaluate ();
		case '|': return param [0]->evaluate () | param [1]->evaluate ();
		case '^': return param [0]->evaluate () ^ param [1]->evaluate ();
		case 's': return (int) sin ((double) param [0]->evaluate ());
		case 'c': return (int) cos ((double) param [0]->evaluate ());
//		case 'p': for (i = param [1]->evaluate (), j = param [0]->evaluate (), k = 1; i; i--, k *= j); return k;
		case 'm': i = param [0]->evaluate (); j = param [1]->evaluate (); if (i < j) return i; else return j;
		case 'M': i = param [0]->evaluate (); j = param [1]->evaluate (); if (i > j) return i; else return j;
		case '<': i = param [0]->evaluate (); j = param [1]->evaluate (); if (i < j) return i; else return 0;
		case '>': i = param [0]->evaluate (); j = param [1]->evaluate (); if (i > j) return i; else return 0;
		case '?': i = param [0]->evaluate (); if (i) return param [1]->evaluate (); else return 0;
		case ':': i = param [0]->evaluate (); if (i) return i; else return param [1]->evaluate ();
		case '%': i = param [1]->evaluate (); if (i) return param [0]->evaluate () % param [1]->evaluate (); else return 0;
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			return variable [opcode - '0'];
		case 'v':
			return constant;
		default:
			return 0;
		}
	}

	unsigned char randomOperation ()
	{
		int i = rand () % 16;
		switch (i)
		{
		case 0: return '+';
		case 1:	return '-';
		case 2: return '*';
		case 3: return '/';
		case 4: return '&';
		case 5: return '|';
		case 6: return '^';
		case 7: return 's';
		case 8: return 'c';
		case 9: return 'm';
		case 10: return 'M';
		case 11: return '<';
		case 12: return '>';
		case 13: return '?';
		case 14: return ':';
		case 15: return '%';
		default: return 'n';
		}
	}

	void assign (bool canBeValue)
	{
		if (!canBeValue || rand () % 2)
		{
			opcode = randomOperation ();
			param [0] = new Operator ();
			param [1] = new Operator ();
			param [0]->assign (true);
			switch (opcode)
			{
			case 's':
			case 'c':
				param [1]->opcode = 'n';
				break;

			default:
				param [1]->assign (true);
				break;
			}
		}
		else if (!(rand () % 3))
			opcode = '0' + rand () % numProgs;
		else
		{
			opcode = 'v';
			constant = rand () % 256;
		}
	}

	void mutate (int ugliness)
	{
		Operator *ptr;
		bool t;

		for (ugliness++; ugliness; ugliness--)
		{
			ptr = this;
			for (t = false; !t; )
			{
#ifdef DEBUG
				printf ("mutating operator %c\n", ptr->opcode);
				getch ();
#endif
				if (rand () % 3 && ptr->opcode != 'v' && ptr->opcode != 'n' && (ptr->opcode < '0' || ptr->opcode > '9'))
					ptr = ptr->param [rand () % 2];
				else
				{
					ptr->assign (false);
					t = true;
				}
			}
		}
	}
};

Operator *program [numProgs];

class Listing
{
public:
	int progNum;
	Listing *next;

	Listing ()
	{
	}

	Listing (int _progNum)
	{
		progNum = _progNum;
	}

	void mutate ()
	{
		int i;
		Listing *l, *m;

		l = this;
		for (i = rand () % 255 + 1; i; i--)
			l = l->next;

		m = new Listing (rand () % (numProgs - 4) + 4);
		m->next = l->next;
		l->next = m;

		if (!(rand () % 5) && l->next != this)
		{
			m = l->next;
			l->next = l->next->next;
			delete m;
		}
	}
};

Listing *listing;
Listing *listingPointer;
int selected;

void initPrograms ()
{
	int i;

	srand (GetTickCount ());

	for (i = 0; i < numProgs; i++)
	{
		program [i] = new Operator ();
		program [i]->assign (false);
	}

	program [4]->opcode = '+';
	program [4]->param [0] = new Operator ();
	program [4]->param [0]->opcode = '4';
	program [4]->param [1] = new Operator ();
	program [4]->param [1]->opcode = 'v';
	program [4]->param [1]->constant = '1';
	program [5]->opcode = '0';
	program [6]->opcode = 'v';
	program [6]->constant = 0;

#ifdef DEBUG
	for (i = 0; i < numProgs; i++)
	{
		printf ("testing program %d\n", i);
		getch ();
		program [i]->evaluate ();
		printf ("\n");
		getch ();
	}
#endif

	listing = new Listing (4);
	listing->next = new Listing (5);
	listing->next->next = new Listing (6);
	listing->next->next->next = listing;

	listingPointer = listing;
	variable [0] = 0;
	variable [1] = 0;
	for (i = 4; i < numProgs; i++)
		variable [i] = rand () % 256;

	selected = rand () % 3 + 4;
}

bool keyPressed [3];

int ugliness = 0;

void displayFcn ()
{
	variable [2] = GetTickCount () % 256;
	variable [3] = GetTickCount () / 256;

	for (variable [1] = 0; variable [1] < winHeight; variable [1]++)
		for (variable [0] = 0; variable [0] < winWidth; variable [0]++)
		{
#ifndef DEBUG
			glColor3f (variable [4] / 256.0f, variable [5] / 256.0f, variable [6] / 256.0f);

			glBegin (GL_LINES);
				glVertex2i (variable [0], variable [1]);
				glVertex2i (variable [0], variable [1] + 1);
			glEnd ();

			if (!keyDown ['1']) keyPressed [0] = false;
			if (!keyDown ['2']) keyPressed [1] = false;
			if (!keyDown ['3']) keyPressed [2] = false;

			if (keyDown ['1'] && !keyPressed [0])
			{
				keyPressed [0] = true;
				listing->mutate ();
			}

			if (keyDown ['1'] && !keyPressed [0]
				|| (keyDown ['2'] && !keyPressed [1]))
			{
				ugliness++;
				program [selected]->mutate (ugliness);
			}

			if (keyDown ['2'] && !keyPressed [1])
				keyPressed [1] = true;

			if (keyDown ['3'] && !keyPressed [2])
			{
				ugliness = 0;
				keyPressed [2] = true;
				program [rand () % numProgs]->mutate (ugliness);
				selected = rand () % 3 + 4;
				program [selected]->mutate (ugliness);
			}		
#endif

			do
			{
#ifdef DEBUG
				printf ("progNum: %d\n", listingPointer->progNum);
				getch ();
#endif
				variable [listingPointer->progNum] = program [listingPointer->progNum]->evaluate ();
#ifdef DEBUG
				printf ("\n");
				getch ();
#endif
				listingPointer = listingPointer->next;
			} while (listingPointer != listing);

#ifdef DEBUG
			printf ("POSX: %d, POSY: %d, R: %d, G: %d, B: %d\n", variable [0], variable [1], variable [4], variable [5], variable [6]);
			getch ();
#endif
		}

#ifdef DEBUG
		program [selected]->mutate ();
#endif
}

#ifdef DEBUG
int main ()
{
	initPrograms ();
	while (true)
		displayFcn ();
}
#endif

#ifndef DEBUG
int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE prevInstance, LPSTR cmd, int n)
{
	MSG msg = { 0 };
	bool done = false;
	int tickCount = 0, tickCountLastMouthChange = 0;

	if (!win.create ("Assisted Learning"))
		return 0;

	initPrograms ();

	glEnable (GL_POINT_SMOOTH);
	glHint (GL_POINT_SMOOTH_HINT, GL_NICEST);
	glEnable (GL_LINE_SMOOTH);
	glHint (GL_LINE_SMOOTH_HINT, GL_NICEST);

	glClearColor (1, 1, 1, 1);
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();
	gluOrtho2D (0.0, winWidth, 0.0, winHeight);

	int startTime = GetTickCount ();

	while (!done && !GetAsyncKeyState (VK_ESCAPE))
	{
			if (PeekMessage (&msg, NULL, 0, 0, PM_REMOVE))
			{
				if (msg.message == WM_QUIT)
					done = true;
				else
				{
					TranslateMessage (&msg);
					DispatchMessage (&msg);
				}
			}
			else
			{
				if (GetTickCount () - startTime > 5000)
				{
						glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
						startTime = GetTickCount ();
				}

				displayFcn ();

				SwapBuffers (win.getDC());
			}
	}

	return 1;
}
#endif