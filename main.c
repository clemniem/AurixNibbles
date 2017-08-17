/**
 * \file main.c
 * \copyright Copyright (c) 2013 Infineon Technologies AG. All rights reserved.
 *
 *
 * 1.1
 * 2014-02-11
 *
 *                                 IMPORTANT NOTICE
 *
 *
 * Infineon Technologies AG (Infineon) is supplying this file for use
 * exclusively with Infineon's microcontroller products. This file can be freely
 * distributed within development tools that are supporting such microcontroller
 * products.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 * OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 * INFINEON SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 * OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 *
 */

#include "display.h"
#include <string.h>
#include "led.h"
#include "interrupts.h"
#include <stdlib.h>
#include "bspconfig.h"
#include "timer.h"
#include "nibbles.h"

#include "tc23xa/IfxVadc_reg.h"
#include "tc23xa/IfxSrc_reg.h"
#include "tc23xa/IfxPort_reg.h"
#include "tc23xa/IfxGtm_reg.h"


#ifdef __TRICORE__
#ifndef __TC161__
#include <machine/intrinsics.h>
#endif /* !__TC161__ */

#ifdef __TC161__
#include "system_tc2x.h"
#endif /* __TC161__ */
#endif /* __TRICORE__ */


/* Important variables and definitions */
//#define SYSTIME_CLOCK	1600	/* timer event rate [Hz] */ In NIBBLES.h
#define READ_POT_X ((TFT_XSIZE)*((100*VADC_G0RES0.B.RESULT)/POT_MAX)/100)
#define READ_POT_Y ((TFT_YSIZE)*((100*VADC_G0RES1.B.RESULT)/POT_MAX)/100)

volatile uint32_t interruptCount = 0;
static volatile int event_flag = 0;
static volatile unsigned int runCount = 0;

/* timer callback handler */
static void my_timer_handler(void)
{
	event_flag = 1;
	++runCount;
}

//uint16_t endX, endY;
int newPotX, oldPotX, diffPotX;
int newPotY, oldPotY, diffPotY;

static struct Melody mel = {
		.count = 0,
		.length = 10,
		.tones =
		{
			{
			  .count = 10,
			  .freqs = {8,4,3,6,6,8,5,7,5,9},
			  .durations = {800,400,200,200,200,800,400,400,100,50}
			},{
					  .count = 3,
					  .freqs = {8,5,2},
					  .durations = {200,100,2000}
			},{
					  .count = 0,
					  .freqs = {8,4,3,6,6,8,5,7,5,9},
					  .durations = {1600,800,400,400,400,1600,800,800,200,100}
			}
		}

};

int freq;
int dur = 0;
int c = 0;
int fcount = 0;

void playMusic(uint8_t mus){
	++mel.count;
	if(dur > 0){
		--dur;
		if(mel.count >= freq ){
			mel.count = 0;
			PORT33_TOGGLE(1);
		}
	}else{
		if(fcount < mel.tones[mus].count){
			dur = mel.tones[mus].durations[fcount];
			freq = mel.tones[mus].freqs[fcount];
			fcount++;
		}else{
			fcount = 0;
		}
	}

}

static void VADCInterrupt(){
	interruptCount++;
}

void initTimer(){
	#ifdef __TC161__
		SYSTEM_Init();
	#endif /* __TC161__ */

	/* initialise timer at SYSTIME_CLOCK rate */
	TimerInit(SYSTIME_CLOCK);
	/* add own handler for timer interrupts */
	TimerSetHandler(my_timer_handler);

	#ifdef __TRICORE__
	#ifndef __TC161__
		/* enable global interrupts */
		_enable();
	#endif /* !__TC161__ */
	#endif /* __TRICORE__ */
	// Timer Setup END

}

// Versuch eien GTM Tom zu starten....
//	static Ifx_GTM * const portGTM = (Ifx_GTM *)&MODULE_GTM;
//	Ifx_CPU
//
//void initPWM() {
//	Ifx_C
//
//	SYSTEM_DisableProtectionExt (0); 		// Access to ENDINIT-prot. reg.
//	portGTM->CLC.B.DISR = 0;
//	SYSTEM_EnableProtectionExt (0); 		// Lock ENDINIT-protected reg.
//	portGTM->ADCTRIG0OUT0.B.SEL0 = 5;
//	portGTM->TOM.TGC0_GLB_CTRL.B.UPEN_CTRL0 = 0b11;
//	portGTM->TOM.TGC0_ENDIS_CTRL.B.ENDIS_CTRL0 = 0b10;
//	portGTM->TOM.TGC0_ENDIS_STAT.B.ENDIS_STAT0 = 0b11;
//	//portGTM->TOM.CH0.CTRL.B.TRIGOUT
//
//
//
//	//enable Clocks
//	portGTM->CMU.CLK_EN.B.EN_CLK0 = 1;
//}


void initPotentiometer(){
	volatile unsigned long dummy;

	InterruptInit();
	_enable();

	//=========================> Load global module registers
	SYSTEM_DisableProtectionExt (0); 		// Access to ENDINIT-prot. reg.
	VADC_CLC.U = 0x0000; 					// Enable module clock and ctrl.
	dummy = VADC_CLC.U; 					// Read back ensures write oper.
	SYSTEM_EnableProtectionExt (0); 		// Lock ENDINIT-protected reg.

	//=========================> Enable converter for group 0
	VADC_G0ARBCFG.B.ANONC  =  0x3; 			// ANONC = 11, analog converter ON
	VADC_GLOBCFG.B.SUCAL  =  1; 			/* SUCAL = 1, start-up calib. */
	VADC_GLOBCFG.B.DIVWC = 1; 				/* DIVWC = 1, enable write */
	VADC_GLOBCFG.B.DIVA = 0x9; 				// DIVA = 9 (clock prescaler)
	VADC_G0ARBPR.B.ASEN2 = 1; 				// AREN2 = 1, enable arb. slot 2

	// (= background source)
	//VADC_G0CHCTR0.B.RESTBS = 1; 			// RESTBS = 1, global result reg.
	VADC_G0CHCTR0.B.RESTBS = 0; // <-------
	VADC_G0CHCTR1.B.RESTBS = 0; // <-------
	VADC_G0CHCTR1.B.RESREG = 0x1;

	VADC_BRSSEL0.U = 0x3; 					// Select CH0 + CH1 of group 0 for scan
	VADC_BRSMR.U = 0x17; 					// ENGT = 10B, enable conv. req.

	//=========================> Init and install service request
	//SRC_VADCCG0SR0 = (1<<10) /* Enable SR node 0, group 0 */
	//|VADC0INT; // Set prio to <VADC0INT>(1..255)
	//interruptHandlerInstall (VADC0INT, & VADC_SCAN_irq);

	SRC_VADCCG0SR0.U = (1<<10);
	SRC_VADCCG0SR1.U = (1<<10);

	InterruptInstall(SRC_ID_VADCG0SR0, VADCInterrupt, 1, 0);
	//InterruptInstall(SRC_ID_VADCG0SR1, VADCInterrupt, 1, 0);

	VADC_GLOBRCR.B.SRGEN =  1; 				// SRGEN = 1, result service requ.

	//=========================> Wait for completion of startup cal.
	while((VADC_G0ARBCFG.U & 0x30000000) != 0x20000000);

	// CALS = 1, CAL = 0: calibr. done
	//=========================> Start a conversion
	VADC_BRSMR.B.LDEV  =  1; 				// LDEV = 1, generate a load event


	// NOISE REDUCTION ATTEMPT
	VADC_G0RCR0.B.DRCTR = 0x0;

}

void showWelcomeScreen(int time){
	int timeToStart = 0;
	//Begrüßungstext 3 Sek anzeigen
	while(timeToStart < time){
		if(event_flag){
			event_flag = 0;
			timeToStart++;
		}
	}
	clearGameDisplayBuffer(COL_WHITE);
    printScrn();
}

void getPotInput(){
	VADC_BRSMR.B.LDEV  =  1;
	// X ACHSE
	newPotX = READ_POT_X;
	diffPotX = oldPotX-newPotX;
	// Y ACHSE
	newPotY = READ_POT_Y;
	diffPotY = oldPotY-newPotY;
}





int main() {
	// init Pulldown Res
	port0->IOCR0.B.PC0 = 0x1; // Secret Button to Pin 00.0
	port0->IOCR0.B.PC1 = 0x1; // On Button to Pin 00.1
	port33->IOCR0.B.PC0 = 0x1;


	// INIT SYSTEM
	initTimer();
	SYSTEM_Init();

	while(!P00_IN.B.P1){
		// OFF
	}

	InitLED();
	DISPLAY_Init();
    int noton = TRUE;
	while(P00_IN.B.P0){
		//Show broken Screen
		if(noton){
			noton = FALSE;
			initDisplay(FALSE);
		}
	}



	initPotentiometer();
	initDisplay(TRUE);

	// INIT NIBBLES
	showWelcomeScreen(5*SYSTIME_CLOCK);
	initGame();

	// INIT Potentiometer Values
	oldPotX = READ_POT_X;
	oldPotY = READ_POT_Y;

	srand((oldPotX+oldPotY)%POT_MAX);

	LED_ON(1);
	LED_OFF(2);

    int blub = 0;

	while(1){
		//get Potentiometer Values


		// STEPS
		if(event_flag){
			++blub;
			event_flag = 0;

			getPotInput();
			updateRand((rand()%(GRID_WIDTH-10)),(rand()%(GRID_HEIGHT-15)));

			playMusic(0);

			if(blub == getSpeed()){
				blub = 0;

				//tick als zentralen Einstieg in die Spiellogik
				int keepPlaying = tick(diffPotX,diffPotY);
				if (keepPlaying > 0) {
					oldPotX = newPotX;
					oldPotY = newPotY;
					LED_TOGGLE(3);
				} else {
					break;
				}
			}

		}



	}
	return 0;
}






