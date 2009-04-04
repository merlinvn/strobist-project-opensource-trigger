/*--------------------------------------------------------------------------------------------------

  Name         :  NokiaLCD.c

  Description  :  This is a driver for the Nokia 84x48 graphic LCD.

  Author       :  2003-03-08 - Sylvain Bissonnette

  History      :  2003-02-08 - First release (v0.1) derived from Sylvain Bissonnette code base.
                  2003-03-09 - v0.2, Louis Frigon: 2x fonts support.
                  2003-03-20 - v0.3: Serialization optimized,
				  2008-09-03 - v0.4: T. Seyfarth, kept only init, clr and write functions and the font definition,
									  introduced LCDSymbols, related code and event handlers etc.

--------------------------------------------------------------------------------------------------*/

#include <avr/io.h>
#include <util/delay.h>

#include "lcd3110.h"
#include "rfm12.h"
#include "timer.h"

#define LCD_FIRMWARE_VERSION       0.4


/*--------------------------------------------------------------------------------------------------
                                Private function prototypes
--------------------------------------------------------------------------------------------------*/
//  Function prototypes are mandatory otherwise the compiler generates unreliable code.

static void Delay ( void );
static void gotoXY(byte, byte);
static void writeData(byte);
static void writeCmd(byte);
static void changeVal( byte, byte );
static void paintSymbols();

/*--------------------------------------------------------------------------------------------------
                                      Private Variables
--------------------------------------------------------------------------------------------------*/

byte hex0[] = 	{ 0x3E, 0x51, 0x49, 0x45, 0x3E};
byte hex1[] = 	{ 0x00, 0x42, 0x7F, 0x40, 0x00};
byte hex2[] = 	{ 0x42, 0x61, 0x51, 0x49, 0x46};
byte hex3[] = 	{ 0x21, 0x41, 0x45, 0x4B, 0x31};
byte hex4[] = 	{ 0x18, 0x14, 0x12, 0x7F, 0x10};
byte hex5[] = 	{ 0x27, 0x45, 0x45, 0x45, 0x39};
byte hex6[] = 	{ 0x3C, 0x4A, 0x49, 0x49, 0x30};
byte hex7[] = 	{ 0x01, 0x71, 0x09, 0x05, 0x03};
byte hex8[] = 	{ 0x36, 0x49, 0x49, 0x49, 0x36};
byte hex9[] = 	{ 0x06, 0x49, 0x49, 0x29, 0x1E};
byte hexA[] = 	{ 0x7E, 0x11, 0x11, 0x11, 0x7E};
byte hexB[] = 	{ 0x7F, 0x49, 0x49, 0x49, 0x36};
byte hexC[] = 	{ 0x3E, 0x41, 0x41, 0x41, 0x22};
byte hexD[] = 	{ 0x7F, 0x41, 0x41, 0x22, 0x1C};
byte hexE[] = 	{ 0x7F, 0x49, 0x49, 0x49, 0x41};
byte hexF[] = 	{ 0x7F, 0x09, 0x09, 0x09, 0x01};

byte* hexCode[] = { hex0, hex1, hex2, hex3, hex4, hex5, hex6, hex7, hex8,
					hex9, hexA, hexB, hexC, hexD, hexE, hexF };


byte codeTx[] = { 0x01, 0x01, 0x7F, 0x01, 0x01, 0x00, 0x44, 0x28, 0x10, 0x28, 0x44, 0xff };
byte codeRx[] = { 0x7F, 0x09, 0x19, 0x29, 0x46, 0x44, 0x28, 0x10, 0x28, 0x44, 0xff };
byte codeA[] = { 0x7E, 0x11, 0x11, 0x11, 0x7E, 0x00, 0x00, 0x00, 0x00, 0xff };
byte codeB[] = { 0x7F, 0x49, 0x49, 0x49, 0x36, 0x00, 0x00, 0x00, 0x00, 0xff };
byte codeC[] = { 0x3E, 0x41, 0x41, 0x41, 0x22, 0x00, 0x00, 0x00, 0x00, 0xff };
byte codeD[] = { 0x7F, 0x41, 0x41, 0x22, 0x1C, 0x00, 0x00, 0x00, 0x00, 0xff };

static byte codeOff[] =	{
						0x3E, 0x41, 0x41, 0x41, 0x3E, 0x00,
						0x08, 0x7E, 0x09, 0x01, 0x02, 0x00,
						0x08, 0x7E, 0x09, 0x01, 0x02, 0x00,
						0x00, 0x00, 0x00, 0x00, 0x00, 0xff
					};
static byte codeFull[] =	{
						0x7F, 0x09, 0x09, 0x09, 0x01, 0x00,
						0x3C, 0x40, 0x40, 0x20, 0x7C, 0x00,
						0x00, 0x41, 0x7F, 0x40, 0x00, 0x00,
						0x00, 0x41, 0x7F, 0x40, 0x00, 0xff
					};
static byte code2[] =	{
						0x00, 0x42, 0x7F, 0x40, 0x00, 0x00,
						0x20, 0x10, 0x08, 0x04, 0x02, 0x00,
						0x42, 0x61, 0x51, 0x49, 0x46, 0x00,
						0x00, 0x00, 0x00, 0x00, 0x00, 0xff
					};
static byte code4[] =	{
						0x00, 0x42, 0x7F, 0x40, 0x00, 0x00,
						0x20, 0x10, 0x08, 0x04, 0x02, 0x00,
						0x18, 0x14, 0x12, 0x7F, 0x10, 0x00,
						0x00, 0x00, 0x00, 0x00, 0x00, 0xff
					};
static byte code8[] =	{
						0x00, 0x42, 0x7F, 0x40, 0x00, 0x00,
						0x20, 0x10, 0x08, 0x04, 0x02, 0x00,
						0x36, 0x49, 0x49, 0x49, 0x36, 0x00,
						0x00, 0x00, 0x00, 0x00, 0x00, 0xff
					};
static byte code16[] =	{
						0x00, 0x42, 0x7F, 0x40, 0x00, 0x00,
						0x20, 0x10, 0x08, 0x04, 0x02, 0x00,
						0x00, 0x42, 0x7F, 0x40, 0x00, 0x00,
						0x3C, 0x4A, 0x49, 0x49, 0x30, 0xff
					};
static byte code32[] =	{
						0x00, 0x42, 0x7F, 0x40, 0x00, 0x00,
						0x20, 0x10, 0x08, 0x04, 0x02, 0x00,
						0x21, 0x41, 0x45, 0x4B, 0x31, 0x00,
						0x42, 0x61, 0x51, 0x49, 0x46, 0xff
					};
static byte code64[] =	{
						0x00, 0x42, 0x7F, 0x40, 0x00, 0x00,
						0x20, 0x10, 0x08, 0x04, 0x02, 0x00,
						0x3C, 0x4A, 0x49, 0x49, 0x30, 0x00,
						0x18, 0x14, 0x12, 0x7F, 0x10, 0xff
					};
static byte lampOn1[] = { 0x08, 0x10, 0xc0, 0x20, 0xa0, 0x2c, 0xa0, 0x20, 0xc0, 0x10, 0x08, 0xff };
static byte lampOff1[]= { 0x00, 0x00, 0xc0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xc0, 0x00, 0x00, 0xff };
static byte lampOn2[] = { 0x11, 0x08, 0x03, 0x05, 0x3c, 0x29, 0x3c, 0x05, 0x03, 0x08, 0x11, 0xff };
static byte lampOff2[]= { 0x00, 0x00, 0x03, 0x07, 0x3f, 0x2b, 0x3f, 0x07, 0x03, 0x00, 0x00, 0xff };

static byte groupCap[] = {
						0x3E, 0x41, 0x49, 0x49, 0x7A, 0x00,
						0x7C, 0x08, 0x04, 0x04, 0x08, 0x00,
						0x38, 0x44, 0x44, 0x44, 0x38, 0x00,
						0x3C, 0x40, 0x40, 0x20, 0x7C, 0x00,
						0x7C, 0x14, 0x14, 0x14, 0x08, 0x00,
						0x00, 0x00, 0x00, 0x00, 0xff
					};

static byte pwrCap[] = { 		0x7F, 0x09, 0x09, 0x09, 0x06, 0x00,
								0x3C, 0x40, 0x30, 0x40, 0x3C, 0x00,
								0x7C, 0x08, 0x04, 0x04, 0x08, 0x00, 0x00, 0x00, 0xff
 					};

static byte groupA[] = { 0x7E, 0x11, 0x11, 0x11, 0x7E, 0xff };
static byte groupB[] = { 0x7F, 0x49, 0x49, 0x49, 0x36, 0xff };
static byte groupC[] = { 0x3E, 0x41, 0x41, 0x41, 0x22, 0xff };
static byte groupD[] = { 0x7F, 0x41, 0x41, 0x22, 0x1C, 0xff };

static byte codeID[] = { 0x00, 0x41, 0x7F, 0x41, 0x00, 0x00, 0x7F, 0x41, 0x41, 0x22, 0x1C, 0x00, 0x00, 0x00, 0x00, 0xff };
static byte id0[] = { 0x3E, 0x51, 0x49, 0x45, 0x3E, 0xff };
static byte id1[] = { 0x00, 0x42, 0x7F, 0x40, 0x00, 0xff };
static byte id2[] = { 0x42, 0x61, 0x51, 0x49, 0x46, 0xff };
static byte id3[] = { 0x21, 0x41, 0x45, 0x4B, 0x31, 0xff };
static byte id4[] = { 0x18, 0x14, 0x12, 0x7F, 0x10, 0xff };
static byte id5[] = { 0x27, 0x45, 0x45, 0x45, 0x39, 0xff };
static byte id6[] = { 0x3C, 0x4A, 0x49, 0x49, 0x30, 0xff };
static byte id7[] = { 0x01, 0x71, 0x09, 0x05, 0x03, 0xff };

static byte sb24[] = {
						0x46, 0x49, 0x49, 0x49, 0x31, 0x00,
						0x7F, 0x49, 0x49, 0x49, 0x36, 0x00,
						0x10, 0x10, 0x10, 0x10, 0x10, 0x00,
						0x42, 0x61, 0x51, 0x49, 0x46, 0x00,
						0x18, 0x14, 0x12, 0x7F, 0x10, 0xff
				};

static byte sb26[] = {
						0x46, 0x49, 0x49, 0x49, 0x31, 0x00,
						0x7F, 0x49, 0x49, 0x49, 0x36, 0x00,
						0x10, 0x10, 0x10, 0x10, 0x10, 0x00,
						0x42, 0x61, 0x51, 0x49, 0x46, 0x00,
						0x3C, 0x4A, 0x49, 0x49, 0x30, 0xff
				};

static byte* powerValues[] = { codeOff, codeFull, code2, code4, code8, code16, code32, code64 };
static byte* blValues1[]   = { lampOff1, lampOn1 };
static byte* blValues2[]   = { lampOff2, lampOn2 };
static byte* modeValues[]  = { codeTx, codeRx };
static byte* groupValues[] = { groupA, groupB, groupC, groupD };
static byte* idValues[] = { id0, id1, id2, id3, id4, id5, id6, id7 };
static byte* deviceValues[] = { sb24, sb26};

struct lcdSymbol powerASym = { 0, 1, 1, 8, 4, codeA, powerValues, 0 };
struct lcdSymbol powerBSym = { 0, 1, 2, 8, 4, codeB, powerValues, 0 };
struct lcdSymbol powerCSym = { 0, 1, 3, 8, 4, codeC, powerValues, 0 };
struct lcdSymbol powerDSym = { 0, 1, 4, 8, 4, codeD, powerValues, 0 };
struct lcdSymbol blSym     = { 0, 11, 1, 2, 2,  0, blValues1, blValues2 };
struct lcdSymbol idSym     = { 0, 10, 3, 8, 2,  codeID, idValues, 0 };
struct lcdSymbol modeSym   = { 0, 10, 4, 2, 2, 0, modeValues, 0 };
struct lcdSymbol groupSym  = { 0, 1, 1, 4, 1, groupCap, groupValues, 0 };
struct lcdSymbol deviceSym = { 0, 1, 2, 2, 5, 0, deviceValues, 0 };
struct lcdSymbol powerRxSym= { 0, 1, 4, 8, 4, pwrCap, powerValues, 0 };

/*--------------------------------------------------------------------------------------------------
                                      Global Variables
--------------------------------------------------------------------------------------------------*/

struct lcdSymbol* txSymbols[] = {&powerASym, &powerBSym, &powerCSym, &powerDSym, &blSym, &idSym, &modeSym};
struct lcdSymbol* rxSymbols[] = {&groupSym, &deviceSym, &powerRxSym, &blSym, &idSym, &modeSym };

struct lcdSymbol** lcdSymbols = txSymbols;
static byte nbSymbols = 7;

static byte cursorPos = 0;
static byte prevCursor = 0;
static byte currVal = 0;
static byte prevVal = 0;


/*--------------------------------------------------------------------------------------------------

  Name         :  LcdInit

  Description  :  Performs MCU SPI & LCD controller initialization.

  Argument(s)  :  None.

  Return value :  None.

--------------------------------------------------------------------------------------------------*/
void LcdInit ( void )
{
    Delay();

    //  Toggle display reset pin.
    PORTD &= ~LCD_RST_PIN;
    Delay();
    PORTD |= LCD_RST_PIN;



    writeCmd( 0x21 );  // LCD Extended Commands.
    writeCmd( 0xC8 );  // Set LCD Vop (Contrast).
    writeCmd( 0x06 );  // Set Temp coefficent.
    writeCmd( 0x13 );  // LCD bias mode 1:48.
    writeCmd( 0x20 );  // LCD Standard Commands, Horizontal addressing mode.
    writeCmd( 0x0C );  // LCD in normal mode.

	clr();
	paintSymbols();
}

/*--------------------------------------------------------------------------------------------------
  Name         :  LcdContrast
  Description  :  Set display contrast.
  Argument(s)  :  contrast -> Contrast value from 0x00 to 0x7F.
  Return value :  None.
  Notes        :  No change visible at ambient temperature.
--------------------------------------------------------------------------------------------------*/
void LcdContrast ( byte contrast )
{
    //  LCD Extended Commands.
    writeCmd( 0x21 );

    // Set LCD Vop (Contrast).
    writeCmd( 0x80 | contrast );

    //  LCD Standard Commands, horizontal addressing mode.
    writeCmd( 0x20 );
}



static void Delay ( void ) {
    for ( int i = -32000; i < 32000; i++ );
}

void clr() {
    for ( int i = 0; i < (LCD_X_RES * LCD_Y_RES); i++ )
		writeData(0x00);
}

static void writeData( byte c ) {
    PORTB &= ~LCD_CE_PIN;				// Chip select
    PORTD |= LCD_DC_PIN;				// Data/Cmd Pin = data
	SPDR = c;							// Send data to display controller
	while ( (SPSR & 0x80) != 0x80 );	// Wait until Tx register empty
	PORTB |= LCD_CE_PIN;
}

static void writeCmd( byte c ) {
    PORTB &= ~LCD_CE_PIN;				// Chip select
 	PORTD &= ~LCD_DC_PIN;				// Data/Cmd Pin = cmd
	SPDR = c;							// Send data to display controller
	while ( (SPSR & 0x80) != 0x80 );	// Wait until Tx register empty
	PORTB |= LCD_CE_PIN;
}

static void gotoXY ( byte x, byte y ) {
    writeCmd( 0x80 | x*6 );
    writeCmd( 0x40 | y );
}

static void paintSymbols() {
	byte currCodeID = 0;
	byte currCode = 0;
	byte symID = 0;
	struct lcdSymbol* currSymbol;
	for (symID = 0; symID < nbSymbols; symID++) {
		currSymbol = lcdSymbols[symID];

		// delete previous cursor
		if (prevCursor == symID) {
			gotoXY(currSymbol->row-1, currSymbol->line);
			writeData(0x00);
			writeData(0x00);
			writeData(0x00);
			writeData(0x00);
		}

		// show current cursor and clear previous value
		if (cursorPos == symID) {
			gotoXY(currSymbol->row-1, currSymbol->line);
			writeData(0x7f);
			writeData(0x3e);
			writeData(0x1c);
			writeData(0x08);
			if (currSymbol->caption != 0) { // skip caption
				for( currCodeID = 0; currSymbol->caption[currCodeID] != 0xff; currCodeID++);
				writeCmd( 0x80 | (currSymbol->row * 6 + currCodeID - 1 ));
			}
			for (int i = 0; i < currSymbol->maxLen*6; i++)
				writeData(0x00);
		}

		gotoXY(currSymbol->row, currSymbol->line);

		// caption
		if( currSymbol->caption != 0 ) {
			currCodeID = 0;
			do {
				currCode = currSymbol->caption[currCodeID++];
				if (currCode != 0xff) {
					writeData( currCode );//<< 1 );
				}
			} while (currCode!=0xff);
		}

		// 1st line of current value
		currCodeID = 0;
		do {
			currCode = currSymbol->line1Values[currSymbol->val][currCodeID++];
			if (currCode != 0xff)
				writeData( currCode );//<< 1);
		} while (currCode!=0xff);

		// 2nd line of current value
		if( currSymbol->line2Values != 0 ) {
			gotoXY(currSymbol->row, currSymbol->line+1);
			currCodeID = 0;
			do {
				currCode = currSymbol->line2Values[currSymbol->val][currCodeID++];
				if (currCode != 0xff)
					writeData( currCode );//<< 1);
			} while (currCode!=0xff);
		}

	}
}

static void changeVal( byte prevVal, byte currVal) {

	if (lcdSymbols == txSymbols) { // Tx mode
		switch (cursorPos) {
			case 0: // power A
			case 1: // power B
			case 2: // power C
			case 3: // power D
				break;

			case 4: // backlight
				if (currVal == 1) {
					PORTD |= LCD_BL;  // Pin 4 high
					transmitFire(idSym.val);
				}
				else {
					PORTD &= ~LCD_BL;  // Pin 4 low
				}
				break;

			case 5: // change id

				break;

			case 6: // mode -> Rx
				setRxMode();
				lcdSymbols = rxSymbols;
				nbSymbols = 6;
				cursorPos = 5;
				clr();
				break;
		}
	} else { // Rx mode
		switch (cursorPos) {
			case 0: // group

				break;

			case 1: // device

				break;

			case 2: // power

				break;

			case 3: // toggle backlight
				if (currVal == 1) {
					PORTD |= LCD_BL;  // Pin 4 high
				}
				else {
					PORTD &= ~LCD_BL;  // Pin 4 low
				}
				break;

			case 4: // change id

				break;

			case 5: // mode -> Tx
				setTxMode();
				lcdSymbols = txSymbols;
				nbSymbols = 7;
				cursorPos = 6;
				clr();
				break;
		}
	}

}

void onCursor( CursorAction action ) {

#ifdef BACKLIGHT_INSTALLED
	PORTD |= LCD_BL;
	setTimer(0, 4000);	// LED on for 4s on cursor activity
#endif

	// if Tx strength change: transmit change and wait for acknowledge, don't changes values
	if( (cursorPos>=0) && (cursorPos<=3) && (lcdSymbols==txSymbols) &&
			((action==CURSOR_LEFT) || (action==CURSOR_RIGHT)) ) {

		if( action == CURSOR_LEFT) {
			prevVal = lcdSymbols[cursorPos]->val;
			currVal = (prevVal + (lcdSymbols[cursorPos]->nbValues) -1) % (lcdSymbols[cursorPos]->nbValues);
		} else {  // CURSOR_RIGHT
			prevVal = lcdSymbols[cursorPos]->val;
			currVal = (prevVal+1) % (lcdSymbols[cursorPos]->nbValues);
		}
		transmitPower(idSym.val, cursorPos, currVal);
		waitingForACK = 1;
		setTimer(2, 20);		// wait 20ms for ACK
		setRxMode();
	} else {
		switch( action ) {  // menu change
			case CURSOR_DOWN:
				prevCursor = cursorPos;
				cursorPos = (cursorPos +1) % nbSymbols;
				break;

			case CURSOR_UP:
				prevCursor = cursorPos;
				cursorPos = (cursorPos + nbSymbols -1) % nbSymbols;
				break;

			case CURSOR_LEFT:
				prevVal = lcdSymbols[cursorPos]->val;
				currVal = (prevVal + (lcdSymbols[cursorPos]->nbValues) -1) % (lcdSymbols[cursorPos]->nbValues);
				lcdSymbols[cursorPos]->val = currVal;
				changeVal(prevVal, currVal);
				break;

			case CURSOR_RIGHT:
				prevVal = lcdSymbols[cursorPos]->val;
				currVal = (prevVal+1) % (lcdSymbols[cursorPos]->nbValues);
				lcdSymbols[cursorPos]->val = currVal;
				changeVal( prevVal, currVal);
				break;

			case CURSOR_PRESS:
				if( lcdSymbols == txSymbols ) { // if transmitter: send fire
					transmitFire(getID());
					PORTD |= LCD_BL;
					setTimer(0, 300);
				} else { // if receiver: fire
					PORTD |= (1<<PD5);   // LED on!
					setTimer(0, 300);
					fireFlash();
				}

				break;
		}
		paintSymbols();
	}
}

byte getID() {
	return idSym.val;
}

unsigned char getGroup() {
	return groupSym.val;
}

unsigned char getCursor() {
	return cursorPos;
}

unsigned char getStrengthRx() {
	return powerRxSym.val;
}

void setStrengthRx(unsigned char str) {
	powerRxSym.val = str;
	paintSymbols();
}

void setStrengthTx( unsigned char str, unsigned char group) {
	txSymbols[group]->val = str;
	paintSymbols();
}

void eraseStrengthTx( unsigned char group ) {
	gotoXY(2, group+1);
	for(int i=0; i<26; i++)
		writeData(0x00);
}

void displayCurrentStrengthTx( unsigned char group ) {
	gotoXY(2, group+1);
	for( int i=0; i<3; i++ )
		writeData( 0 );
	unsigned char currCodeID = 0;
	unsigned char currCode = 0;
	do {
		currCode = powerValues[txSymbols[group]->val][currCodeID++];
		if (currCode != 0xff)
			writeData( currCode );
	} while (currCode!=0xff);
}

void displayHex(unsigned char number, unsigned char xPos) {
	xPos*=2;
	gotoXY(xPos, 0);
	for(int i=0; i<11; i++)
		writeData(0x00);
	gotoXY(xPos, 0);
	unsigned char hex1 = number;
	hex1 = (hex1 >> 4) & 0x0f;
	for(int i=0; i<5; i++)
		writeData( hexCode[hex1][i] );
	writeData(0x00);
	unsigned char hex2 = number;
	hex2 &=  0x0f;
	for(int i=0; i<5; i++)
		writeData( hexCode[hex2][i] );

}

/*--------------------------------------------------------------------------------------------------
                                     Character generator

             This table defines the standard ASCII characters in a 5x7 dot format.
--------------------------------------------------------------------------------------------------*/
/*
static const byte FontLookup [][5] =
{
    { 0x00, 0x00, 0x00, 0x00, 0x00 },  // sp
    { 0x00, 0x00, 0x2f, 0x00, 0x00 },   // !
    { 0x00, 0x07, 0x00, 0x07, 0x00 },   // "
    { 0x14, 0x7f, 0x14, 0x7f, 0x14 },   // #
    { 0x24, 0x2a, 0x7f, 0x2a, 0x12 },   // $
    { 0xc4, 0xc8, 0x10, 0x26, 0x46 },   // %
    { 0x36, 0x49, 0x55, 0x22, 0x50 },   // &
    { 0x00, 0x05, 0x03, 0x00, 0x00 },   // '
    { 0x00, 0x1c, 0x22, 0x41, 0x00 },   // (
    { 0x00, 0x41, 0x22, 0x1c, 0x00 },   // )
    { 0x14, 0x08, 0x3E, 0x08, 0x14 },   // *
    { 0x08, 0x08, 0x3E, 0x08, 0x08 },   // +
    { 0x00, 0x00, 0x50, 0x30, 0x00 },   // ,
    { 0x10, 0x10, 0x10, 0x10, 0x10 },   // -
    { 0x00, 0x60, 0x60, 0x00, 0x00 },   // .
    { 0x20, 0x10, 0x08, 0x04, 0x02 },   // /
    { 0x3E, 0x51, 0x49, 0x45, 0x3E },   // 0
    { 0x00, 0x42, 0x7F, 0x40, 0x00 },   // 1
    { 0x42, 0x61, 0x51, 0x49, 0x46 },   // 2
    { 0x21, 0x41, 0x45, 0x4B, 0x31 },   // 3
    { 0x18, 0x14, 0x12, 0x7F, 0x10 },   // 4
    { 0x27, 0x45, 0x45, 0x45, 0x39 },   // 5
    { 0x3C, 0x4A, 0x49, 0x49, 0x30 },   // 6
    { 0x01, 0x71, 0x09, 0x05, 0x03 },   // 7
    { 0x36, 0x49, 0x49, 0x49, 0x36 },   // 8
    { 0x06, 0x49, 0x49, 0x29, 0x1E },   // 9
    { 0x00, 0x36, 0x36, 0x00, 0x00 },   // :
    { 0x00, 0x56, 0x36, 0x00, 0x00 },   // ;
    { 0x08, 0x14, 0x22, 0x41, 0x00 },   // <
    { 0x14, 0x14, 0x14, 0x14, 0x14 },   // =
    { 0x00, 0x41, 0x22, 0x14, 0x08 },   // >
    { 0x02, 0x01, 0x51, 0x09, 0x06 },   // ?
    { 0x32, 0x49, 0x59, 0x51, 0x3E },   // @
    { 0x7E, 0x11, 0x11, 0x11, 0x7E },   // A
    { 0x7F, 0x49, 0x49, 0x49, 0x36 },   // B
    { 0x3E, 0x41, 0x41, 0x41, 0x22 },   // C
    { 0x7F, 0x41, 0x41, 0x22, 0x1C },   // D
    { 0x7F, 0x49, 0x49, 0x49, 0x41 },   // E
    { 0x7F, 0x09, 0x09, 0x09, 0x01 },   // F
    { 0x3E, 0x41, 0x49, 0x49, 0x7A },   // G
    { 0x7F, 0x08, 0x08, 0x08, 0x7F },   // H
    { 0x00, 0x41, 0x7F, 0x41, 0x00 },   // I
    { 0x20, 0x40, 0x41, 0x3F, 0x01 },   // J
    { 0x7F, 0x08, 0x14, 0x22, 0x41 },   // K
    { 0x7F, 0x40, 0x40, 0x40, 0x40 },   // L
    { 0x7F, 0x02, 0x0C, 0x02, 0x7F },   // M
    { 0x7F, 0x04, 0x08, 0x10, 0x7F },   // N
    { 0x3E, 0x41, 0x41, 0x41, 0x3E },   // O
    { 0x7F, 0x09, 0x09, 0x09, 0x06 },   // P
    { 0x3E, 0x41, 0x51, 0x21, 0x5E },   // Q
    { 0x7F, 0x09, 0x19, 0x29, 0x46 },   // R
    { 0x46, 0x49, 0x49, 0x49, 0x31 },   // S
    { 0x01, 0x01, 0x7F, 0x01, 0x01 },   // T
    { 0x3F, 0x40, 0x40, 0x40, 0x3F },   // U
    { 0x1F, 0x20, 0x40, 0x20, 0x1F },   // V
    { 0x3F, 0x40, 0x38, 0x40, 0x3F },   // W
    { 0x63, 0x14, 0x08, 0x14, 0x63 },   // X
    { 0x07, 0x08, 0x70, 0x08, 0x07 },   // Y
    { 0x61, 0x51, 0x49, 0x45, 0x43 },   // Z
    { 0x00, 0x7F, 0x41, 0x41, 0x00 },   // [
    { 0x55, 0x2A, 0x55, 0x2A, 0x55 },   // 55
    { 0x00, 0x41, 0x41, 0x7F, 0x00 },   // ]
    { 0x04, 0x02, 0x01, 0x02, 0x04 },   // ^
    { 0x40, 0x40, 0x40, 0x40, 0x40 },   // _
    { 0x00, 0x01, 0x02, 0x04, 0x00 },   // '
    { 0x20, 0x54, 0x54, 0x54, 0x78 },   // a
    { 0x7F, 0x48, 0x44, 0x44, 0x38 },   // b
    { 0x38, 0x44, 0x44, 0x44, 0x20 },   // c
    { 0x38, 0x44, 0x44, 0x48, 0x7F },   // d
    { 0x38, 0x54, 0x54, 0x54, 0x18 },   // e
    { 0x08, 0x7E, 0x09, 0x01, 0x02 },   // f
    { 0x0C, 0x52, 0x52, 0x52, 0x3E },   // g
    { 0x7F, 0x08, 0x04, 0x04, 0x78 },   // h
    { 0x00, 0x44, 0x7D, 0x40, 0x00 },   // i
    { 0x20, 0x40, 0x44, 0x3D, 0x00 },   // j
    { 0x7F, 0x10, 0x28, 0x44, 0x00 },   // k
    { 0x00, 0x41, 0x7F, 0x40, 0x00 },   // l
    { 0x7C, 0x04, 0x18, 0x04, 0x78 },   // m
    { 0x7C, 0x08, 0x04, 0x04, 0x78 },   // n
    { 0x38, 0x44, 0x44, 0x44, 0x38 },   // o
    { 0x7C, 0x14, 0x14, 0x14, 0x08 },   // p
    { 0x08, 0x14, 0x14, 0x18, 0x7C },   // q
    { 0x7C, 0x08, 0x04, 0x04, 0x08 },   // r
    { 0x48, 0x54, 0x54, 0x54, 0x20 },   // s
    { 0x04, 0x3F, 0x44, 0x40, 0x20 },   // t
    { 0x3C, 0x40, 0x40, 0x20, 0x7C },   // u
    { 0x1C, 0x20, 0x40, 0x20, 0x1C },   // v
    { 0x3C, 0x40, 0x30, 0x40, 0x3C },   // w
    { 0x44, 0x28, 0x10, 0x28, 0x44 },   // x
    { 0x0C, 0x50, 0x50, 0x50, 0x3C },   // y
    { 0x44, 0x64, 0x54, 0x4C, 0x44 }    // z
};
*/


/*--------------------------------------------------------------------------------------------------
                                         End of file.
--------------------------------------------------------------------------------------------------*/



