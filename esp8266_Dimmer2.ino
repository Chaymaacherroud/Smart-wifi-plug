/*
 Change: zero-crossing detection to look for RISING edge rather
 than falling.  (originally it was only chopping the negative half
 of the AC wave form). 
 
 Also the dim_check() to turn on the Triac, leaving it on 
 until the zero_cross_detect() turn's it off.
 
 */

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
//#include <Ticker.h>

const char* ssid     = "SEMI_PUBLIC";
const char* password = "microelecpub";


volatile int i=0;               // Variable to use as a counter volatile as it is in an interrupt
volatile boolean zero_cross=0;  // Boolean to store a "switch" to tell us if we have crossed zero
int AC_pin = 5;                // Output to Opto Triac
int dim = 0;                    // Dimming level (0-128)  0 = on, 128 = 0ff


// It is calculated based on the frequency of your voltage supply (50Hz or 60Hz)
// and the number of brightness steps you want. 
// 
// Realize that there are 2 zerocrossing per cycle. This means
// zero crossing happens at 120Hz for a 60Hz supply or 100Hz for a 50Hz supply. 

// To calculate freqStep divide the length of one full half-wave of the power
// cycle (in microseconds) by the number of brightness steps. 
// 
// The ESP8266 run at 80MHz - For the setup Timer1 see below
//  TIM_DIV1 = 0,   //80MHz (80 ticks/us - 104857.588 us max)
//  TIM_DIV16 = 1,  //5MHz (5 ticks/us - 1677721.4 us max)
//  TIM_DIV256 = 3  //312.5Khz (1 tick = 3.2us - 26843542.4 us max)
//
// Whith TIM_DIV16 we are 5 ticks/us we want 10ms(100Hz) / 128(iteration) = 78us for the step (50Hz)
//                                           8,33ms(120Hz) / 128          = 65us for the step (60Hz)
// we are need 65 * 5 (ticks/us) = 325 for the value freqStep (60hz)
// we are need 78 * 5 (ticks/us) = 390 for the value freqStep (50hz)
//

int freqStep = 390;    // This is the delay-per-brightness step in microseconds.

ESP8266WebServer server(80);

                      
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
  timer1_write(freqStep);                                
}                                   


void setup() {                                      // Begin setup

  Serial.begin(115200);
  Serial.println("Connecting Wifi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP()); 
  
  pinMode(AC_pin, OUTPUT);                          // Set the Triac pin as output
  attachInterrupt(12, zero_cross_detect, RISING);    // Attach an Interupt to Pin 2 (interupt 0) for Zero Cross Detection

  timer1_attachInterrupt(dim_check);
  timer1_enable(TIM_DIV16, TIM_EDGE, TIM_SINGLE);
  timer1_write(freqStep);
  
  server.on("/ON", Triac_ON_OFF);   // http://192.168.10.101/ON?out=1&state=1
  server.on("/dimm", Triac_Dim);    // http://192.168.10.101/dimm?out=1&level=50
  server.begin(); 

  //ESP.wdtDisable();
}
                      


void loop() {                        

  server.handleClient();
  
//  dim+=inc;
//  if((dim>=128) || (dim<=0))
//    inc*=-1;
  delay(18);
}


void zero_cross_detect() {    
  zero_cross = true;               // set the boolean to true to tell our dimming function that a zero cross has occured
  i=0;
  digitalWrite(AC_pin, LOW);       // turn off TRIAC (and AC)
}       

void Triac_ON_OFF(){

  String oPWM = server.arg("out");
  String ostate = server.arg("state");
  int state= ostate.toInt();
	
}

void Triac_Dim() {

  String oPWM = server.arg("out");
  String oLevel = server.arg("level");
  int level= oLevel.toInt();
  if ((level >= 0)&&(level <= 100)){

    dim = level*1.28;
    Serial.print("Niveau dimmer: ");
    Serial.print(level);
    Serial.print(" - var. dim: ");
    Serial.println(dim);
  }
  
}
