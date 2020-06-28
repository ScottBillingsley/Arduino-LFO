/*                 LFO V3.
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
  *                      3        PWM LED
  *         4            4        Wave restart   
  *         2           A4        SDA
  *         3           A5        SCL
  *         10          10        Speed, Hi Low   (10)
  *         11          11        Wave, Sine Tri  (11)
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

#define r_pin 4
#define led_pin 3
#define s_pin 10
#define w_pin 11
#define dac_i2c 0x60      //Change to match your DAC

boolean write_dac = false;
boolean new_dac_value = true;
boolean change = false;
boolean r_next = true;
int dac_value;
byte dac_count = 0;
byte wave_select = 1;
byte speed_multiply = 1;    //0 = Fast 1 to 30 Hz, 1 = Slow 30 to 1 second..

int a0_val;
int old_a0_val;

int interrupt_val;

const int dac_sine[] PROGMEM = {
0x19a,0x1a7,0x1b4,0x1c1,0x1cd,0x1da,0x1e7,0x1f3,0x200,0x20c,0x219,0x225,0x231,0x23d,0x249,0x254,0x260,0x26b,0x276,0x280,
0x28b,0x295,0x29f,0x2a9,0x2b3,0x2bc,0x2c5,0x2ce,0x2d6,0x2de,0x2e6,0x2ed,0x2f4,0x2fb,0x301,0x307,0x30d,0x312,0x317,0x31c,
0x320,0x324,0x327,0x32a,0x32d,0x32f,0x331,0x332,0x333,0x334,0x334,0x334,0x333,0x332,0x331,0x32f,0x32d,0x32a,0x327,0x324,
0x320,0x31c,0x317,0x312,0x30d,0x307,0x301,0x2fb,0x2f4,0x2ed,0x2e6,0x2de,0x2d6,0x2ce,0x2c5,0x2bc,0x2b3,0x2a9,0x29f,0x295,
0x28b,0x280,0x276,0x26b,0x260,0x254,0x249,0x23d,0x231,0x225,0x219,0x20c,0x200,0x1f3,0x1e7,0x1da,0x1cd,0x1c1,0x1b4,0x1a7,
0x19a,0x18d,0x180,0x173,0x167,0x15a,0x14d,0x141,0x134,0x128,0x11b,0x10f,0x103,0xf7,0xeb,0xe0,0xd4,0xc9,0xbe,0xb4,
0xa9,0x9f,0x95,0x8b,0x81,0x78,0x6f,0x66,0x5e,0x56,0x4e,0x47,0x40,0x39,0x33,0x2d,0x27,0x22,0x1d,0x18,
0x14,0x10,0xd,0xa,0x7,0x5,0x3,0x2,0x1,0x0,0x0,0x0,0x1,0x2,0x3,0x5,0x7,0xa,0xd,0x10,
0x14,0x18,0x1d,0x22,0x27,0x2d,0x33,0x39,0x40,0x47,0x4e,0x56,0x5e,0x66,0x6f,0x78,0x81,0x8b,0x95,0x9f,
0xa9,0xb4,0xbe,0xc9,0xd4,0xe0,0xeb,0xf7,0x103,0x10f,0x11b,0x128,0x134,0x141,0x14d,0x15a,0x167,0x173,0x180,0x18d,
};

const int dac_triangle[] PROGMEM = {
0,8,16,24,32,40,48,56,64,72,80,88,96,104,112,120,128,136,144,152,
160,168,176,184,192,200,208,216,224,232,240,248,256,264,272,280,288,296,304,312,
320,328,336,344,352,360,368,376,384,392,400,408,416,424,432,440,448,456,464,472,
480,488,496,504,512,520,528,536,544,552,560,568,576,584,592,600,608,616,624,632,
640,648,656,664,672,680,688,696,704,712,720,728,736,744,752,760,768,776,784,792,
800,792,784,776,768,760,752,744,736,728,720,712,704,696,688,680,672,664,656,648,
640,632,624,616,608,600,592,584,576,568,560,552,544,536,528,520,512,504,496,488,
480,472,464,456,448,440,432,424,416,408,400,392,384,376,368,360,352,344,336,328,
320,312,304,296,288,280,272,264,256,248,240,232,224,216,208,200,192,184,176,168,
160,152,144,136,128,120,112,104,96,88,80,72,64,56,48,40,32,24,16,8,

};

const int dac_expo[] PROGMEM = {
0x0,0x10,0x20,0x30,0x3f,0x4e,0x5d,0x6b,0x79,0x87,0x95,0xa2,0xaf,0xbc,0xc8,0xd5,0xe1,0xec,0xf8,0x103,
0x10e,0x119,0x124,0x12e,0x139,0x143,0x14c,0x156,0x160,0x169,0x172,0x17b,0x184,0x18c,0x195,0x19d,0x1a5,0x1ad,0x1b5,0x1bc,
0x1c4,0x1cb,0x1d2,0x1d9,0x1e0,0x1e7,0x1ed,0x1f4,0x1fa,0x200,0x206,0x20c,0x212,0x218,0x21e,0x223,0x228,0x22e,0x233,0x238,
0x23d,0x242,0x247,0x24b,0x250,0x255,0x259,0x25d,0x262,0x266,0x26a,0x26e,0x272,0x276,0x279,0x27d,0x281,0x284,0x288,0x28b,
0x28e,0x292,0x295,0x298,0x29b,0x29e,0x2a1,0x2a4,0x2a7,0x2aa,0x2ac,0x2af,0x2b2,0x2b4,0x2b7,0x2b9,0x2bc,0x2be,0x2c0,0x2c3,
0x2c5,0x2c7,0x2c9,0x2cb,0x2ce,0x2d0,0x2d2,0x2d4,0x2d5,0x2d7,0x2d9,0x2db,0x2dd,0x2de,0x2e0,0x2e2,0x2e3,0x2e5,0x2e7,0x2e8,
0x2ea,0x2eb,0x2ed,0x2ee,0x2ef,0x2f1,0x2f2,0x2f3,0x2f5,0x2f6,0x2f7,0x2f8,0x2f9,0x2fb,0x2fc,0x2fd,0x2fe,0x2ff,0x300,0x301,
0x302,0x303,0x304,0x305,0x306,0x307,0x308,0x309,0x30a,0x30a,0x30b,0x30c,0x30d,0x30e,0x30e,0x30f,0x310,0x311,0x311,0x312,
0x313,0x313,0x314,0x315,0x315,0x316,0x316,0x317,0x318,0x318,0x319,0x319,0x31a,0x31a,0x31b,0x31b,0x31c,0x31c,0x31d,0x31d,
0x31e,0x31e,0x31e,0x31f,0x31f,0x320,0x320,0x321,0x321,0x321,0x322,0x322,0x322,0x323,0x323,0x323,0x324,0x324,0x324,0x325,  
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


  pinMode(r_pin, INPUT);
  pinMode(led_pin, OUTPUT);
  pinMode(s_pin, INPUT_PULLUP);
  pinMode(w_pin, INPUT_PULLUP);  

  Wire.begin();
  Wire.setClock(800000L);

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
elapsedMillis r_wait;         // debounce restart pin

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
      dac_value = pgm_read_word_near(&dac_expo[200 - dac_count]);   
      break;
}

  new_dac_value = false;
}

if(write_dac){
  dac_write(dac_value);
  write_dac = false;
  new_dac_value = true;
  dac_count ++;
    //analogWrite(led_pin, dac_value / 5);
  if(dac_count > 199){
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
  //Serial.println(a0_step + 1);
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

/*wave restart on pin 4*/
if(digitalRead(r_pin) && r_next && r_wait > 30){
      //dac_count = 0; 
      r_next = false;
}
if(!digitalRead(r_pin) && !r_next){
  r_next = true;
  r_wait = 0;
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
