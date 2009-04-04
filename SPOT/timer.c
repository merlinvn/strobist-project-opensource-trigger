/*
 * timer.c
 *
 *  Created on: 04.03.2009
 *      Author: tseyfarth
 */

#include <avr/io.h>
#include "timer.h"

struct timer {
	uint16_t timeLeft;
	unsigned char isActive;
}timers[NB_TIMERS];


void processTimers(void) {
	for(int i=0; i<NB_TIMERS; i++) {
		if( (timers[i].isActive == 1) && (timers[i].timeLeft>0) ) {
			timers[i].timeLeft--;
		}
	}
}

void setTimer(unsigned char t, uint16_t time) {
	timers[t].timeLeft=time;
	timers[t].isActive=1;
}

unsigned char isExpired(unsigned char t) {
	unsigned char ret = 0;
	if( (timers[t].timeLeft == 0) && (timers[t].isActive) ) {
		timers[t].isActive = 0;
		ret = 1;
	}
	return ret;
}

void startTimer(unsigned char t) {
	timers[t].isActive=1;
}

void stopTimer(unsigned char t) {
	timers[t].isActive=0;
}
