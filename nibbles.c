/*
 * nibbles.c
 *
 *  Created on: 06.03.2015
 *      Author: Studi Grandioso
 */

#include "display.h"
#include "nibbles.h"
#include "stdlib.h"
#include "led.h"



int speed = SPEED;

int getSpeed(){
	return speed;
}

//nach Fressen wird Figur nicht richtig angezeigt und gezeichnet
//Walls einfügen für die Level

//GLOBALE VARRIABLEN
int preyX, preyY;
int randPreyX,randPreyY;

static char charNums[11] = {'0','1','2','3','4','5','6','7','8','9','\0'};
static char liveStatus[8] = {'L','i','v','e','s',0x20,'5','\0'};

static char highScore[7] = {'0','0','0','0','0','0','0'};
static int  scoreCount[5] = { 0,0,0,0,0 };

static struct NibFigure nib = {
	.color = 56,
	.nibBodyPoints = {{0,0}},
	.nibHead = {40,55},
	.nibTail = {35,55},
	.bodyLength = 3,
	.previousLength = 0,
	.directionX = 1,
	.directionY = 0,
	.preyCount = 0,
	.prey_eaten = FALSE,
	.levelCount = 0,
	.speed = SPEED,
	.lives = LIVES
};

static struct Walls walls = {
	.color = COL_RED,
	.length = MAX_WALLS,
	{
		{{{10,10},{11,10},{12,10},{13,10},{14,10},{15,10},{16,10},{17,10},{18,10},{19,10}}},
		{{{20,20},{21,20},{22,20},{23,20},{24,20},{25,20},{26,10},{27,10},{28,10},{29,10}}},
		{{{10,10},{11,10},{12,10},{13,10},{14,10},{15,10},{16,20},{17,20},{18,20},{19,20}}}
	}
};

void updateHighScore(int score100){
	int i,j;
	for (i=0;i<score100;i++){
		scoreCount[0]++;
		for(j=0;j<4;j++){
			if(scoreCount[j]==10){
				scoreCount[j] = 0;
				scoreCount[j+1]++;
			}
		}
	}
	for(i=0;i<4;i++){
		highScore[3-i] = charNums[scoreCount[i]];
	}

}

void clearGameDisplayBuffer(uint8_t color) {
	int i = 0;
	int j = 0;
	for (i = 5; i < (GRID_WIDTH*PIX_SIZE)-PIX_SIZE; i++) {
		for (j = 5; j < (GRID_HEIGHT*PIX_SIZE); j++) {
			setPx(i, j, color);
		}
	}

}

void updateStatusBar(uint8_t color){
	int i = 0;
	int j = 0;
	for (i = 0; i < (GRID_WIDTH*PIX_SIZE); i++) {
		for (j = (GRID_HEIGHT*PIX_SIZE); j < TFT_YSIZE; j++) {
			setPx(i, j, color);
		}
	}
	liveStatus[6] = charNums[nib.lives];
	drawString(150, 225, 7,	liveStatus, COL_BLACK, FONT_STANDARD, 0, 0);
	drawString(250, 225, 6,	highScore, COL_BLACK, FONT_STANDARD, 0, 0);
}

void drawKrizzle(){
	srand(0);
	int i,j,k,r;
	for(i=0;i<20;i++){
		r  = rand()%100;

		if(r<30){
			for(j=0;j<95;j++){
				writePixel(100+j,(140+r),COL_WHITE);
				if(j%7==0){
					for(k=0;k<10;k++){
						writePixel((100+j+(k%2)),140+k,COL_BLACK);

					}
				}
			}
		}

	}
}

void initDisplay(int button){
	clearDisplayBuffer(COL_WHITE);
	drawString(100, 140, 12,	"Nibbles 0.42", COL_BLACK, FONT_STANDARD, 0, 0);
	if(!button){
		drawKrizzle();
	}
	printScrn();
}

////////////////////////////////////////HELPER FUNCTIONS///////////////////////////////////////////////////
void writeSquare(int w, int h, int x, int y, uint8_t color) {
	int i,k;
	if(y+h>239) y= 240-h;
	if(x+w>319) x= 320-w;
	for (i=0; i<h; i++) {
		for (k=0; k<w; k++) {
			writePixel(x+k, y+i, color);
		}
	}
}


void drawWall(){
	int i;
	int wallX,wallY;
	for(i=0; i<walls.length;i++){
		wallX = walls.levels[nib.levelCount].stones[i].xpos;
		wallY = walls.levels[nib.levelCount].stones[i].ypos;
		writeSquare(PIX_SIZE,PIX_SIZE,wallX*PIX_SIZE,wallY*PIX_SIZE,walls.color); 	        								// links unten
		writeSquare(PIX_SIZE,PIX_SIZE,(GRID_WIDTH-wallX)*PIX_SIZE,wallY*PIX_SIZE,walls.color); 				// rechts unten
		writeSquare(PIX_SIZE,PIX_SIZE,wallX*PIX_SIZE,(GRID_HEIGHT-wallY)*PIX_SIZE,walls.color); 			// links oben
		writeSquare(PIX_SIZE,PIX_SIZE,(GRID_WIDTH-wallX)*PIX_SIZE,(GRID_HEIGHT-wallY)*PIX_SIZE,walls.color);	// rechts oben
	}
}

void drawFrame(){
	int i;
	for(i = 0; i<GRID_HEIGHT;i++){
		writeSquare(PIX_SIZE,PIX_SIZE,0,i*PIX_SIZE,COL_RED);
		writeSquare(PIX_SIZE,PIX_SIZE,(TFT_XSIZE-PIX_SIZE),i*PIX_SIZE,COL_RED);
	}
	for(i = 0;i<GRID_WIDTH;i++){
		writeSquare(PIX_SIZE,PIX_SIZE,i*PIX_SIZE,0,COL_RED);
		writeSquare(PIX_SIZE,PIX_SIZE,i*PIX_SIZE,(GRID_HEIGHT*PIX_SIZE),COL_RED);
	}
}

char prey[1] = {'0'};

void updateRand(int x, int y){
	randPreyX = 6+(x*PIX_SIZE);
	randPreyY = (y*PIX_SIZE)-1;
}

void drawPrey(){
	srand(nib.nibBodyPoints[1].xpos+(nib.directionX*nib.nibTail.ypos));
	prey[0] = charNums[nib.preyCount];
	int i;
	int prevPX,prevPY;

	prevPX = preyX;
	prevPY = preyY;

	preyX = randPreyX;
	preyY = randPreyY;


	if(preyX == prevPX || preyY == prevPY){
		preyX = STARTX;
		preyY = STARTY;
	}
	//check if prey is in nib
	for(i = 0; i<nib.bodyLength;i++){
		if(checkPreyTouch(nib.nibBodyPoints[i].xpos,nib.nibBodyPoints[i].ypos)){
			preyX = STARTX;
			preyY = STARTY;			}
	}
	//check if prey is in wall
	for(i = 0; i<walls.length; i++){
		if(checkPreyTouch(walls.levels[nib.levelCount].stones[i].xpos,walls.levels[nib.levelCount].stones[i].xpos)){
			preyX = STARTX;
			preyY = STARTY;
		}
	}
	i = (int)nib.preyCount;
	updateStatusBar(63);
	//check if calculated position is to be populated by a wall
	drawString(preyX,preyY,1,prey,COL_BLACK, FONT_STANDARD, 0, 0);
}

void drawNib(uint8_t color){

	int j = 0;
	while(nib.nibBodyPoints[j].xpos != 0){
		writeSquare(PIX_SIZE, PIX_SIZE, nib.nibBodyPoints[j].xpos, nib.nibBodyPoints[j].ypos, color);
		j++;
	}

	//writeSquare(PIX_SIZE, PIX_SIZE, nib.nibTail.xpos, nib.nibTail.ypos, 54);

}
////////////////////////////////////////HELPER FUNCTIONS END///////////////////////////////////////////////////

void initGame(){
	drawPrey();
	drawScreen();
}


void updateNibBody(int headX, int headY){
	// Falls Nib genug gewachsen -> Pixel loeschen
	if((nib.bodyLength-nib.previousLength) == 0){
		writeSquare(PIX_SIZE, PIX_SIZE, TAIL_X, TAIL_Y, 63);
	}else{
		nib.previousLength++;
	}

	// Nib Array um eins verschieben
	int i;
	for (i = (nib.bodyLength-1); i > 0; i--) {
			*(nib.nibBodyPoints + i) = *(nib.nibBodyPoints + i - 1);
	}

	// neuen Head einfuegen
	nib.nibBodyPoints[0].xpos = headX;
	nib.nibBodyPoints[0].ypos = headY;
	// Tail benennen
//	nib.nibTail.xpos = nib.nibBodyPoints[(nib.bodyLength-1)].xpos;
//	nib.nibTail.ypos = nib.nibBodyPoints[(nib.bodyLength-1)].ypos;



}



//neue Headposition bestimmen
void calcHead(int diffX, int diffY){
	if(nib.directionX == 1 || nib.directionX == -1) {
		if(diffY<-CALIB){
			nib.directionX = 0;
			nib.directionY = -1;
			nib.nibHead.ypos -= PIX_SIZE;
		} else if (diffY > CALIB) {
			nib.directionX = 0;
			nib.directionY = 1;
			nib.nibHead.ypos += PIX_SIZE;
		} else if (nib.directionX == 1) {
			nib.nibHead.xpos += PIX_SIZE;
		} else if (nib.directionX == -1) {
			nib.nibHead.xpos -= PIX_SIZE;
		}
	} else if (nib.directionY == 1 || nib.directionY == -1){
		if (diffX < -CALIB) {
			nib.directionY = 0;
			nib.directionX = -1;
			nib.nibHead.xpos -= PIX_SIZE;
		} else if (diffX > CALIB) {
			nib.directionY = 0;
			nib.directionX = 1;
			nib.nibHead.xpos += PIX_SIZE;
		} else if (nib.directionY == 1) {
			nib.nibHead.ypos += PIX_SIZE;
		} else if (nib.directionY == -1) {
			nib.nibHead.ypos -= PIX_SIZE;
		}
	}
}

int checkCollWall(){
	int i;
	int wallX,wallY;
	for(i=0; i<walls.length;i++){
		wallX = walls.levels[nib.levelCount].stones[i].xpos;
		wallY = walls.levels[nib.levelCount].stones[i].ypos;
		if(HEAD_X == (wallX*PIX_SIZE) || HEAD_X == ((GRID_WIDTH-wallX)*PIX_SIZE)){
			if(HEAD_Y == (wallY*PIX_SIZE) || HEAD_Y == ((GRID_HEIGHT-wallY)*PIX_SIZE)){
				return TRUE;
			}
		}
	}
	return FALSE;
}

int calcCollision(){
	//wenn nib head mit irgendwas kollidiert kollision, sonst normal weiter
	int collision_flag = FALSE;
	//verlassen des Spielfelds
	if(nib.nibHead.xpos >=  (GRID_WIDTH*PIX_SIZE-PIX_SIZE)  || nib.nibHead.ypos >= (GRID_HEIGHT*PIX_SIZE) || nib.nibHead.xpos < 5 || nib.nibHead.ypos < 5){
		collision_flag = TRUE;
	}
	int c;
	//Kollision von NibHead mit NibBody, beginnend bei 3, sonst unmoeglich.
	for (c= 3; c<nib.bodyLength; c++) {
		if(nib.nibHead.xpos == nib.nibBodyPoints[c].xpos && nib.nibHead.ypos == nib.nibBodyPoints[c].ypos) {
			collision_flag = TRUE;
		}
	}
	//for Schleife zum Prüfen auf Kollision mit Wall ergänzen
	if(checkCollWall()){
		collision_flag = TRUE;
	}

	return collision_flag;
}

void winningRoutine() {
	clearDisplayBuffer(COL_WHITE);
	updateHighScore(nib.lives*10);
	drawString(100, 140, 13,"You have Won!", COL_BLACK, FONT_STANDARD, 0, 0);
	drawString(50, 120, 14,"NEW HIGHSCORE:", COL_RED, FONT_STANDARD, 0, 0);
	drawString(180, 120, 7,highScore, NIBBLUE, FONT_STANDARD, 0, 0);

	printScrn();

}

int loosingRoutine(){
	nib.lives--;
	updateStatusBar(COL_WHITE);

	if(nib.lives <= 0){
		clearDisplayBuffer(COL_WHITE);
		drawString(100, 140, 14,"You have Lost!", COL_BLACK, FONT_STANDARD, 0, 0);
		drawString(50, 120, 12,"Total Score:", COL_RED, FONT_STANDARD, 0, 0);
		drawString(175, 120, 6,highScore, NIBBLUE, FONT_STANDARD, 0, 0);
		printScrn();
		return TRUE;
	}
	drawNib(COL_WHITE);
	startLevelRoutine(FALSE);
	return FALSE;
}

void startLevelRoutine(int bool){
	int i;
	for(i = 0;i<nib.bodyLength;i++){
			nib.nibBodyPoints[i].xpos = 0;
			nib.nibBodyPoints[i].ypos = 0;
	}
	if(bool){

		nib.preyCount = 0;
		nib.previousLength = 0;
		nib.bodyLength = 3;
	}



	nib.directionX = 1;
	nib.directionY = 0;



	switch(nib.levelCount) {
		case 1:  nib.nibHead.xpos = 40;nib.nibHead.ypos = 40; break;
		default: nib.nibHead.xpos = 30;nib.nibHead.ypos = 30; break;
	}
}

int checkPreyTouch(int xpos, int ypos){
//	if((xpos == preyX) && (ypos == (preyY+(2*PIX_SIZE)) || ypos == (preyY+PIX_SIZE) /*|| (ypos == (preyY-PIX_SIZE))*/)) {
//		return TRUE;
//	}else{
//		return FALSE;
//	}
	int difx = xpos-preyX;
	int dify = ypos-(preyY+6);
	if(difx < 0){difx = -difx;}
	if(dify < 0){dify = -dify;}
	if(difx < PIX_SIZE+1 && dify < 2*PIX_SIZE){
		return TRUE;
	}else{
		return FALSE;
	}

}


void drawScreen(){
	drawFrame();
	drawWall();
	drawNib(NIBBLUE);
	printScrn();
}


int tick(int diffPotX, int diffPotY){

	calcHead(diffPotX, diffPotY);

	//  --> GAME OVER
	if(calcCollision()){
		if(loosingRoutine()){
			return GAME_OVER;
		}
	}

	// --> GROW / LEVEL UP / WIN
	if(checkPreyTouch(nib.nibHead.xpos,nib.nibHead.ypos)){
		nib.preyCount++;
		updateHighScore(nib.preyCount);

		//check for LEVEL DONE
		if(nib.preyCount >= MAX_EAT){
			nib.levelCount++;
			if(nib.levelCount >= MAX_LEVEL){
				winningRoutine();
				return GAME_OVER;
			}else{
				startLevelRoutine(TRUE);

			}
		// JUST GROWTH
		}
		clearGameDisplayBuffer(COL_WHITE);
		drawPrey();
		//länge der spielfigur anpassen
		speed = SPEED-((SPEED/10)*(nib.preyCount+nib.levelCount));
		nib.previousLength = nib.bodyLength;
		nib.bodyLength += (2*nib.preyCount);
	}

	//Array mit Body der Spielfigur anpassen und den aktuellen Head hineinschreiben
	updateNibBody(nib.nibHead.xpos, nib.nibHead.ypos);


	drawScreen();
	return TRUE;
}
