/*
 * nibbles.h
 *
 *  Created on: 06.03.2015
 *      Author: Studi Grandioso
 */

#ifndef NIBBLES_H_
#define NIBBLES_H_
#define SYSTIME_CLOCK	1600	/* timer event rate [Hz] */

#define TRUE  1
#define FALSE 0
#define RIGHT  1
#define LEFT (-1)
#define UP   1
#define DOWN (-1)
#define NOCHANGE 0
#define GAME_OVER (-1)


//Konstanten und Parameter
#define CALIB 5
#define POT_MAX 2750
#define PIX_SIZE 5
#define SPEED (SYSTIME_CLOCK/8)
#define LIVES 5
#define MAX_LEVEL 3
#define MAX_MELOD 3
#define MAX_FREQS 10

#define MAX_EAT 10
#define MAX_WALLS 10
#define MAX_LENGTH 100
#define GRID_HEIGHT 44
#define GRID_WIDTH 64
#define HEAD_X (nib.nibHead.xpos)
#define HEAD_Y (nib.nibHead.ypos)
#define TAIL_X (nib.nibBodyPoints[(nib.bodyLength-1)].xpos)
#define TAIL_Y (nib.nibBodyPoints[(nib.bodyLength-1)].ypos)
#define NIBBLUE 56
#define STARTX 40
#define STARTY 40

struct Tone {
	uint8_t count;
	uint16_t freqs[MAX_FREQS];
	uint16_t durations[MAX_FREQS];
};

struct Melody {
	uint8_t count;
	uint8_t length;
	struct Tone tones[MAX_MELOD];
};


struct gridPoint {
	uint16_t ypos;
	uint16_t xpos;
};

struct NibFigure {
	uint8_t color;
	struct gridPoint nibBodyPoints[MAX_LENGTH];
	struct gridPoint nibHead;
	struct gridPoint nibTail;
	uint16_t bodyLength;
	uint16_t previousLength;

	int directionX;
	int directionY;

	int preyCount;
	uint8_t prey_eaten;
	uint8_t levelCount;
	int speed;
	int lives;
};

typedef struct {
	struct gridPoint stones[10];
} Level;

struct Walls {
	uint8_t color;
	uint8_t length;
	Level levels[MAX_LEVEL];
};


//void initDisplay();
void initDisplay();
void initGame();
int  tick(int diffPotX, int diffPotY);
void calcPos(int diffX, int diffY);
void updateNibBody(int headX, int headY);
void writeSquare(int w, int h, int x, int y, uint8_t color);
int  calcCollision();
int  checkPreyTouch();
void drawPrey();
void drawScreen();
void drawNib(uint8_t color);
void drawWall();
int checkCollWall();
void updateRand(int x,int y);
int loosingRoutine();
void startLevelRoutine();
void clearGameDisplayBuffer(uint8_t color);
void playMusic(uint8_t mus);
#endif /* NIBBLES_H_ */
