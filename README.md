# Arduino-LFO
An LFO using an Arduino Uno/Nano and MCP4725 DAC V3. Update

  The LFO uses an Arduino Uno, Nano or Pro Micro and MCP4725 DAC..
A 200 sample array is stored in PORGMEM for a Sine and Triangle waveform..
Timer1 is used to clock thru the array and send the value to the DAC using 
I2C..Timer1 is set using the input from A0, which is mapped to 100 steps to
reduce jitter...The precalculated timer valuse are stored in an array in PROGMEM..
Two SPST switchs set speed High , 1 to 18 Hz, or Low, 30 to 3 seconds, and either
Sine or Triangle wave out...A 120 sample for Expo wave is included in the software
as well as normal and inverted out but I chose not to use them in this project...
  The DAC produces a 1 vpp waveform which is bufferd and then ampilifed thru an
op amp with a gain of aprox 4.7 for an output of 5 vpp with and offest of 2.5 volt..
 
