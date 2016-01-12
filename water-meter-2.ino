// This #include statement was automatically added by the Particle IDE.
#include "HttpClient.h"

// This #include statement was automatically added by the Particle IDE.
#include "ThingSpeak.h"

#include "application.h"




int sw, Psw;
String Click;
String SVolume;
float Multiplier;
float Volume;
int i;
int Stat;
String Version;

TCPClient client;

//ThingSpeak identifiers
unsigned long myChannelNumber=MYCHANNELNUMBER;
const char * myWriteAPIKey="MYWRITEAPIKEY";

int now, pnow;
bool SendTS, SendTSDone;

Thread* ThreadGetStatus;
Thread* ThreadSendMessage;
Thread* ThreadFlashLED;

//If just after switch on then send status to ThingSpeak
bool FirstTime;

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

int Diff, MinS;

//Sets LED response to switch position
//D1 Set Green
int Switch1[2] = {HIGH, LOW};
//D2 Set Blue
int Switch2[2] = {LOW, HIGH};

WLanSelectAntenna_TypeDef AntSel;


void setup() {


//Switch
  pinMode(D0, INPUT);
//Multicoloured LED
  pinMode(D1, OUTPUT);
  pinMode(D2, OUTPUT);
  pinMode(D3, OUTPUT);
//Onboard Blue LED
  pinMode(D7, OUTPUT);

//Each closed contact on the switch is 10litres of water
  Multiplier = 10.0;
//Inital Volume used
  Volume = 0;
//Time between checks to send to ThinkSpeak
  MinS=20;

  ThingSpeak.begin(client);
  pnow = 0;

  Serial.begin(9600);

  ReadyToSend = true;

  Stat = LOW;
// Turn onboard Blue LED off
  digitalWrite(D7, Stat);

  Version="2.5";

//Get Threads ready
  ThreadGetStatus = new Thread("Get Switch Status", LoopGetStatus);
  ThreadSendMessage = new Thread("Send Message to ThingSpeak", LoopSendMessage);
  ThreadFlashLED = new Thread("Flash the onboard Blue LED", LoopFlashLED);

  SendTS = false;
  SendTSDone = false;

  FirstTime = true;

// Select internal or external antenna
  //AntSel = ANT_AUTO;
  //AntSel = ANT_INTERNAL;
  AntSel = ANT_EXTERNAL;

  WiFi.selectAntenna(AntSel);
}

void loop() {
    //Main loop does nothing
}

//Flash the Blue onboard LED
void FlashLED(){
//Could do somehting more fancy here
  for (i=0; i < 1; i=i+1) {
      if (Stat == LOW){Stat=HIGH;}else{Stat=LOW;}
      digitalWrite(D7, Stat);
      Serial.print("D7 LED ");
      Serial.println(Stat);
      delay(500);
  }
}

//Get the status of the water meter switch
os_thread_return_t LoopGetStatus(void* param){
    for(;;) {
        GetStatus();
    }
}

//Sets Blue LED pin on multicoloured LED if sending to ThinkSpeak
os_thread_return_t LoopSendMessage(void* param){
    int mySendTS;
    for(;;) {
      mySendTS = SendTS;

      digitalWrite(D3, mySendTS);
      SendToService(mySendTS);

    }
}

//Flashes onboard Blue LED
os_thread_return_t LoopFlashLED(void* param){
    for(;;) {
      FlashLED();
    }
}


// Reads switch status and prepare signal to ThingSpeak
void GetStatus() {

  Serial.print("Version ");
  Serial.println(Version);

  sw = digitalRead(D0);

// If just turned on then send data to ThingSpeak
  if ( FirstTime ) {
    Psw = sw;
    SVolume = String(Volume);
    Particle.publish("Volume", SVolume);
    ThingSpeak.setField(2,SVolume);
    ReadyToSend = true;
    FirstTime = false;
    SendTS = true;
    pnow = millis();
  }

//Set multicoloured LED
  digitalWrite(D1, Switch1[sw]);
  digitalWrite(D2, Switch2[sw]);

  if (sw != Psw ) {
      //Serial.println("Switch Change");
      Click = String(sw);
      if (sw == 0) {
        Serial.println("Switch Closed");
        Volume = Volume + Multiplier;
        SVolume = String(Volume);
        Particle.publish("Volume", SVolume);
        ThingSpeak.setField(2,SVolume);
        ReadyToSend = true;
      } else {
        Serial.println("Switch Open");
      }
  }

  now = millis();

  Diff = (now - pnow)/1000;

  if (SendTSDone) {
    if (Diff < MinS) {
      SendTS = false;
    } else {
      if ( ReadyToSend ) {
        SendTS = true;
        ReadyToSend = false;
      } else {
        SendTS = false;
      }
    }
  }

  SVolume = String(Volume);

//Previous switch state
  Psw = sw;

}

void SendToService(bool mySendTS) {

  if ( mySendTS ) {
    Particle.publish("SendTS SendToService", String(mySendTS));
    ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
    pnow = now;
    delay(10);
    Particle.publish("SendTS SendToService End", String(mySendTS));
    SendTSDone = true;
  }
}
