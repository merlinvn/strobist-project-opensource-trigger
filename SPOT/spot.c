/**
 *  \brief Main module of the Strobist Project Opensource Trigger
 *  \author Till Seyfarth
 *  \version 1.0
 *  \date    2009-03-19
 *
 * Date        Author         Changes
 * ----------  ---------      ------------------
 * 2009-03-19  Till Seyfarth  first documented version
 *
 */

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include "lcd3110.h"
#include "rfm12.h"
#include "timer.h"

static void init(void);

static volatile unsigned char counter;
static signed char blinks = 0;

ISR(INT0_vect) {						// shutter interrupt
	transmitFire( getID() );
	PORTD |= LCD_BL;
	setTimer(0, 300);
}

ISR(INT1_vect) {						// FIFO ready interrupt
	onReceive( rf12_trans(0xB000) );
}

ISR(TIMER0_OVF_vect) {					// timer interrupt: counted to 256 (at 1/8 clock)
	counter++;
}


int main (void) {

	init();

	while(1) {

        if (counter >= 4) { // 3906,25Hz interrupt frequency => divided by 1000 for ms-ticks => 2.4% too slow
            counter = 0;
			processTimers();
		}

		if( isExpired(0) ) {
			PORTD &= ~LCD_BL;				// LED off
		}

		if( isExpired(1) ) {				// poll buttons
			if( !(PINC & (1<<PINC1)) ) {	// test
				onCursor( CURSOR_PRESS );
			}
			if( !(PINC & (1<<PINC2)) ) {	// down
				onCursor( CURSOR_DOWN );
			}
			if( !(PINC & (1<<PINC3)) ) {	// up
				onCursor( CURSOR_UP );
			}
			if( !(PINC & (1<<PINC4)) ) {	// right
				onCursor( CURSOR_RIGHT );
			}
			if( !(PINC & (1<<PINC5)) ) {	// left
				onCursor( CURSOR_LEFT );
			}
			setTimer(1, 200);
		}

		if( isExpired(2) && waitingForACK ) { // waited too long for ACK; Rx -> Tx
//			displayHex(0xff, 0);			// for debugging!
			setTxMode();
			waitingForACK = 0;
			PORTD &= ~LCD_BL;				// LED off
			blinks = 4;
			setTimer(5, 1);					// blink
		}

		if( isExpired(3) ) {				// flash fired
			PORTD &= ~(1<<PD1);   			// quench signal off
		}

		if( isExpired(4) ) {				// stop waiting for 2nd byte of transmission
			expectingSecondByte = 0;
		}

		if( isExpired(5) && (blinks >= 0) ) {				// blink "Off" twice, then change str. to 0
			if( blinks > 0 ) {
				if( (blinks%2) == 0 ) {			// erase strength display
					eraseStrengthTx( getCursor() );
					setTimer(5, 300);
				} else {
					displayCurrentStrengthTx( getCursor() );
					setTimer(5, 300);
				}
			}
//			else { // done blinking, set strength to zero
//				setStrengthTx( 0, getCursor() );
//			}
			blinks--;
		}
	}
}

void init() {

    //  Set input bits
	DDRC &= ~( (1 << DDC1) | (1<<DDC2) | (1<<DDC3) | (1<<DDC4)| (1<<DDC5) ); // pins 2-5 of port c as input
    PORTC |= ( (1 << PC1) | (1 << PC2) | (1 << PC3) | (1 << PC4) | (1 << PC5) ); // pullup (??? oder eher set high ???)


    //  Set output bits for LCD
    PORTD |= LCD_RST_PIN;
	DDRB |= LCD_CE_PIN | SPI_MOSI_PIN | SPI_CLK_PIN;
	DDRD |= LCD_RST_PIN | LCD_DC_PIN | LCD_BL;
	PORTD &= ~LCD_BL;

    //  Set output bits for RFM12 (chip select)
	PORTB |= RFM12_CS_PIN;
	DDRB  |= RFM12_CS_PIN;

    //  Set output bits for optocouplers
    DDRD |= (1<<PD0) | (1<<PD1);

    //  Enable SPI port: No interrupt, MSBit first, Master mode, CPOL->0, CPHA->0, Clk/8
    SPCR = 0x51;			//
	SPSR |= (1<<SPI2X);			// SPI2x = 1

	LcdInit();

	// RFM12 Init
	rf12_init();					// ein paar Register setzen (z.B. CLK auf 10MHz)
	rf12_setfreq(RF12FREQ(433.92));	// Sende/Empfangsfrequenz auf 433,92MHz einstellen
	rf12_setbandwidth(4, 1, 4);		// 200kHz Bandbreite, -6dB Verstärkung, DRSSI threshold: -79dBm
	rf12_setbaud(56000);			// 56000 baud
	rf12_setpower(0, 6);			// 1mW Ausgangangsleistung, 120kHz Frequenzshift


	// Interrupt INT0: Tx send fire on hotshoe contact
	DDRD &= ~(1<<DDD2);		// D2 als Eingang
	PORTD |= (1<<PD2);		// pull-up aktivieren (Interrupt bei auf Masse ziehen)
	MCUCR |= (1<<ISC01);
	MCUCR &= ~(1<<ISC00);		// MCR Control Register: Interrupt on falling edge of D2
	GICR |= (1<<INT0);		// Gerneral Interrupt Control Register: Enable INT0

	// Interrupt INT1: receive data from RFM12 on FFST->D3/INT1 rising edge
	DDRD &= ~(1<<DDD3);		// D3 als Eingang
	MCUCR |= ((1<<ISC11) | (1<<ISC10));		// MCR Control Register: Interrupt on rising edge of D3

	// Timer 0 Interrupt
	TIMSK |= (1 << TOIE0);  		// enable Timer Overflow Interrupt
	TCCR0 |= (0 << CS02) | (1 << CS01) | (0<<CS00);  // prescaler=8 and 8bit-count to 256 => 3906,25Hz interrupt frequency @ 8MHz F_CPU


	sei();				// Interrupt enable: Tx: Trigger; Rx: RFM12 new data in FIFO; both: timer

#ifdef BACKLIGHT_INSTALLED
	PORTD |= LCD_BL;
	setTimer(0, 5000);	// activate backlight for 5s at startup
#endif

	setTimer(1, 200);	// timer for pin polling

}
