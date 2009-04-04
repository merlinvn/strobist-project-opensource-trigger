/*--------------------------------------------------------------------------------------------------

  Name         :  NokiaLCD.h

  Description  :  Header file for Nokia 84x48 graphic LCD driver.

  Author       :  2003-03-08 - Louis Frigon.

  History      :  2003-03-08 - First release.

--------------------------------------------------------------------------------------------------*/
#ifndef _NOKIALCD_H_

#define _NOKIALCD_H_

/*--------------------------------------------------------------------------------------------------
                                  General purpose constants
--------------------------------------------------------------------------------------------------*/
#define NULL                       0
#define FALSE                      0
#define TRUE                       1

#define LCD_X_RES                  84
#define LCD_Y_RES                  48

// Pinout for LCD acc. to layout (combination with RFM12)
#define LCD_RST_PIN                0x40  //  PD6 -
#define LCD_DC_PIN                 0x80  //  PD7 -

#define LCD_CE_PIN                 0x01  //  PB0 -
#define SPI_MOSI_PIN               0x08  //  PB3
#define SPI_CLK_PIN                0x20  //  PB5

#define LCD_BL						0x20	//PD5 backlight

/*--------------------------------------------------------------------------------------------------
                                       Type definitions
--------------------------------------------------------------------------------------------------*/
typedef char                       bool;
typedef unsigned char              byte;
typedef unsigned int               word;

typedef enum
{
    LCD_CMD  = 0,
    LCD_DATA = 1

} LcdCmdData;

typedef enum
{
    PIXEL_OFF =  0,
    PIXEL_ON  =  1,
    PIXEL_XOR =  2

} LcdPixelMode;

typedef enum
{
    FONT_1X = 1,
    FONT_2X = 2

} LcdFontSize;

typedef enum
{
    CURSOR_LEFT  = 0,
    CURSOR_RIGHT = 1,
    CURSOR_UP    = 2,
	CURSOR_DOWN  = 3,
	CURSOR_PRESS = 4
} CursorAction;


struct lcdSymbol {
	byte		val;
	byte		row;
	byte		line;
	byte		nbValues;
	byte		maxLen;
	byte*		caption;		// 0 if unused; e.g. "ID "
	byte**		line1Values;	// e.g. "Full", "1/2", "1/4"
	byte**		line2Values;	// 0 if unused
};


/*--------------------------------------------------------------------------------------------------
                                 Public function prototypes
--------------------------------------------------------------------------------------------------*/
extern void LcdInit       ( void );
extern void clr(void);
extern void onCursor( CursorAction );
extern byte getID();
extern unsigned char getGroup();
extern unsigned char getCursor();
extern unsigned char getStrengthRx();
extern void setStrengthRx(unsigned char str);
extern void setStrengthTx(unsigned char str, unsigned char group);
extern void eraseStrengthTx( unsigned char group );
extern void displayCurrentStrengthTx( unsigned char group );
extern void displayHex(unsigned char number, unsigned char xPos);


#endif  //  _NOKIALCD_H_
/*--------------------------------------------------------------------------------------------------
                                         End of file.
--------------------------------------------------------------------------------------------------*/

