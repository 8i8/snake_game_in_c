/*
 * Snake game
 */

#define _BSD_SOURCE	// Fix for usleep

#include <curses.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

#define START_LENGTH		(width*height/300)
#define GROW			((width*height/300)+1)

static WINDOW* gameWin;
static int gameOver;
static int maxWidth;
static int maxHeight;
static int width;
static int height;
static int x, y, fruitX, fruitY, score;
static int tail[200][3] = {{0},{0}};
static int nTail;
static int speedDelay;
static char dir;
static char dir2;

static int grown;
static int imortal;
static int ticker;
static int debug;
static int resize;
static int step;

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ * 
 *  The snake
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

typedef struct node {

	int y;
	int x;
	int type;
	struct node* next;

} sTail;

static sTail* root;
static sTail* counter;

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ *
 *  Setup
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

void gameMode() {

	noecho();			// Hide typing.
	cbreak();			// disable line buffering.
	curs_set(false);		// No visible cursor.
	nodelay(stdscr, true);		// Do not wait for getch()
}

void SetScreenSize() {

	static int hOffset;
	static int wOffset;

	/*
	 * Check for screen size changes.
	 */

	getmaxyx(stdscr, maxHeight, maxWidth);

	if (maxHeight != hOffset || maxWidth != wOffset || resize == 1) {

		height = maxHeight;
		width = maxWidth;
		height--;
		resize = 0;

		if (debug == 1)
			height = height - 9;
		if (x > width)
			x = width/2;
		if (y > height)
			y = height/2;
		if (fruitX > width || fruitY > height) {
			srand(time(NULL));
			fruitX 	= rand() % width;
			fruitY 	= rand() % height;
		}
	}

	/*
	 * Set these values at the end of the function, that the routine check
	 * is faster during program loop.
	 */

	hOffset = maxHeight;
	wOffset = maxWidth;

}

void SetupCurses()
{
	initscr();
	SetScreenSize();
	gameWin = newwin(maxHeight, maxWidth, 0, 0);
	keypad(stdscr, true);		// Use advanced keyboard functionality.
	gameMode();
}

void Start()
{
	srand(time(NULL));

	/* 
	 * Initialize var's
	 */

	speedDelay	= 100000;
	ticker  	= 0;
	dir		= 'R';
	fruitX 		= rand() % width;
	fruitY 		= rand() % height;
	x		= width / 2;
	y		= height / 2;
	score 		= 0;

	/*
	 * Booleans
	 */

	imortal		= 0;
	gameOver	= 0;

	/*
	 * Clean the tail array, who knows who used it last!
	 */

	for (int i = 0; i < nTail; i++)
		tail[i][2] = '\0';

	nTail = START_LENGTH;

	for (int i = 0; i < nTail; i++) {
		tail[i][0] = 0;
		tail[i][1] = 0;
		tail[i][2] = 'R';
	}
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ *
 *  Debugging
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

void Pause()
{
	nodelay(stdscr, false);
	getch();
	nodelay(stdscr, true);
}

/*
 * Fold up the bottom of the screen to reveal details of the coordinates of the
 * snakes tail, to be used for implementing modifications to the tail array
 * behavior,
 */

void debugSwitch()
{
	if (debug == 0) {
		debug = 1;
		resize = 1;
		SetScreenSize();
		wrefresh(gameWin);
	} else {
		debug = 0;
		resize = 1;
		SetScreenSize();
		wrefresh(gameWin);
	}
}

/*
 * Turn off nodelay() mode to allow step by step movement.
 */

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

/*
 * Turn of fatality.
 */

void superpower()
{
	if (imortal == 0)
		imortal = 1;
	else
		imortal = 0;
}

/*
 * Display a table of the tail array data.
 */

void arrayTable(int i, int j, int h, int m, int w) {

	if (i >= j*h && i <= (j+1)*h && maxWidth > m+2*w) {
		mvwprintw(gameWin, height+i-j*h, m+j*w, "y:%-4dx:%-4dd:%-3d\n",
				tail[i][0], tail[i][1], tail[i][2]);
	}
}

/*
 * Precise details of the snakes tail array, displayed below the main screen
 * when initialised; the above function write from this one..
 */

void debugTail()
{
	int h = 10;
	int m = 15;	// left margin for array columns.
	int w = 17;	// separation of array columns.

	if (debug == 1) {

		mvwprintw(gameWin, height+1, 4, "  y: %-3.d", y);
		mvwprintw(gameWin, height+2, 4, "  x: %-3.d", x);
		mvwprintw(gameWin, height+3, 4, "dir: %c\n", dir);

		for (int i = 0; i < nTail; i++)	{
			for (int j = 0; j < (width-m)/w; j++)
				arrayTable(i, j, h, m, w);
		}
	}
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ *
 *  Draw
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

void drawTail(int* j, int* i)
{
	for (int k = 0; k < nTail; k++) {
		if (*j != 0 && *i != 0 && tail[k][0] == *j && tail[k][1] == *i) {

			/*
			 * Print the correct glyph for the snakes body.
			 */

			switch (tail[k][2]) {

			case 1:
				mvwaddch(gameWin, *j, *i, ACS_HLINE);
				break;
			case 2:
				mvwaddch(gameWin, *j, *i, ACS_VLINE);
				break;
			case 3:
				mvwaddch(gameWin, *j, *i, ACS_ULCORNER);
				break;
			case 4:
				mvwaddch(gameWin, *j, *i, ACS_URCORNER);
				break;
			case 5:
				mvwaddch(gameWin, *j, *i, ACS_LRCORNER);
				break;
			case 6:
				mvwaddch(gameWin, *j, *i, ACS_LLCORNER);
				break;
			default:
				break;
			}
		}
	}
}

void Draw() {

	/*
	 * TODO this screen wipe needs to be replaced with a localised redraw
	 * of only the new snake segment, and that which has been removed from
	 * the end of the tail. The global screen refresh at the end of the
	 * function needs to be local, perhaps the entire section split into
	 * two functions; One for the initial draw and the other for local game
	 * movement.
	 *
	 * If removed at this time the tail remains infinitly visable on
	 * screen.
	 */

	werase(gameWin);

	/*
	 * Borders
	 */

	for (int i = 1; i < width-1; i++)
		mvwaddch(gameWin, 0, i, ACS_HLINE);
	for (int i = 1; i < width-1; i++)
		mvwaddch(gameWin, height-1, i, ACS_HLINE);
	for (int i = 1; i < height-1; i++)
		mvwaddch(gameWin, i, 0, ACS_VLINE);
	for (int i = 1; i < height-1; i++)
		mvwaddch(gameWin, i, width-1, ACS_VLINE);

	/*
	 * Corners
	 */

	mvwaddch(gameWin, 0, 0, ACS_ULCORNER);
	mvwaddch(gameWin, 0, width-1, ACS_URCORNER);
	mvwaddch(gameWin, height-1, 0, ACS_LLCORNER);
	mvwaddch(gameWin, height-1, width-1, ACS_LRCORNER);

	/*
	 * Objects on the matrix
	 */

	for (int j = 0; j < height; j++) {
		for (int i = 0; i < width; i++) {
			if (j == y && i == x && (grown == 1))
				mvwaddch(gameWin, j, i, 'O');
			else if	(j == y && i == x && (grown == 0))
				mvwaddch(gameWin, j, i, 'o');
			else if (j == fruitY && i == fruitX)
				mvwaddch(gameWin, j, i, 'X');
			else
				drawTail(&j, &i);
		}
	}

	/*
	 * Set the score
	 */

	mvprintw(height, 2, "Score: %d", score);

	/*
	 * Indicate debugging with deathless death.
	 */

	if (imortal == 1)
		mvwaddch(gameWin, height-2, width-2, 'I');

	if (debug == 1)
		debugTail();

	wrefresh(gameWin);
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ *
 *  Logic sub routines
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/*
 * Keys() takes pointers from the user Input() function, permitting multiple
 * keys linked to the same switch case.
 */

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

/*
 * Gather user input from the keyboard.
 */

void Input()
{
	int c = getch();
	keys(&c);

	dir2 = dir;

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
 * Use rand() to reduce the probability of the occurrence of fruit placed on
 * the window border line.
 */

void reduceProbability()
{
	srand(time(NULL)-777);

	while (fruitX == width-1 || fruitX == 0 || fruitY == height-1 || fruitY == 0 ) {

		int i = rand();

		if ((i % 6) == 0)
			break;
		else {
			fruitX 	= rand() % width;
			fruitY 	= rand() % height;
		}
	}
}

/*
 * Add to the score and replace fruit.
 */

void fruity()
{
	/*
	 * Generate new fruit placements, if they occur on the screen border,
	 * run a further  rand command to decide whether or not to leave them
	 * in place.
	 *
	 * Border cases:
	 *
	 * (fruitX == width-1 || fruitX == 0 || fruitY == height-1 || fruitY == 0 
	 */

	srand(time(NULL));
	score 	+= 10;
	fruitX 	= rand() % width;
	fruitY 	= rand() % height;
	if (fruitX == width-1 || fruitX == 0 || fruitY == height-1 || fruitY == 0 )
		reduceProbability();

	for (int i = 0; i <= nTail; i++)
		if (fruitY == tail[i][0] && fruitX == tail[i][1])
			fruity();

	/*
	 * Tail growth.
	 */

	nTail++;

	/*
	 * Increase snakes speed.
	 */

	if (score % 50 == 0)
		speedDelay = speedDelay - 3000;
}

/*
 * An endgame with a get-out clause.
 *
 * (x == width || x == -1 || y == height || y == -1)
 */

void endGame()
{
	if (imortal == 1) {

		if (x == width)
			x = 0;
		else if	(x == -1)
			x = width-1;
		else if	(y == height)	
			y = 0;
		else if (y == -1)
			y = height-1;

	} else 
		gameOver = 1;

}

void setTail(int* type, int* y, int* x)
{
	/*
	 * Set the snakes tail.
	 */

	switch (*type)
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
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ *
 *  Logic main
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

void Logic()
{
	/*
	 * Snakes tail, shift the details of each tail segment back one place
	 * in the array, the second dimension of the array holds three values.
	 *
	 * 	0 = y
	 * 	1 = x
	 * 	2 = tail segments.
	 *
	 * Map glyphs for the snakes tail while cornering.
	 *
	 * 	1	ACS_HLINE
	 * 	2	ACS_VLINE
	 * 	3	ACS_ULCORNER
	 * 	4	ACS_URCORNER
	 * 	5	ACS_LRCORNER
	 * 	6	ACS_LLCORNER
	 */

	//root = malloc(sizeof(struct node));
	//root->next = 0;
	//counter = root;

	//root->x = 

	int prevY   =  tail[0][0];
	int prevX   =  tail[0][1];
	int prevDir =  tail[0][2];
	int prev2X, prev2Y, prev2Dir;

	tail[0][0] = y;
	tail[0][1] = x;

	if 	((dir == dir2) && (dir == 'R' || dir == 'L'))
		tail[0][2] = 1;
	else if	((dir == dir2) && (dir == 'U' || dir == 'D'))
		tail[0][2] = 2;
	else if	((dir == 'R' && dir2 == 'U') || (dir == 'D' && dir2 == 'L'))
		tail[0][2] = 3;
	else if	((dir == 'D' && dir2 == 'R') || (dir == 'L' && dir2 == 'U'))
		tail[0][2] = 4;
	else if	((dir == 'L' && dir2 == 'D') || (dir == 'U' && dir2 == 'R'))
		tail[0][2] = 5;
	else if	((dir == 'U' && dir2 == 'L') || (dir == 'R' && dir2 == 'D'))
		tail[0][2] = 6;

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
	 * Move the snakes head.
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
	 * Set the snakes head size to grow at a specific length.
	 */

	if (nTail >= GROW)
		grown = 1;

	/*
	 * Game boundary; Through the walls or total destruction? If fruit is
	 * placed on the boundary and is eaten by the snake, yes a vegetarian
	 * snake, the snake passes through the boundary an emerges on the
	 * opposite side of the screen.
	 */

	if ((x == width-1 || x == 0 || y == height-1 || y == 0 ) && 
			(x == fruitX && y == fruitY)) {

		if (x == width-1)
			x = 0;

		else if (x == 0)
			x = width-1;

		else if (y == height-1)	
			y = 0;

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

	/*
	 * This loop runs throughout game play being essentially the crux of
	 * this program.
	 */

	while (gameOver == 0) {

		SetScreenSize();
		Draw();
		Input();
		Logic();
		usleep(speedDelay);
		ticker++;
	}
}

/*
 * Program shut down.
 */

void Exit()
{
	wborder(gameWin, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
	wrefresh(gameWin);
	delwin(gameWin);
	endwin();
}

/*
 * Program menu.
 */

void Menu()
{
	erase();
	mvprintw((height/2)-2, (width/2)-16,
			"Welcome to Snake Dungeons");
	mvprintw((height/2), (width/2)-16,
			"Enter at your own peril ...");
	refresh();
	usleep(1200000);
	mvprintw((height/2+2), (width/2)-16,
			"Press 's' to begin, or to run away use 'q'.");
	refresh();

	int c;

	while ((c = getch()) != 'q') {

		switch (c) {

		case 's':
			werase(stdscr);
			werase(gameWin);
			refresh();
			Start();
			Play();
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

