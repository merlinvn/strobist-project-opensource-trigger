# Hardware prerequisites #
## 1. Programmer ##
To load the firmware into the Atmega you need a programmer. I've used an USBISP that I've bought from ebay, but there are several other options. Mine works like a STK500, with the only downside that it doesn't provide power to the board.

## 2. Cable ##
I've made a small cable from a 6 pins header, to 2 seperate pins for +5v and 0v and a 5 pins header, which goes into the 10 pins block which comes from the USBISP. I've put the 2 pins into a HDD power cable from my pc. This way the board has a good powersupply and there is no chance of powerleaks through the USB plug. You could also try to get the power out of the USB plug.

# Configuration of the MC: Setting the fuses #
There are a couple of things you need to take in order before you can load the firmware into your ATMega8. Your ATMega has been shipped with the following settings. Internal Clock 1 MHz; Start-up time: 6 CK + 64 ms. To program the ATMega while it is installed on your board you need to max the frequency to maximum 1/4th of the clock used. A fresh chip runs at 1 MHz so you need to set the ISP Frequency at 250 KHz max! Once you've set the clock to use the external clock, or changed it to 8mhz int. RC, you'll be able to use a higher ISP Frequency.

Inside AVRStudio you need to select Tools, Program AVR, Connect... You get the choice for the right board and COM port. I choose STK500 at port COM3. Once connected you get to choose the right Device: ATMega8. And you need to choose the right programming mode and frequency. ISP Mode and 115KHz is a safe value. After that you click on the button Read Signature. If you recieve an error, there's something wrong. Made a mistake with the pinout, the circuit doesn't have power, or so further.

_Screenshot ISP Frequency_

The next part is setting the Fuses correct. The option CKOPT can be left off. In the dropdown of SUT\_CKSEL you need to choose the right option. In my case, without the crystal I've choosen Int RC. 8MHZ 6 CK 64ms. To use the crystal, for a more stable and accurate clock, you'd choose Ext RC. 8MHz 16K CK 65 ms. Be carefull not to touch SPIEN and RSTDISBL. If you mark them, you won't be able to program this way anymore. Press program, and verify after that. The new settings are now programmed into the chip. After an restart the new settings are ready.

_Screenshot Fuses_

# Loading the firmware #
Now we need to program the spot.hex into the chip. Go to the program tab. In the flash-part browse to the location of spot.hex. Press the button Program. The software will do some things, and when it's ready you push verify. Now you're able to check if the programming is done. If so, you're ready.

_Screenshot Program_

Leave the programmer and disconnect the wires. Reconnect the LCD Display and reconnect the battery's. Now your SPOT should start to breath :)

_Picture: Alive SPOT_