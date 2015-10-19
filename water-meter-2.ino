// This #include statement was automatically added by the Particle IDE.
#include "HttpClient/HttpClient.h"

// This #include statement was automatically added by the Particle IDE.
#include "ThingSpeak/ThingSpeak.h"

#include "application.h"


// Comment

int sw, Psw;
String Click;
String SVolume;
float Multiplier;
float Volume;
int i;
int Stat;

TCPClient client;

unsigned long myChannelNumber=59727;
const char * myWriteAPIKey="2BMATOYFUS6TLB5P";

int now, pnow;
bool SendTS;

typedef struct {
    int pin;
    int delay;
} LED_PARAM;

int led1 = D7;
int led2 = D3;
int led3 = D1;

LED_PARAM ledParams[3] = {
    {led1, 500},
    {led2, 500},
    {led3, 500}
};

Thread* led1Thread;

/**
* Declaring the variables.
*/
unsigned int nextTime = 0;    // Next time to contact the server
HttpClient http;

// Headers currently need to be set at init, useful for API keys etc.
http_header_t headers[] = {
    //  { "Content-Type", "application/json" },
    //  { "Accept" , "application/json" },
    { "Accept" , "*/*"},
    { NULL, NULL } // NOTE: Always terminate headers will NULL
};

http_request_t request;
http_response_t response;

String Line;

bool ReadyToSend;

int LoopN;
int Diff, MinS;


void setup() {



  pinMode(D0, INPUT);
  pinMode(D1, OUTPUT);
  pinMode(D7, OUTPUT);
  Psw = -1;
  Multiplier = 10.0;
  Volume = 0;
  MinS=20;
//  myChannelNumber=59727;
//  myWriteAPIKey="2BMATOYFUS6TLB5P";
  ThingSpeak.begin(client);
  pnow = -20;

  Serial.begin(9600);
  
  ReadyToSend = true;
  
  LoopN = 0;
  
  Stat = LOW;
  digitalWrite(D7, Stat);
  
  

  //led1Thread = new Thread("D7", ledBlink, &ledParams[0]);
}

void loop() {
    DoLoop();
}

os_thread_return_t ledBlink(void* param){
    LED_PARAM *p = (LED_PARAM*)param;
    
    for(;;) {
        //DoLoop();
        digitalWrite(p->pin, HIGH);
        delay(p->delay);
        digitalWrite(p->pin, LOW);
        delay(p->delay);
    }
}


void DoLoop() {

  LoopN = LoopN + 1;
  
  //Serial.println("Loop");
  //Serial.println(LoopN);
  
  sw = digitalRead(D0);

  digitalWrite(D1, sw);
  
  Serial.print("Switch Before ");
  Serial.print(String(Psw));
  Serial.print("  Switch Now ");
  Serial.println(String(sw));
  
  now = millis();
  
  Diff = (now - pnow)/1000;
  Serial.print(String(Diff));
  Serial.println(" Seconds");
  if (Diff < MinS) {
    SendTS = false;
    Serial.print(String(MinS));
    Serial.println(" too soon to send to Internet");
  } else {
    if ( ReadyToSend ) {
        SendTS = true;
        ReadyToSend = false;
        Serial.println("Send this loop");
    } else {
        SendTS = false;
        Serial.println("Nothing to send this loop");
    }
  }
  Serial.print("SendTS ");
  Serial.println(String(SendTS));

  if (sw != Psw ) {
      Serial.println("Switch Change");
      Click = String(sw);
      if (sw == 0) {
        Serial.println("Switch Closed");
        Volume = Volume + Multiplier;
        SVolume = String(Volume);
        Particle.publish("Volume", SVolume);
        //if ( SendTS ) {
            ThingSpeak.setField(2,SVolume);
        //}
        ReadyToSend = true;
      } else {
        Serial.println("Switch Open");
      }
  }
  
  Particle.publish("Click", Click);
  //if ( SendTS ) {
      ThingSpeak.setField(1,Click);
  //}

  SVolume = String(Volume);
  Serial.print("Volume ");
  Serial.println(SVolume);
  
  if ( SendTS ) {
      
    Serial.println("Sending to Internet");

    Serial.print("Contacting Publish ");
    Serial.println(String(millis()));
    
    Particle.publish("SendTS", String(SendTS));
    Serial.print("Contacted Publish ");
    Serial.println(String(millis()));
    
    Serial.print("Contacting ThingSpeak " );
    Serial.println(String(millis()));
    ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
    Serial.print("Contacted ThingSpeak ");
    Serial.println(String(millis()));
    pnow = now;
  }



  Psw = sw;
  



  //Stat=LOW;
  //digitalWrite(D7, Stat);
  for (i=0; i < 1; i=i+1) {
      if (Stat == LOW){Stat=HIGH;}else{Stat=LOW;}
      digitalWrite(D7, Stat);
      //Serial.print("D7 LED ");
      //Serial.println(Stat);
      delay(500);
      
  }

}


//void MessageS(const char* Line){
//    Serial.println(Line);
//}

