# Release 1.0 #

Prototype working, ~~all~~ most important mandatory requirements fulfilled
  * **Synch-speed <= 1/250s _(ok!)_**
  * Distance > 30m _(ok, but better measurement needed)_
  * Reliability to be measured, but looks good so far
  * Power adjustment only in full stops yet
  * Compatibility: Camera: Sony Hotshoe and PC-socket, Flash: Nikon SB-24

# Release 1.1 #

  * Tidy up source code
  * Finish documentation

# Release 2 #

  * Personalities: New flashes, new cameras
  * Leave out oscillator and 22pF capacitors (use internal oscillator instead)
  * Buzzer for improved user interface
    * additional buzzer
  * Read in flash ready signal
    * additional optocoupler
    * replace headphone plug with slim RJ-10 / 4P4C
  * Beep on receiver as audio feedback for flash ready
  * Beep on transmitter for “all connected non-zero-strength flashes ready”
    * requires modifications to wireless protocol (“flash ready” polling as 3rd packet  type)
  * Introduce ½ stops of power
  * Protocol choice via UI: High-Speed or Long-Distance ("SH"/"LD")
  * Radio interference indicator
    * Add a value indicator icon to the display
    * Increase the value on each x received but rejected bytes
    * Decrease the value every y seconds
  * Low battery indicator
    * RFM12 provides signal (?)

# Release 3 #

  * Design printed circuit board
  * Add 1,5V -> 3V step-up converter circuit. Only 1 AAA cell needed!
  * Improved synch speed
    * by means of reading the serial protocol between camera and flash for detecting shutter half-press and initializing the radio communication at that moment
    * minus intro sequence of 5 byte!
    * possibly switch to one interrupt for every 4 bits (or less) instead of one per received byte