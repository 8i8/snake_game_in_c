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
static int tailX[100], tailY[100];
static int nTail;
static int speedDelay;
static char dir;

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ *
 *  Setup
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

void gameMode()
{
	noecho();			// Hide typeing.
	cbreak();			// disable line buffering.
	curs_set(false);		// No visable cursor.
	nodelay(stdscr, true);		// Do not wait for getch()
}

void menuMode()
{
	//echo();			// Show typing.
	//nocbreak();			// Return to cooked mode.
	//curs_set(true);		// Show cursor.
	nodelay(stdscr, false);		// Pause for getch()
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
	gameMode();
	imortal = 0;
	gameOver = 0;
	speedDelay = 100000;
	dir = 'S';
	x = width / 2;
	y = height / 2;
	srand(time(NULL));
	fruitX = rand() % width;
	fruitY = rand() % height;
	score = 0;
	wrefresh(gameWin);
	refresh();
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ *
 *  Draw
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

void drawTail(int* i, int* j)
{
	for (int k = 0; k < nTail; k++) {

		if (*i != 0 && tailX[k] == *i && tailY[k] == *j)
			mvwprintw(gameWin, *j, *i, "o");
	}
}

void Draw()
{
	werase(gameWin);
	box(gameWin, 0, 0);

	for (int j = 0; j < height; j++) {

		for (int i = 0; i < width; i++) {

			if (j == y && i == x)
				mvwprintw(gameWin, j, i, "@");

			else if (j == fruitY && i == fruitX)
				mvwprintw(gameWin, j, i, "F");

			else
				drawTail(&i, &j);
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
 *  Logic
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

void superpower()
{
	if (imortal == 0)
		imortal = 1;
	else
		imortal = 0;
}

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
	}
}

void fruity()
{
	score += 10;
	srand(time(NULL));
	fruitX = rand() % width;
	fruitY = rand() % height;
	nTail = nTail + 1;

	// Speed up
	if (score % 50 == 0)
		speedDelay = speedDelay - 3000;
}

void endGame()
{
	if (imortal == 1) {
		x = width/2;
		y = height/2;
	} else {
		gameOver = 1;
		//for (int i = 0; i <= nTail; i++) {
		//	tailY[i] = -1;
		//	tailX[i] = -1;
		//}
	}
}

void Logic()
{
	/*
	 * Snakes tail.
	 */

	int prevX = tailX[0];
	int prevY = tailY[0];
	int prev2X, prev2Y;

	tailX[0] = x;
	tailY[0] = y;

	//if (nTail > 1) {
	//	mvwaddch(gameWin, tailY[nTail-1], tailX[nTail-1], ' ');
	//}

	for (int i = 1; i < nTail; i++) {

		prev2X = tailX[i];
		prev2Y = tailY[i];
		tailX[i] = prevX;
		tailY[i] = prevY;
		prevX = prev2X;
		prevY = prev2Y;
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
	 * Game boundry.
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
	 * Don't hit the tail.
	 */

	for (int i = 0; i < nTail; i++)
		if (tailX[i] == x && tailY[i] == y)
			endGame();

	/*
	 * Get fruity here.
	 */

	if (x == fruitX && y == fruitY)
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
			gameOver = 0;
			mvprintw(height/2, (width/2)-4,
					"Game over!");
			refresh();
			usleep(1200000);
			mvprintw(height/2, (width/2)-17,
					"Would you like to play again?");
			mvprintw((height/2)+1, (width/2)-17,
					"Press 's' to start or 'q' to quit.");
			mvprintw((height/2)+2, (width/2)-17, 
					">>> %d", gameOver);
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

