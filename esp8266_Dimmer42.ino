#include <ESP8266WiFi.h>
#include <WebSocketsServer.h>

const char* ssid     = "AndroidAP";
const char* password = "seqr5070";

#define N_Channel 2
volatile int counter[N_Channel]={0,0};               // Variable to use as a counter volatile as it is in an interrupt
volatile boolean zero_cross=0;  // Boolean to store a "switch" to tell us if we have crossed zero
int AC_pin[N_Channel] = {5,4};
int dim[N_Channel] = {0,0};                    // Dimming level (0-128)  0 = on, 128 = 0ff

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

WebSocketsServer webSocket = WebSocketsServer(81);                // Server WebSocket a l'ecoute sur le port 81

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t lenght) {       // Ya t il une connexion websocket ?
    Serial.printf("[%u] get Message: %s\r\n", num, payload);
    switch(type) {
        case WStype_DISCONNECTED:         // deconnexion
            break;
        case WStype_CONNECTED:            // connexion
            {
              IPAddress ip = webSocket.remoteIP(num);
              Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\r\n", num, ip[0], ip[1], ip[2], ip[3], payload);    
            }
            break;
        
        case WStype_TEXT:                                                 // lecture du message envoye
            {
              Serial.printf("[%u] get Text: %s\r\n", num, payload);
              String _payload = String((char *) &payload[0]);
              //Serial.println(_payload);
              
              String idOut = (_payload.substring(3,4));                                             // recuperation des données utiles -> Sortie
              String intensity = (_payload.substring(_payload.indexOf(":")+1,_payload.length()));   // -> intensité (ou niveau du pwm)
              int intout = idOut.toInt();                                                           // transforme les string en integer
              int intlevel = intensity.toInt();
              //Serial.print("Output: "); Serial.print(intout); Serial.print(" Intensity: "); Serial.println(intlevel);
              Triac_Dim(intout, intlevel);                                                          // mise a jour de la sortie
              
            }   
            break;     
             
        case WStype_BIN:                                                // lecture d'un fichier binaire
            {
              hexdump(payload, lenght);
            }
            // echo data back to browser
            webSocket.sendBIN(num, payload, lenght);
            break;
  
    }
}

// Turn on the TRIAC at the appropriate time
void ICACHE_RAM_ATTR dim_check() {                            // routine de gestion pwm de la sortie
static unsigned int zccount=N_Channel;

  if(zero_cross == true) {        
    for(int i=0; i < 2; i++) {      
      if(counter[i]>=dim[i]) {                     
        digitalWrite(AC_pin[i], LOW); // turn on light       
        counter[i]=0;  // reset time step counter                         
        //zero_cross = false; //reset zero cross detection
        zccount--;
      } 
      else {
        counter[i]++; // increment time step counter                     
      }                         
    }
    if (zccount==0) {
      zero_cross = false; //reset zero cross detection       
      zccount = N_Channel;
    }
  }  
  timer1_write(freqStep);                                     // rafraichissement du timer 1 (interruption)
}     

void setup() {
  
  Serial.begin(115200);
  for(int i=0; i < N_Channel; i++) {
    pinMode(AC_pin[i], OUTPUT);                          // Set the Triac pin as output
  }
  WiFi.begin(ssid, password);

  while(WiFi.status() != WL_CONNECTED) {
     Serial.print(".");
     delay(200);
  }
    
  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  attachInterrupt(12, zero_cross_detect, RISING);    // Attach an Interupt to Pin 2 (interupt 0) for Zero Cross Detection

  timer1_attachInterrupt(dim_check);
  timer1_enable(TIM_DIV16, TIM_EDGE, TIM_SINGLE);
  timer1_write(freqStep);
  
  delay(300);  
   
  Serial.println("Start Websocket Server");
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
}

void loop() {
  webSocket.loop();
}

void zero_cross_detect() {          // routine de gestion du passage par zero de reseau (interruption)
  zero_cross = true;               // set the boolean to true to tell our dimming function that a zero cross has occured
  for(int i=0; i < N_Channel; i++) {
    digitalWrite(AC_pin[i], HIGH);                          // turn off TRIAC (and AC)
    counter[i]=0;
  }
}       

//void Triac_ON_OFF(){
//
//  String oPWM = server.arg("out");
//  String ostate = server.arg("state");
//  int state= ostate.toInt();
//  
//}

void Triac_Dim(int idout, int level) {

    dim[idout-1] = map(level, 0, 100, 0, 128);
    //dim = level*1.28;
    Serial.print("Sortie: ");
    Serial.print(idout);
    Serial.print(" - Niveau dimmer: ");
    Serial.print(level);
    Serial.print(" - var. dim: ");
    Serial.println(dim[idout-1]);
}
