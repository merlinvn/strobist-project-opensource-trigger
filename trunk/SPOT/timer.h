/*--------------------------------------------------------------------------------------------------

  Name         :  timer.h

  Description  :  Header file for timer lib

  Author       :  2009-03-02 - Till Seyfarth

  History      :  2009-03-02 - First release.

--------------------------------------------------------------------------------------------------*/
#ifndef _TIMER_H_
#define _TIMER_H_

#define NB_TIMERS 6


extern void processTimers(void);
extern void setTimer(unsigned char t, uint16_t time);
extern unsigned char isExpired(unsigned char t);
extern void startTimer(unsigned char t);
extern void stopTimer(unsigned char t);

#endif
