# Introduction #
<a href='http://www.flickr.com/photos/9601007@N03/3420380437/' title='Block Diagram von Till_Hamburg bei Flickr'><img src='http://farm4.static.flickr.com/3608/3420380437_5b460b3939_o.jpg' alt='Block Diagram' width='559' height='402' /></a>

_Block diagram of the SPOT components_

# Components #
## The Microcontroller: Atmel Mega8L ##
_8-Kbyte self-programming Flash Program Memory, 1-Kbyte SRAM, 512 Byte EEPROM, 6 or 8 Channel 10-bit A/D-converter. Up to 16 MIPS throughput at 16 Mhz. 2.7 - 5.5 Volt operation._

Datasheet: http://www.atmel.com/dyn/resources/prod_documents/doc2486.pdf

## The Display: Nokia 3310 ##
Display requirements were size, low energy consumption, sufficient resolution and ease of use. The Nokia 3310 can be found online for approximately €6.- and has been used in similar projects.

<a href='http://www.flickr.com/photos/9601007@N03/3420349593/' title='LCD 3310 von Till_Hamburg bei Flickr'><img src='http://farm4.static.flickr.com/3557/3420349593_677bff4526.jpg' alt='LCD 3310' width='500' height='338' /></a>

The display is connected to the microcontroller via the SPI bus (signals “SDIN” and “SCK”) that is also used for the radio module. The addressee of SPI data is determined by chip select (“CS”, “SCE”) signals. The 3310s’ D/C signal differentiates data from command packets. VCC, GND and VOUT provide energy to the display. The principal hurdle is connecting wires to it.

<a href='http://www.flickr.com/photos/9601007@N03/3420363561/' title='Connector LCD 3310 von Till_Hamburg bei Flickr'><img src='http://farm4.static.flickr.com/3298/3420363561_88418eef29.jpg' alt='Connector LCD 3310' width='500' height='292' /></a>

Connectors:

  * VCC (power +)
  * SCK (SPI clock)
  * SDIN (SPI data line)
  * D/C (data / command)
  * SCE (chip select)
  * GND (power -)
  * VOUT (over capacitor to power -)
  * RES (reset)

On the circuit board you can find two 4-pin connectors for the display
(compare 3rd picture from the bottom):

  * VCC --> VCC
  * RES --> RES
  * D/C --> D/C
  * CS --> SCE

  * GND --> GND
  * SCK --> SCK
  * CAP --> VOUT
  * SI --> SDIN

## The Radio Module: HopeRF RFM12B ##
_Hope’s RF12B is a single chip, low power, multi-channel FSK transceiver designed for use in applications requiring FCC or ETSI conformance for unlicensed use in the 433, 868 and 915 MHz bands._ (cited from http://www.hoperf.com/pro/RF12B.html)

Predecessor projects report that the modules’ documentation contains several bugs. However the datasheet of the microprocessor embedded in the HopeRF module seems to be much more reliable.

The following configuration options (based on Benedikt K.’s experiences with his RS232 radio bridge project) were used (numbers refer to the datasheet):

```
rf12_trans(0xC0E0);	// AVR CLK: 10MHz (14. Low Batt. Detector & 
			//       MC Clock Div. Comm.)
rf12_trans(0x80D7);	// Enable FIFO (1. Config Setting Command)
rf12_trans(0xC2AB);	// Data Filter: internal (6. Data Filter Comm.)
rf12_trans(0xCA81);	// Set FIFO mode (7. FIFO and Reset Mode Comm.)
rf12_trans(0xE000);	// disable wakeuptimer (12.)
rf12_trans(0xC800);	// disable low duty cycle (13.)
rf12_trans(0xC4F7);	// AFC settings: autotuning: -10kHz...+7,5kHz (9.)

rf12_setfreq(RF12FREQ(433.92));	// set frequency to 433,92MHz
rf12_setbandwidth(4, 1, 4);	// 200kHz bandwidth, -6dB amplification, DRSSI
				//       threshold: -79dBm
rf12_setbaud(57600);	// 57600 baud
rf12_setpower(0, 6);	// power 1mW, 120kHz frequency shift
```

# Circuit Diagram #
<a href='http://code.google.com/p/strobist-project-opensource-trigger/wiki/SpotSchematics' title='Spot-Layout von Till_Hamburg bei Flickr'><img src='http://farm4.static.flickr.com/3662/3421151642_f534eb6698.jpg' alt='Spot-Layout' width='500' height='317' /></a>

_Please click for a bigger version_

## Circuit Diagram Design Issues ##

  * Programming is done with 5V - dangerous for the display!
  * Resistors for limiting the OC-LED current need to be 30 Ohm instead of 100 Ohm!

# Part List #
Todo: "The rest"

| Antenna | € 10 |
|:--------|:-----|
| Nokia 3310 display | € 7  |
| Case    | € 5  |
| Hotshoe adapter | € 5  |
| RFM12B radio module | € 5  |
| ATMega8L + all the rest | € 15 |
| **Sum** | **€ 47** |


# Circuit Board Design #
<a href='http://www.flickr.com/photos/9601007@N03/3421176408/' title='Circuitboard Topview von Till_Hamburg bei Flickr'><img src='http://farm4.static.flickr.com/3606/3421176408_6dbdfc6959.jpg' alt='Circuitboard Topview' width='500' height='351' /></a>
# Pin Headers on the circuit board #
<a href='http://www.flickr.com/photos/9601007@N03/3421174824/' title='SPOT Pinheaders von Till_Hamburg bei Flickr'><img src='http://farm4.static.flickr.com/3309/3421174824_d302917621.jpg' alt='SPOT Pinheaders' width='500' height='482' /></a>
# Assembly #
<a href='http://www.flickr.com/photos/9601007@N03/3643918003/' title='The other side of the coin von Till_Hamburg bei Flickr'><img src='http://farm4.static.flickr.com/3639/3643918003_8d6e099fc7.jpg' alt='The other side of the coin' width='500' height='370' /></a>

<a href='http://www.flickr.com/photos/9601007@N03/3421178772/' title='SPOT Interior von Till_Hamburg bei Flickr'><img src='http://farm4.static.flickr.com/3281/3421178772_3e3e02fcb0.jpg' alt='SPOT Interior' width='500' height='384' /></a>