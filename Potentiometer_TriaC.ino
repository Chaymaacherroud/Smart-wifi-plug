
#include  <TimerOne.h>          // Avaiable from http://www.arduino.cc/playground/Code/Timer1
volatile int i=0;               // Variable to use as a counter volatile as it is in an interrupt
volatile boolean zero_cross=0;  // Boolean to store a "switch" to tell us if we have crossed zero
int AC_pin = 11;                // Output to Opto Triac
int dim = 0;  // Dimming level (0-128)  0 = on, 128 = 0ff
int Dim=0;
int inc=1;  // counting up or down, 1=up, -1=down
int potPin = A0; 

int freqStep = 78;    // This is the delay-per-brightness step in microseconds.
                      // For 60 Hz it should be 65
long zcrosstime = 0L;               
// It is calculated based on the frequency of your voltage supply (50Hz or 60Hz)
// and the number of brightness steps you want. 
// 
// Realize that there are 2 zerocrossing per cycle. This means
// zero crossing happens at 120Hz for a 60Hz supply or 100Hz for a 50Hz supply. 

// To calculate freqStep divide the length of one full half-wave of the power
// cycle (in microseconds) by the number of brightness steps. 
//
// (120 Hz=8333uS) / 128 brightness steps = 65 uS / brightness step
// Firing angle calculation : 1 full 50Hz wave =1/50=20ms 
// Every zerocrossing thus: (50Hz)-> 10ms (1/2 Cycle) 
// 10ms=10000us
// (100Hz=10000uS) / 128 steps = 75uS/step

void setup() {                                      // Begin setup

Serial.begin(9600);
  pinMode(AC_pin, OUTPUT);                          // Set the Triac pin as output
  attachInterrupt(0, zero_cross_detect, RISING);    // Attach an Interupt to Pin 2 (interupt 0) for Zero Cross Detection
  Timer1.initialize(freqStep);                      // Initialize TimerOne library for the freq we need
  Timer1.attachInterrupt(dim_check, freqStep);      
  
  // Use the TimerOne Library to attach an interrupt
  // to the function we use to check to see if it is 
  // the right time to fire the triac.  This function 
  // will now run every freqStep in microseconds.                                            
}
/*
Pin    |  Interrrupt # | Arduino Platform
---------------------------------------
2      |  0            |  All -But it is INT1 on the Leonardo
3      |  1            |  All -But it is INT0 on the Leonardo
18     |  5            |  Arduino Mega Only
19     |  4            |  Arduino Mega Only
20     |  3            |  Arduino Mega Only
21     |  2            |  Arduino Mega Only
0      |  0            |  Leonardo
1      |  3            |  Leonardo
7      |  4            |  Leonardo
The Arduino Due has no standard interrupt pins as an iterrupt can be attached to almosty any pin. 

In the program pin 2 is chosen
*/
void zero_cross_detect() {    
  static unsigned long startMillis = 0L; 
  int freqstep ;
  zero_cross = true;               // set the boolean to true to tell our dimming function that a zero cross has occured
  i=0;
      if (startMillis == 0L) {
        startMillis = millis(); 
      }
      else {
        unsigned long currentMillis = millis();
        zcrosstime = (currentMillis - startMillis)/1e3; 
        startMillis = currentMillis;
        freqstep =zcrosstime/128;
      } 
  digitalWrite(AC_pin, LOW);       // turn off TRIAC (and AC)
}                                 

// Turn on the TRIAC at the appropriate time
void dim_check() {                   
  if(zero_cross == true) {              
    if(i>=dim) {                     
      digitalWrite(AC_pin, HIGH); // turn on light       
      i=0;  // reset time step counter                         
      zero_cross = false; //reset zero cross detection
    } 
    else {
      i++; // increment time step counter 
    }                                
  }                                  
}                                   

void loop() { 

 /* dim+=inc;
  if((dim>=128) || (dim<=0))
    inc*=-1;
  delay(18); */
  Dim=analogRead(potPin)>>2;
  dim=(Dim*128)/255;
  Serial.println(dim);
  
}



