/*
 * Snake game
 */

#define _BSD_SOURCE	// Fix for usleep

#include <curses.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

static WINDOW* gameWin;
static int gameOver;
static int imortal;
static int width;
static int height;
static int x, y, fruitX, fruitY, score;
static int tail[100][3]= {{0},{0}};
static int nTail;
static int speedDelay;
static char dir;

static int ticker;
static int debug;
static int step;

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ *
 *  Setup
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

void gameMode()
{
	noecho();			// Hide typing.
	cbreak();			// disable line buffering.
	curs_set(false);		// No visible cursor.
	nodelay(stdscr, true);		// Do not wait for getch()
}

void SetupCurses()
{
	initscr();
	getmaxyx(stdscr, height, width);// Set screen height and width.
	height--;
	gameWin = newwin(height, width, 0, 0);
	keypad(stdscr, true);		// Use advanced keyboard functionality.
	gameMode();
}

void Start()
{
	srand(time(NULL));

	// Nav and positions.
	fruitX 		= rand() % width;
	fruitY 		= rand() % height;
	x		= width / 2;
	y		= height / 2;
	dir		= 'S';

	// Booleans
	ticker  	= 0;
	imortal		= 0;
	gameOver	= 0;

	speedDelay	= 100000;
	nTail		= 0;
	score 		= 0;

	wrefresh(gameWin);
	refresh();
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ *
 *  Draw
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

void drawTail(int* j, int* i)
{
	for (int k = 0; k < nTail; k++) {

		if (*j != 0 && *i != 0 && tail[k][0] == *j && tail[k][1] == *i) {
			//mvwaddch(gameWin, *j, *i, 'o');
			switch (tail[k][2]) {

			case 'L':
				if 	(tail[k][2] == 'L') {
					mvwaddch(gameWin, *j, *i, ACS_HLINE);
				}
				else if	(tail[k][2] == 'U') {
					mvwaddch(gameWin, *j, *i, ACS_URCORNER);
				}
				else if (tail[k][2] == 'D') {
					mvwaddch(gameWin, *j, *i, ACS_LRCORNER);
				}
				break;
			case 'R':
				if	(tail[k][2] == 'R') {
					mvwaddch(gameWin, *j, *i, ACS_HLINE);
				}
				else if	(tail[k][2] == 'U') {
					mvwaddch(gameWin, *j, *i, ACS_ULCORNER);
				}
				else if (tail[k][2] == 'D') {
					mvwaddch(gameWin, *j, *i, ACS_LLCORNER);
				}
				break;
			case 'U':
				if 	(tail[k][2] == 'L') {
					mvwaddch(gameWin, *j, *i, ACS_LLCORNER);
				}
				else if	(tail[k][2] == 'R') {
					mvwaddch(gameWin, *j, *i, ACS_LRCORNER);
				}
				else if	(tail[k][2] == 'U') {
					mvwaddch(gameWin, *j, *i, ACS_VLINE);
				}
				break;
			case 'D':
				if 	(tail[k][2] == 'L') {
					mvwaddch(gameWin, *j, *i, ACS_ULCORNER);
				}
				else if	(tail[k][2] == 'R') {
					mvwaddch(gameWin, *j, *i, ACS_URCORNER);
				}
				else if (tail[k][2] == 'D') {
					mvwaddch(gameWin, *j, *i, ACS_VLINE);
				}
				break;
			default:
				break;
			}
		}
	}
}

void Draw()
{
	werase(gameWin);
	box(gameWin, 0, 0);

	for (int j = 0; j < height; j++) {

		for (int i = 0; i < width; i++) {

			if (j == y && i == x)
				mvwaddch(gameWin, j, i, ACS_DIAMOND);

			else if (j == fruitY && i == fruitX)
				mvwprintw(gameWin, j, i, "F");

			else
				drawTail(&j, &i);
		}
	}

	/*
	 * Set the score
	 */

	mvprintw(height, 2, "Score: %d", score);
	wrefresh(gameWin);
	refresh();
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ *
 *  Debuging
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

void debugSwitch()
{
	if (debug == 0) {
		debug = 1;
		height-=10;
		delwin(gameWin);
		gameWin = newwin(height, width, 0, 0);
		clear();
		wrefresh(gameWin);
		refresh();
	} else {
		debug = 0;
		height+=10;
		delwin(gameWin);
		gameWin = newwin(height, width, 0, 0);
		clear();
		wrefresh(gameWin);
		refresh();
	}
}

void stepMode()
{
	if (step == 0){
		nodelay(stdscr, false);
		step = 1;
	} else {
		nodelay(stdscr, true);
		step = 0;
	}
}


void superpower()
{
	if (imortal == 0)
		imortal = 1;
	else
		imortal = 0;
}

void debugTail()
{
	if (debug == 1) {
		if (y > height-10)
			y = (height-10)/2;
		if (fruitY > height-10)
			fruitY = (height-10)/2;
		for (int i = 0; i < nTail; i++)	{

			mvprintw(height, 14, "y = %2.d, x = %2.d, dir = %2.d.\n", y, x, dir);

			if (i < 10)
				mvprintw(height+1+i,    14,	"y = %2.d, x = %2.d, dir = %2.d.\n",
						tail[i][0], tail[i][1], tail[i][2]);
			else if (i >= 10 && i <= 20)
				mvprintw(height+1+i-10, 44,	"y = %2.d, x = %2.d, dir = %2.d.\n",
						tail[i][0], tail[i][1], tail[i][2]);
			else if (i >= 20 && i <= 20)
				mvprintw(height+1+i-10, 74,	"y = %2.d, x = %2.d, dir = %2.d.\n",
						tail[i][0], tail[i][1], tail[i][2]);
			else if (i >= 30 && i <= 30)
				mvprintw(height+1+i-10, 104,	"y = %2.d, x = %2.d, dir = %2.d.\n", 
						tail[i][0], tail[i][1], tail[i][2]);
			else if (i >= 40 && i <= 40)
				mvprintw(height+1+i-10, 134, 	"y = %2.d, x = %2.d, dir = %2.d.\n", 
						tail[i][0], tail[i][1], tail[i][2]);
			else if (i >= 50 && i <= 50)
				mvprintw(height+1+i-10, 164, 	"y = %2.d, x = %2.d, dir = %2.d.\n", 
						tail[i][0], tail[i][1], tail[i][2]);

			refresh();
		}
	}
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ *
 *  Logic
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

void keys(int* c)
{
	if 	((*c == 'h') || (*c == KEY_LEFT))
		*c = 1;
	else if ((*c == 'j') || (*c == KEY_DOWN))
		*c = 2;
	else if ((*c == 'k') || (*c == KEY_UP))	
		*c = 3;
	else if ((*c == 'l') || (*c == KEY_RIGHT))
		*c = 4;
	else if  (*c == 'x')
		*c = 5;
	else if  (*c == 'i')
		*c = 6;
	else if  (*c == 'd')
		*c = 7;
	else if  (*c == 's')
		*c = 8;
}

void Input()
{
	int c = getch();
	keys(&c);

	switch (c) {

	case 1:
		if (dir == 'R')
			break;
		dir = 'L';
		break;
	case 2:
		if (dir == 'U')
			break;
		dir = 'D';
		break;
	case 3:
		if (dir == 'D')
			break;
		dir = 'U';
		break;
	case 4:
		if (dir == 'L')
			break;
		dir = 'R';
		break;
	case 5:
		if (imortal == 1)
			superpower();
		gameOver = 1;
		break;
	case 6:
		superpower();
		break;
	case 7:
		debugSwitch();
		break;
	case 8:
		stepMode();
		break;
	}
}

/*
 * Add to score and replace fruit.
 */

void fruity()
{
	srand(time(NULL));
	// Generate fruit.
	score 	+= 10;
	fruitX 	= rand() % width;
	fruitY 	= rand() % height;

	// Tail growth.
	nTail++;

	// Speed up
	if (score % 50 == 0)
		speedDelay = speedDelay - 3000;
}

/*
 * An endgame with a get-out clause.
 */

void endGame()
{
	if (imortal == 1) {
		x = width/2;
		y = height/2;
	} else 
		gameOver = 1;

}

void Logic()
{
	/*
	 * Snakes tail.
	 */

	int prevY   =  tail[0][0];
	int prevX   =  tail[0][1];
	int prevDir =  tail[0][2];
	int prev2X, prev2Y, prev2Dir;

	tail[0][0] = y;
	tail[0][1] = x;
	tail[0][2] = dir;

	for (int i = 1; i < nTail; i++) {

		prev2Y     =  tail[i][0];
		prev2X     =  tail[i][1];
		prev2Dir   =  tail[i][2];
		tail[i][0] =  prevY;
		tail[i][1] =  prevX;
		tail[i][2] =  prevDir;
		prevY	   =  prev2Y;
		prevX	   =  prev2X;
		prevDir    =  prev2Dir;
	}

	/*
	 * Snake direction.
	 */

	switch (dir)
	{
		case 'L':
			x--;
			break;
		case 'R':
			x++;
			break;
		case 'U':
			y--;
			break;
		case 'D':
			y++;
			break;
		default:
			break;
	}

	/*
	 * Game boundary; Through the walls or total destruction.
	 */

	if ((x == width-1 || x == 0 || y == height-1 || y == 0 ) && 
			(x == fruitX && y == fruitY)) {

		if (x == width-1)
			x = 1;

		else if (x == 0)
			x = width-1;

		else if (y == height-1)	
			y = 1;

		else if (y == 0)
			y = height-1;

		fruity();

	} else if (x == width || x == -1 || y == height || y == -1)

		endGame();

	/*
	 * Don't bite your own tail.
	 */

	for (int i = 0; i < nTail; i++)
		if (tail[i][0] == y && tail[i][1] == x)
			endGame();

	/*
	 * Get fruity here.
	 */

	if (y == fruitY && x == fruitX)
		fruity();
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ *
 *  Menu
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

void Play()
{
	gameOver = 0;

	while (gameOver == 0) {

		Draw();
		Input();
		Logic();
		usleep(speedDelay);
		ticker++;
		debugTail();
	}
}

void Exit()
{
	wborder(gameWin, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
	wrefresh(gameWin);
	delwin(gameWin);
	endwin();
}

void Menu()
{
	werase(gameWin);
	erase();
	mvprintw((height/2)-2, (width/2)-16,
			"Welcome to Snake Dungeons");
	mvprintw((height/2), (width/2)-16,
			"Enter at your own peril ...");
	wrefresh(stdscr);
	usleep(1200000);
	mvprintw((height/2+2), (width/2)-16,
			"Press 's' to begin, or to run away use 'q'.");
	refresh();

	int c = 'm';

	while ((c = getch()) != 'q') {

		switch (c) {

		case 's':
			werase(stdscr);
			werase(gameWin);
			refresh();
			Start();
			Play();
			c = 'o';
		case 'o':
			mvprintw(height/2, (width/2)-4,
					"Game over!");
			refresh();
			usleep(1200000);
			mvprintw(height/2, (width/2)-17,
					"Would you like to play again?");
			mvprintw((height/2)+1, (width/2)-17,
					"Press 's' to start or 'q' to quit.");
			refresh();
			break;
		default:
			break;
		}
	}
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ *
 *  Main
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

int main()
{
	SetupCurses();
	Menu();
	Exit();
	return 0;
}

