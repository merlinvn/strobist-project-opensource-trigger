# Interrupt Driven Control Flow #

<a href='http://www.flickr.com/photos/9601007@N03/3588834792/' title='Interrupt-driven control flow von Till_Hamburg bei Flickr'><img src='http://farm4.static.flickr.com/3333/3588834792_d43ba85e5c_o.jpg' alt='Interrupt-driven control flow' width='614' height='293' /></a>

## Excerpts from spot.c ##
### Interrupts ###
```
// shutter interrupt
ISR(INT0_vect) {
        transmitFire( getID() );
        PORTD |= LCD_BL;
        setTimer(0, 300);
}

// FIFO ready interrupt 
ISR(INT1_vect) {
        onReceive( rf12_trans(0xB000) );
}

// timer interrupt: counted to 256 (at 1/8 clock)
ISR(TIMER0_OVF_vect) {
        counter++;
}
```
### Main loop with timer handling ###
```

int main (void) {

	init();

	while(1) {
 
		if (counter >= 4) {	// 3906,25Hz interrupt frequency => divided by 1000 for ms-ticks => 2.4% too slow
			counter = 0;
			processTimers();
		}

		if( isExpired(0) ) {	// LED off
			PORTD &= ~LCD_BL;
		}

		if( isExpired(1) ) {	// poll buttons
			if( !(PINC & (1<<PINC1)) ) {    // test
				onCursor( CURSOR_PRESS );
			}
			if( !(PINC & (1<<PINC2)) ) {    // down
				onCursor( CURSOR_DOWN );
			}

			// ... same for UP, LEFT, RIGHT

			setTimer(1, 200);
		}
 
		// ... same for other timers
	}
}

```
# User Interface #

The SPOT is controlled by a 4-way joystick button. If pressed down in Tx mode it transmits a fire signal. In Rx mode it fires the connected flash. The four directions are used for menu navigation as indicated below.

<a href='http://www.flickr.com/photos/9601007@N03/3588026887/' title='User interface navigation von Till_Hamburg bei Flickr'><img src='http://farm4.static.flickr.com/3417/3588026887_38f333ec70_o.jpg' alt='User interface navigation' width='702' height='446' /></a>

## Excerpts from lcd3110.c ##
### Joystick event handler ###
```

void onCursor( CursorAction action ) {

// ...
 
	// if Tx strength change: transmit change and wait for acknowledge, don't changes values
	if( (cursorPos>=0) && (cursorPos<=3) && (lcdSymbols==txSymbols) && ((action==CURSOR_LEFT) || (action==CURSOR_RIGHT)) ) {
		if( action == CURSOR_LEFT) {
			prevVal = lcdSymbols[cursorPos]->val;
			currVal = (prevVal + (lcdSymbols[cursorPos]->nbValues) -1) % (lcdSymbols[cursorPos]->nbValues);
		} else {  // CURSOR_RIGHT
			prevVal = lcdSymbols[cursorPos]->val;
			currVal = (prevVal+1) % (lcdSymbols[cursorPos]->nbValues);
		}
		transmitPower(idSym.val, cursorPos, currVal);
		waitingForACK = 1;
		setTimer(2, 20); // wait 20ms for ACK
		setRxMode();
	} else {
		switch( action ) {  // menu change
			case CURSOR_DOWN:
				prevCursor = cursorPos;
				cursorPos = (cursorPos +1) % nbSymbols;
				break;
 
			case CURSOR_LEFT:
				prevVal = lcdSymbols[cursorPos]->val;
				currVal = (prevVal + (lcdSymbols[cursorPos]->nbValues) -1) % (lcdSymbols[cursorPos]->nbValues);
				lcdSymbols[cursorPos]->val = currVal;
				changeVal(prevVal, currVal);
				break;

			// ... similar for UP and RIGHT

			case CURSOR_PRESS:
				if( lcdSymbols == txSymbols ) {	// if transmitter: send fire
					transmitFire(getID());
					PORTD |= LCD_BL;
					setTimer(0, 300);
				} else {			// if receiver: fire
					PORTD |= (1<<PD5);	// LED on for 300ms
					setTimer(0, 300);
					fireFlash();
				}
 
				break;
		}
		paintSymbols(); // refresh the screen
	}
}

```

### Type definition: LCD Symbol ###

```
struct lcdSymbol {
	byte	val;
	byte	row;
	byte	line;
	byte	nbValues;
	byte	maxLen;
	byte*	caption;	// 0 if unused; e.g. "ID "
	byte**	line1Values;	// e.g. "Full", "1/2", "1/4"
	byte**	line2Values;	// 0 if unused
};

```

### Painting the screen ###

```

static void paintSymbols() {

	byte currCodeID = 0;
	byte currCode = 0;
	byte symID = 0;
	struct lcdSymbol* currSymbol;

	for (symID = 0; symID < nbSymbols; symID++) { // loop: paint all symbols

		currSymbol = lcdSymbols[symID];
 
		// delete previous cursor
			// ... 
		// show current cursor and clear previous value
			// ... 

		gotoXY(currSymbol->row, currSymbol->line);

		// paint caption first (if there is one)
		if( currSymbol->caption != 0 ) {
			currCodeID = 0;
			do {
				currCode = currSymbol->caption[currCodeID++];
				if (currCode != 0xff) {
					writeData( currCode );
				}
			} while (currCode!=0xff);
		}

		// paint 1st line of current value
		currCodeID = 0;
		do {
			currCode = currSymbol->line1Values[currSymbol->val][currCodeID++];
			if (currCode != 0xff)
				writeData( currCode );//<< 1);
		} while (currCode!=0xff);
 
		// paint 2nd line of current value (if there is one)
			// ... 
	}
}

```

# Radio Protocol #
## Packet definition ##
One packet consists of 2 bytes as the following diagram shows:

<a href='http://www.flickr.com/photos/9601007@N03/3588026923/' title='Radio packet definition von Till_Hamburg bei Flickr'><img src='http://farm4.static.flickr.com/3196/3588026923_3c19ce32cf_o.jpg' alt='Radio packet definition' width='722' height='135' /></a>

## Checksum: Hamming 8-4 ##

Right now, a static code is used. Investigate if improvement is worth the effort.
See http://en.wikipedia.org/wiki/Hamming_code

## Fire command ##
A fire command is accepted by a receiver if
  1. cs is correct
  1. f = 1
  1. id matches receiver’s id

## Set power and acknowledge command ##
A set power command is accepted by a receiver if
  1. cs is correct
  1. id matches receiver’s id
  1. group matches receiver’s group

After receiving a valid set power command the receiver waits for 1ms (to give the transmitter time to get ready for reception) and then transmits the set power command back as an acknowledgement. The transmitter waits 10ms for the acknowledgment. If it is sent within that timeframe the new power setting is displayed. If not, the currently selected strength blinks twice as a user feedback for an unsuccessful transmission and remains unchanged.

## Excerpts from rfm12.c ##

```



```

## Quench timings for different flash models ##

_1 µs = 1 micro second = 10E-6 s = 1/1.000.000 s_

| **Power** | **SB-24** | **SB-80-DX** | **SB800** | **SB-25** | **SB-28DX** | **Metz 60 CT-1** |
|:----------|:----------|:-------------|:----------|:----------|:------------|:-----------------|
| 1/1       | 1500 µs   | 8000 µs      | 8400 µs   | 1000 µs   | 1190 µs     | 5000 µs          |
| 1/2       | 640  µs   | 650 µs       | 1045 µs   | 910 µs    | 910 µs      | 2000 µs          |
| 1/4       | 320  µs   | 335  µs      | 520 µs    | 370 µs    | 430 µs      | 1000 µs          |
| 1/8       | 190  µs   | 210  µs      | 285 µs    | 180 µs    | 210 µs      | 500 µs           |
| 1/16      | 130  µs   | 140 µs       | 170 µs    | 90 µs     | 110 µs      | 330 µs           |
| 1/32      | 90  µs    | 98 µs        | 119 µs    | ?         | 50 µs       | 220 µs           |
| 1/64      | 70  µs    | 75 µs        | 85 µs     | ?         | 30 µs       | 160 µs           |

**Note** Only the SB-24 timings were measured with a SPOT!

  * SB-80-DX and SB-800 were measured with different hardware (see [here](http://www.flickr.com/photos/fotoopa_hs/3709381751/in/set-72157611107153997/)).
  * SB-25, SB-28DX and Metz timings are not quench times but flash burn times - that's a difference! Nonetheless these values may serve as a starting point for getting the quench timings right.