
#ifndef RFM12_H_
#define RFM12_H_


extern unsigned short rf12_trans(unsigned short wert);					// transfer 1 word to/from module
extern void rf12_init(void);											// initialize module
extern void rf12_setfreq(unsigned short freq);							// set center frequency
extern void rf12_setbaud(unsigned short baud);							// set baudrate
extern void rf12_setpower(unsigned char power, unsigned char mod);		// set transmission settings
extern void rf12_setbandwidth(unsigned char bandwidth, unsigned char gain, unsigned char drssi);	// set receiver settings
extern void rf12_txdata(unsigned char *data, unsigned char number);		// transmit number of bytes from array
extern void rf12_rxdata(unsigned char *data, unsigned char number);		// receive number of bytes into array
extern void rf12_ready(void);											// wait until FIFO ready (to transmit/read data)

extern void transmitPower(unsigned char id, unsigned char group, unsigned char value);
extern void transmitFire(unsigned char id);
extern void setRxMode(void);
extern void setTxMode(void);
extern void onReceive(unsigned char data);
extern void fireFlash();

extern unsigned char waitingForACK;
extern unsigned char expectingSecondByte;

#define RF12FREQ(freq)	((freq-430.0)/0.0025)							// macro for calculating frequency value out of frequency in MHz

#define RFM12_CS_PIN		0x04  //  PB2

#endif /* RFM12_H_ */
