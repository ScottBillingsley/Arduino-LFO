/*                 LFO V2.
 *                 Copyright (c) 2020 Vernon Billingsley  
 *                     
 *                     
  * Permission is hereby granted, free of charge, to any person obtaining a copy
  * of this software and associated documentation files (the "Software"), to deal
  * in the Software without restriction, including without limitation the rights
  * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  * copies of the Software, and to permit persons to whom the Software is
  * furnished to do so, subject to the following conditions:
  *
  * The above copyright notice and this permission
  * notice shall be included in all copies or substantial portions of the Software.
  *
  * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  * THE SOFTWARE.   
  * 
  *         Pro Micro   Uno/Nano
  *         Pin                   Function
  *         2           A4        SDA
  *         3           A5        SCL
  *         8           8         Speed, Hi Low
  *         9           9         Wave, Sine Tri
  *         A0          A0        Freq Adj
   */


#include <avr/pgmspace.h>
#include <Wire.h>
#include <elapsedMillis.h>


#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

#define s_pin 8
#define w_pin 9
#define dac_i2c 0x60      //Change to match your DAC

boolean write_dac = false;
boolean new_dac_value = true;
boolean change = false;
int dac_value;
byte dac_count = 0;
byte wave_select = 1;
byte speed_multiply = 1;    //0 = Fast 1 to 30 Hz, 1 = Slow 30 to 1 second..

int a0_val;
int old_a0_val;

int interrupt_val;

const int dac_sine[] PROGMEM = {
0x1c2,0x1da,0x1f1,0x208,0x220,0x236,0x24d,0x263,0x279,0x28e,0x2a3,0x2b7,0x2cb,0x2dd,0x2ef,0x300,0x310,0x320,0x32e,0x33b,
0x348,0x353,0x35d,0x366,0x36e,0x375,0x37a,0x37e,0x382,0x383,0x384,0x383,0x382,0x37e,0x37a,0x375,0x36e,0x366,0x35d,0x353,
0x348,0x33b,0x32e,0x320,0x310,0x300,0x2ef,0x2dd,0x2cb,0x2b7,0x2a3,0x28e,0x279,0x263,0x24d,0x236,0x220,0x208,0x1f1,0x1da,
0x1c2,0x1aa,0x193,0x17c,0x164,0x14e,0x137,0x121,0x10b,0xf6,0xe1,0xcd,0xb9,0xa7,0x95,0x84,0x74,0x64,0x56,0x49,
0x3c,0x31,0x27,0x1e,0x16,0xf,0xa,0x6,0x2,0x1,0x0,0x1,0x2,0x6,0xa,0xf,0x16,0x1e,0x27,0x31,
0x3c,0x49,0x56,0x64,0x74,0x84,0x95,0xa7,0xb9,0xcd,0xe1,0xf6,0x10b,0x121,0x137,0x14e,0x164,0x17c,0x193,0x1aa,
};

const int dac_triangle[] PROGMEM = {
0,15,30,45,60,75,90,105,120,135,150,165,180,195,210,225,240,255,270,285,
300,315,330,345,360,375,390,405,420,435,450,465,480,495,510,525,540,555,570,585,
600,615,630,645,660,675,690,705,720,735,750,765,780,795,810,825,840,855,870,885,
900,885,870,855,840,825,810,795,780,765,750,735,720,705,690,675,660,645,630,615,
600,585,570,555,540,525,510,495,480,465,450,435,420,405,390,375,360,345,330,315,
300,285,270,255,240,225,210,195,180,165,150,135,120,105,90,75,60,45,30,15,
};

const int dac_expo[] PROGMEM = {
0x0,0xf,0x1e,0x2c,0x3a,0x48,0x56,0x63,0x70,0x7d,0x8a,0x97,0xa3,0xaf,0xbb,0xc7,0xd3,0xde,0xe9,0xf4,
0xff,0x10a,0x114,0x11f,0x129,0x133,0x13c,0x146,0x150,0x159,0x162,0x16b,0x174,0x17d,0x185,0x18e,0x196,0x19e,0x1a6,0x1ae,
0x1b6,0x1be,0x1c5,0x1cc,0x1d4,0x1db,0x1e2,0x1e9,0x1f0,0x1f6,0x1fd,0x203,0x20a,0x210,0x216,0x21c,0x222,0x228,0x22e,0x233,
0x239,0x23e,0x244,0x249,0x24e,0x253,0x258,0x25d,0x262,0x267,0x26c,0x270,0x275,0x279,0x27e,0x282,0x286,0x28b,0x28f,0x293,
0x297,0x29b,0x29f,0x2a2,0x2a6,0x2aa,0x2ad,0x2b1,0x2b4,0x2b8,0x2bb,0x2bf,0x2c2,0x2c5,0x2c8,0x2cb,0x2ce,0x2d1,0x2d4,0x2d7,
0x2da,0x2dd,0x2e0,0x2e2,0x2e5,0x2e8,0x2ea,0x2ed,0x2ef,0x2f2,0x2f4,0x2f6,0x2f9,0x2fb,0x2fd,0x300,0x302,0x304,0x306,0x308,  
};

//  1 to 30 Hz
const int fast_lfo[] PROGMEM = {
2082,1601,1301,1095,945,832,743,671,611,562,
519,483,451,424,399,377,358,340,324,309,
296,284,273,262,253,244,235,227,220,213,
207,201,195,190,185,180,175,171,167,163,
159,155,152,148,145,142,139,136,134,131,
129,126,124,122,120,118,116,114,112,110,
108,106,105,103,102,100,99,97,96,95,
93,92,91,89,88,87,86,85,84,83,
82,81,80,79,78,77,76,75,75,74,
73,72,71,71,70,69,68,68,67,66,

};

void setup() {
//debug
  Serial.begin(115200);

  pinMode(s_pin, INPUT_PULLUP);
  pinMode(w_pin, INPUT_PULLUP);  

  Wire.begin();
  Wire.setClock(400000);

  cli();                //stop interrupts
  
//set timer1 
  TCCR1A = 0;// set entire TCCR1A register to 0
  TCCR1B = 0;// same for TCCR1B
  TCNT1  = 0;//initialize counter value to 0
  // set compare match register
  //OCR1A = ((16000000 / (num samples * prescale * hZ)) - 1)
  //Start Clock 1 on 30 Hz
  OCR1A = 68;
  // turn on CTC mode
  sbi(TCCR1B, WGM12);
  // Set 64 prescaler
  cbi(TCCR1B, CS12);
  sbi(TCCR1B, CS11);
  sbi(TCCR1B, CS10);  
  // enable timer compare interrupt
  sbi(TIMSK1, OCIE1A); 
  sei();                //allow interrupts
}//*****************************  End Setup   **************************


//ISR to handle Timer 1 interrupt
ISR(TIMER1_COMPA_vect){ 
    //Tell an interrupt has occured 
    write_dac = true;
  }

elapsedMillis check_A0;       // Check A0 every 10 ms  

void loop() {
if(!write_dac && new_dac_value){
  switch(wave_select){
    case 0:   
      dac_value = pgm_read_word_near(&dac_sine[dac_count]);
      break;
    case 1:
      dac_value = pgm_read_word_near(&dac_triangle[dac_count]);
      break;
    case 2:  
      dac_value = pgm_read_word_near(&dac_expo[dac_count]); 
      break;
    case 3:  
      dac_value = pgm_read_word_near(&dac_expo[120 - dac_count]);   
      break;
}

  new_dac_value = false;
}

if(write_dac){
  dac_write(dac_value);
  write_dac = false;
  new_dac_value = true;
  dac_count ++;
  if(dac_count > 119){
    dac_count = 0;
  }
}

//************************* After Timer 1 interrupt
if(!write_dac){
  //Check A0 every 10 ms
  if(check_A0 > 10){
  a0_val = (a0_val + analogRead(A0)) / 2;       //read and average
  int a0_step = map(a0_val, 0, 1024, 0, 99);    //Get the interrupt step
  if(a0_step > old_a0_val || a0_step < old_a0_val || change){
  OCR1A = (pgm_read_word_near(&fast_lfo[a0_step]) * speed_multiply);     //Set the new interrupt time
  Serial.println(a0_step + 1);
  change = false;
  }
  old_a0_val = a0_step;
  check_A0 = 0;
  }
if(!digitalRead(s_pin)){
  speed_multiply = 30;
  change = true;  
}else{
  speed_multiply = 1;
  change = true;
}
if(!digitalRead(w_pin)){
  wave_select = 1;
  change = true;  
}else{
  wave_select = 0;
  change = true;
}
  
}

}//***********************  End Loop    ***********************************

void dac_write(int cc){
  Wire.beginTransmission(dac_i2c);
  Wire.write(64);                     // cmd to update the DAC
  Wire.write(cc >> 4);        // the 8 most significant bits...
  Wire.write((cc & 15) << 4); // the 4 least significant bits...
  Wire.endTransmission();
  write_dac = false;                //Reset for the next interrupt  
}
