#include <SPI.h>
#include <Ethernet.h>

// This example will log some basic data to a stream at daqlog.com
// It will generate a 'sawtooth' and 'triangle wave
// It can easily be adapted to log from sensors or on events
//
// Hardware required: Arduino Ethernet or an Arduino Uno Ethernet Shield
//   This has not been tested on anything else, but may work
//
// You'll need to:
//   - register at www.daqlog.com
//   - create a Stream
//   - add two Data Streams with the params: 'sawtooth' and 'triangle'
//   - generate an API Key
// 
// The code will need to be customized as indicated in the comments


// --- insert your API key here ---
char apiKey[] = "";
// --- insert your logging interval (in minutes) ---
long postTimeMin = 5;
// --- your Arduino's MAC address (sticker on bottom, or make one up (hex))
byte mac[] = { 0x24, 0x9F, 0xD7, 0x46, 0xA1, 0x72 };

// define our test variables
float streamSawtooth = 0.0;
float streamTriangle = 0.0;
// data is POSTed as a str, so define a char buffer for each variable
char dataSawtooth[10];
char dataTriangle[10];

char serverName[] = "daqlog.com";

EthernetClient client;

// these are used for our POST interval timers
unsigned long currentMillis;
unsigned long previousMillis;

// forces an ethernet connection on first run and after failures
boolean forceRun = true;

// helper for the triangle generation
boolean goingUp = true;

void setup(void){
  Serial.begin(9600);
  Ethernet.begin(mac);
  Serial.println("Setup Complete");
}

void post_to_server(){
  String data;
  data+="";
  // USE STREAM PARAMS HERE
  data+="sawtooth=";
  data+=dataSawtooth;
  // ANOTHER PARAM
  data+="&triangle=";
  data+=dataTriangle;
  // API KEY
  data+="&key=";
  data+=apiKey;
  
  Serial.println("connecting to server");
  if (client.connect(serverName, 80)>0) {
    Serial.println("connected");
    client.println("POST /streams/api/v1/add_data/ HTTP/1.1");
    client.println("Host: daqlog.com");
    client.println("Connection: close");
    client.println("Content-Type: application/x-www-form-urlencoded");
    client.print("Content-Length: ");
    client.println(data.length());
    client.println();
    client.print(data);
    client.println();
    Serial.println("POST sent");
    delay(100);
    client.stop();
    Serial.println("stopped");
  } else {
    Serial.println("not connected!");
    forceRun = true;
  }
}

void increment_triangle(){
    if (goingUp && (streamTriangle < 5.0)){
      streamTriangle = streamTriangle + 1.0;
    } else if (goingUp && (streamTriangle >= 5.0)){
      goingUp = false;
      streamTriangle = streamTriangle - 1.0;
    } else if (!goingUp && (streamTriangle > -5.0)){
      streamTriangle = streamTriangle -1.0;
    } else if (!goingUp && (streamTriangle <= -5.0)){
      goingUp = true;
      streamTriangle = streamTriangle + 1.0;
    }
}

void increment_sawtooth(){
   if (streamSawtooth < 10.0){
     streamSawtooth = streamSawtooth + 1.0;
   } else {
     streamSawtooth = 0.0;
   }
}
 
void loop(void){
  currentMillis = millis(); 
  
  if ((currentMillis - previousMillis) > (postTimeMin*60*1000)) {
    Serial.println("interval hit");
    previousMillis = currentMillis;
    forceRun = false;
    increment_triangle(); 
    dtostrf(streamTriangle,3,2,dataTriangle);
    increment_sawtooth();
    dtostrf(streamSawtooth,3,2,dataSawtooth); 
    post_to_server();
    
  } else {
    Serial.print(currentMillis - previousMillis);
    Serial.print(" < ");
    Serial.println(postTimeMin*60*1000);
  }
  
  if (forceRun == true) {
    Serial.println("Force run");
    forceRun = false;
    post_to_server();
  }
}
