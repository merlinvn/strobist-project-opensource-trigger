/**
 *  \brief Module controlling the RFM12B radio unit
 *  \author Benedikt K. (all low-level driver functions "rfm12_")
 *  \author Till Seyfarth (the rest: mainly SPOT protocol handler)
 *
 *  \version 1.0
 *  \date    2009-03-19
 *
 * Date        Author         Changes
 * ----------  ---------      ------------------
 * 2009-03-19  Till Seyfarth  rfm12_trans now uses hardware SPI instead of software bit-banging
 *
 */
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "global.h"
#include "rfm12.h"
#include "lcd3110.h"
#include "timer.h"

#define RF_PORT	PORTB
#define RF_DDR	DDRB
#define RF_PIN	PINB

#define SCK		5
#define SDO		4
#define SDI		3
#define CS		2

unsigned char encodeFireAndID(unsigned char fire, unsigned char id);
unsigned char encodeGroupAndValue(unsigned char group, unsigned char value);
unsigned char checkFireAndID(unsigned char data);
unsigned char checkGroupAndValue(unsigned char data);

static unsigned char buffer[2];
unsigned char waitingForACK = 0;
unsigned char expectingSecondByte = 0;

int timeTableSB24[] = { // 1 ^= 1/100.000s
		0, 			// Off
		150, 		// Full
		64, 		// 1/2
		32,	 		// 1/4
		19,	 		// 1/8
		13,	 		// 1/16
		9,			// 1/32
		7 };		// 1/64

unsigned short rf12_trans(unsigned short wert) {
	// HW-SPI
	unsigned char c2 = (unsigned char)((wert) & 0x00ff);
	unsigned char c1 = (unsigned char)((wert>>8) & 0x00ff);

	cbi(RF_PORT, CS);
	SPDR = c1;							// Send data
	while ( (SPSR & 0x80) != 0x80 );	// Wait until Tx register empty
	SPDR = c2;							// Send data
	while ( (SPSR & 0x80) != 0x80 );	// Wait until Tx register empty
	sbi(RF_PORT, CS);

	return SPDR;
}

void rf12_init(void) {

	for (unsigned char i=0; i<10; i++)
		_delay_ms(10);			// wait until POR done

	rf12_trans(0xC0E0);			// AVR CLK: 10MHz (14. Low Batt. Detector & MC Clock Div. Comm.)
	rf12_trans(0x80D7);			// Enable FIFO (1. Config Setting Command)
	rf12_trans(0xC2AB);			// Data Filter: internal (6. Data Filter Comm.)
	rf12_trans(0xCA81);			// Set FIFO mode (7. FIFO and Reset Mode Comm.)
	rf12_trans(0xE000);			// disable wakeuptimer (12.)
	rf12_trans(0xC800);			// disable low duty cycle (13.)
	rf12_trans(0xC4F7);			// AFC settings: autotuning: -10kHz...+7,5kHz (9.)
}

void rf12_setbandwidth(unsigned char bandwidth, unsigned char gain, unsigned char drssi) {
	rf12_trans(0x9400|((bandwidth&7)<<5)|((gain&3)<<3)|(drssi&7));
}

void rf12_setfreq(unsigned short freq) {
	if (freq<96)				// 430,2400MHz
		freq=96;
	else if (freq>3903)			// 439,7575MHz
		freq=3903;
	rf12_trans(0xA000|freq);
}

void rf12_setbaud(unsigned short baud) {
	if (baud<663)
		return;
	if (baud<5400)					// Baudrate= 344827,58621/(R+1)/(1+CS*7)
		rf12_trans(0xC680|((43104/baud)-1));
	else
		rf12_trans(0xC600|((344828UL/baud)-1));
}

void rf12_setpower(unsigned char power, unsigned char mod) {
	rf12_trans(0x9800|(power&7)|((mod&15)<<4));
}

void rf12_ready(void) {
	cbi(RF_PORT, CS);
	while (!(RF_PIN&(1<<SDO))); // wait until FIFO ready
}

void rf12_txdata(unsigned char *data, unsigned char number) {
	unsigned char i;
	rf12_trans(0x8238);			// TX on
	rf12_ready();
	rf12_trans(0xB8AA);
	rf12_ready();
	rf12_trans(0xB8AA);
	rf12_ready();
	rf12_trans(0xB8AA);
	rf12_ready();
	rf12_trans(0xB82D);
	rf12_ready();
	rf12_trans(0xB8D4);
	for (i=0; i<number; i++)
	{		rf12_ready();
		rf12_trans(0xB800|(*data++));
	}
	rf12_ready();
	rf12_trans(0x8208);			// TX off
}

void rf12_rxdata(unsigned char *data, unsigned char number)
{	unsigned char i;
	rf12_trans(0x82C8);			// RX on
	rf12_trans(0xCA81);			// set FIFO mode
	rf12_trans(0xCA83);			// enable FIFO
	for (i=0; i<number; i++)
	{	rf12_ready();
		*data++=rf12_trans(0xB000);
	}
	rf12_trans(0x8208);			// RX off
}

void transmitPower(unsigned char id, unsigned char group, unsigned char value) {
// group A-D coded as 0..3
// value 0=off, 1=full, 2=1/2, 3=1/4, ... ==> n=1/2^(n-1)

	unsigned char settings[3];
	settings[0] = encodeFireAndID( 0, id );
	settings[1] = encodeGroupAndValue( group, value );
	rf12_txdata(settings, 3);
}

void transmitFire(unsigned char id) {
	unsigned char fire[3];
	fire[0] = encodeFireAndID( 1, id );
	fire[1] = 0x00;  // todo: understand, why necessary...
	fire[2] = 0x00;
	rf12_txdata(fire, 3);
}

void setRxMode(void) {

	// Interrupts for receiver: RFM12 new data received (FFST -> low)
	GICR |= (1<<INT1);		// Gerneral Interrupt Control Register: enable INT1
	GICR &= ~(1<<INT0);		// Gerneral Interrupt Control Register: disable INT0 (transmitter)

	// enable rfm12 receiver
	rf12_trans(0x82C8);			// RX on
	rf12_trans(0xCA81);			// set FIFO mode
	_delay_ms(.8);				// todo: is this really necessary???
	rf12_trans(0xCA83);			// enable FIFO: synch word search
}

void setTxMode(void) {

	// Interrupts for receiver: RFM12 new data received (FFST -> low)
	GICR &= ~(1<<INT1);		// Gerneral Interrupt Control Register: disable INT1 (FIFO data rdy)
	GICR |= (1<<INT0);		// Gerneral Interrupt Control Register: enable INT0 (transmitter)

	// disable rfm12 receiver
	rf12_trans(0x8208);			// RX off
}


void onReceive( unsigned char data ) {

	unsigned char fire, id, group, val;

	static unsigned char cycle = 0;

//	displayHex(data, cycle);			// for debugging!
	cycle = (cycle+1)%2;

	if( expectingSecondByte==0 ) {				// if this is the first byte of a transmission
		if( checkFireAndID(data) ) {			// and if it's valid
//			displayHex(data, 4);			// for debugging!
			fire = data;
			fire &= 0x08;						// extract fire-bit
			id = data;
			id &= 0x07;					// extract id
			if( (fire!=0) && (id==getID()) ) {	// fire command on own channel?
				if( getStrengthRx()>0 ) {		// strength > 0
					PORTD |= (1<<PD5);			// LED on!
					setTimer(0, 300);			// for 1/3s
					fireFlash();				// fire!
				}
				expectingSecondByte = 0;		// fire commands don't have a 2nd byte
				rf12_trans(0x8208);				// RX off
				rf12_trans(0x82C8);				// RX on
				rf12_trans(0xCA81);				// set FIFO mode
				_delay_ms(.8);					// todo: is this really necessary???
				rf12_trans(0xCA83);				// enable FIFO, wait for synch pattern
			} else if( (fire==0) && (id==getID()) ) { // first byte of 2-byte command
				buffer[0]=data;					// store for later use
				setTimer(4, 2);					// wait only 2ms for 2nd byte!
				expectingSecondByte=1;
			}
		} else { // ignore byte. todo: increase interference counter
//			PORTD |= (1<<PD5);			// LED on
//			setTimer(0, 100);			// for 1/10s
			rf12_trans(0x8208);				// RX off
			rf12_trans(0x82C8);				// RX on
			rf12_trans(0xCA81);				// set FIFO mode
			_delay_ms(.8);					// todo: is this really necessary???
			rf12_trans(0xCA83);				// enable FIFO, wait for synch pattern
		}
	}
	else if ( checkGroupAndValue(data) ) {  // second Byte has arrived -> check validity!

		rf12_trans(0x8208);						// RX off

		expectingSecondByte=0;
		buffer[1]=data;

//		displayHex(buffer[0], 1);				// for debugging!
//		displayHex(buffer[1], 2);					// for debugging!

		// unpack data
		id = buffer[0] & 0x07;					// extract id
		group = buffer[1];
		group = (group>>4) & 0x03;
		val = buffer[1] & 0x0f;

		if( waitingForACK ) { 					// device is Tx, gets ACK for sent data
			if ( (group == getCursor()) && (id == getID()) ) {
				setStrengthTx( val, group );
				waitingForACK = 0;
				setTxMode();
			}
		} else {								// device is Rx, data reception complete
			// fire = 0 and correct id is precondition for reception of 2nd byte!
			if ( (group == getGroup()) && (val<9) ) {		// power-change of own group?
				rf12_trans(0x8208);				// RX off
				PORTD |= (1<<PD5);   			// LED on!
				setTimer(0, 1000);
				_delay_ms(1);					// let Tx prepare to receive the ACK
				transmitPower(id, group, val);	// feedback to transmitter: got your data!
				setStrengthRx( val ); 			// strenghts: Off, Full, 1/2, 1/4, 1/8, 1/16, 1/32, 1/64
			}

			rf12_trans(0x82C8);			// RX on
			rf12_trans(0xCA81);			// set FIFO mode
			rf12_trans(0xCA83);			// enable FIFO, wait for synch pattern
		}
	} else { // received invalid 2nd byte --> ignore it. todo: increase interference counter
		rf12_trans(0x8208);				// RX off
		rf12_trans(0x82C8);				// RX on
		rf12_trans(0xCA81);				// set FIFO mode
		_delay_ms(.8);					// todo: is this really necessary???
		rf12_trans(0xCA83);				// enable FIFO, wait for synch pattern
	}
}

void fireFlash() {
	PORTD |= (1<<PD0);		// fire signal on
	for( int i = 0; i < timeTableSB24[getStrengthRx()]; i++ ) {
		_delay_us(10);		// wait x times 1/100.000s for flash to fire it's desired output power
	}
	PORTD &= ~(1<<PD0);   	// fire signal off
	PORTD |= (1<<PD1);   	// quench signal on
	setTimer(3, 50);		// timer for resetting the quench signal
}

unsigned char encodeFireAndID( unsigned char fire, unsigned char id ) {
	return ( 0x50 | (fire<<3) | id );
}

unsigned char encodeGroupAndValue( unsigned char group, unsigned char value ) {
	return ( 0x80 | (group<<4) | value );
}

unsigned char checkFireAndID( unsigned char data ) {
	return ( (data & 0x50) == 0x50 );
}

unsigned char checkGroupAndValue( unsigned char data ) {
	return ( (data & 0x80) == 0x80 );
}
