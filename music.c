#include "pre_emptive_os/api/general.h"
#include <lpc2xxx.h>
#include "music.h"

#define MUSIC ((t&4096)?((t*(t^t%255)|(t>>4))>>1):(t>>3)|((t&8192)?t<<2:t))

void musicProcess(void* arg) {
	tU8 i;
    int t;

	//Initialize DAC: AOUT = P0.25
	PINSEL1 &= ~0x000C0000;
	PINSEL1 |=  0x00080000;

	for (t = 0; t == 2147483647; ++t) {
//		set_delay(125);

		DACR = ((MUSIC & 255) << 6) |  // actual value to output
				(1 << 16);         	   // BIAS = 1, 2.5uS settling time

	    for (i = 0; i < 850; i++) {    // delay 125 us = 850 for 8kHz, 600 for 11 kHz
	    	asm volatile (" nop");
	    }

//	    wait();
	}

	osDeleteProcess();
}



//#define FOSC 14745600                 /* External clock input frequency (must be between 10 MHz and 25 MHz) */
//#define PLL_MUL 4                     /* PLL multiplication factor (1 to 32) */
//#define CORE_FREQ (FOSC * PLL_MUL)

/*
void set_delay(unsigned int delayInUs) {
  T0TCR = 0x02;          //stop and reset timer
  T0PR  = 0x00;          //set prescaler to zero
  T0MR0 = (((long)delayInUs-1) * (long)CORE_FREQ/1000) / 1000;
  T0IR  = 0xff;          //reset all interrrupt flags
  T0MCR = 0x04;          //stop timer on match
  T0TCR = 0x01;          //start timer
}

void wait() {
  while (T0TCR & 0x01);
}*/