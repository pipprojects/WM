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
int StatR, Dir, StatRMin, StatRMax;
String Version;

TCPClient client;

unsigned long myChannelNumber=59727;
const char * myWriteAPIKey="2BMATOYFUS6TLB5P";

int now, pnow;
bool SendTS, SendTSDone;

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
Thread* led2Thread;
Thread* led3Thread;

int As;

bool FirstTime;

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

int Switch1[2] = {HIGH, LOW};;
int Switch2[2] = {LOW, HIGH};;

WLanSelectAntenna_TypeDef AntSel;


void setup() {



  pinMode(D0, INPUT);
  pinMode(D1, OUTPUT);
  pinMode(D2, OUTPUT);
  pinMode(D3, OUTPUT);
  pinMode(D7, OUTPUT);

  Multiplier = 10.0;
  Volume = 0;
  MinS=20;
//  myChannelNumber=59727;
//  myWriteAPIKey="2BMATOYFUS6TLB5P";
  ThingSpeak.begin(client);
  pnow = 0;

  Serial.begin(9600);

  ReadyToSend = true;

  LoopN = 0;

  Stat = LOW;
  digitalWrite(D7, Stat);

  StatRMin = 0;
  StatRMax = 255;
  StatR = StatRMin;
  As = LOW;
  //analogWrite(D7, StatR);

  Dir = 1;

  Version="2.4";

  //Switch1 = {HIGH, LOW};
  //Switch2 = {LOW, HIGH};




  led1Thread = new Thread("D7", ledBlink, &ledParams[0]);
  led2Thread = new Thread("D7", Loop2, &ledParams[0]);
  led3Thread = new Thread("D7", Loop3, &ledParams[0]);

  SendTS = false;
  SendTSDone = false;

  FirstTime = true;

  //AntSel = ANT_AUTO;
  //AntSel = ANT_INTERNAL;
  AntSel = ANT_EXTERNAL;

  WiFi.selectAntenna(AntSel);



}

void loop() {
    //DoLoop();
}

void flashLED(){
  //Particle.publish("SendTS Loop 3", String(SendTS));
  for (i=0; i < 1; i=i+1) {
      if (Stat == LOW){Stat=HIGH;}else{Stat=LOW;}
      digitalWrite(D7, Stat);
      Serial.print("D7 LED ");
      Serial.println(Stat);
      delay(500);
  }
}

os_thread_return_t ledBlink(void* param){
    LED_PARAM *p = (LED_PARAM*)param;

    for(;;) {
        DoLoop();
        //digitalWrite(p->pin, HIGH);
        //delay(p->delay);
        //digitalWrite(p->pin, LOW);
        //delay(p->delay);
    }
}

os_thread_return_t Loop2(void* param){
    LED_PARAM *p = (LED_PARAM*)param;
    int mySendTS;

    for(;;) {
      mySendTS = SendTS;
      //Serial.print("SendTS ");
      //Serial.println(SendTS);
      //Particle.publish("SendTS Loop 2", String(SendTS));
      digitalWrite(D3, mySendTS);
      SendToService(mySendTS);
      //As = !As;
      //digitalWrite(D3, SendTS);
      //delay(1000);
      //digitalWrite(p->pin, HIGH);
      //delay(p->delay);
      //digitalWrite(p->pin, LOW);
      //delay(p->delay);
    }
}

os_thread_return_t Loop3(void* param){
    LED_PARAM *p = (LED_PARAM*)param;

    for(;;) {
      flashLED();
    }
}


void DoLoop() {

  Serial.print("Version ");
  Serial.println(Version);

  sw = digitalRead(D0);

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
  LoopN = LoopN + 1;

  //Serial.println("Loop");
  //Serial.println(LoopN);



  digitalWrite(D1, Switch1[sw]);
  digitalWrite(D2, Switch2[sw]);

  //Serial.print("Switch Before ");
  //Serial.print(String(Psw));
  //Serial.print("  Switch Now ");
  //Serial.println(String(sw));

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
  //Serial.print(String(Diff));
  //Serial.println(" Seconds");

  if (SendTSDone) {
    if (Diff < MinS) {
      SendTS = false;
      //Serial.print(String(MinS));
      //Serial.println(" too soon to send to Internet");
    } else {
      if ( ReadyToSend ) {
        SendTS = true;
        ReadyToSend = false;
        //Serial.println("Send this loop");
      } else {
        SendTS = false;
        //Serial.println("Nothing to send this loop");
      }
    }
  }

  //Serial.print("SendTS ");
  //Serial.println(String(SendTS));



  //Particle.publish("Click", Click);
  //Particle.publish("Seconds", String(Diff));
  //if ( SendTS ) {
      //ThingSpeak.setField(1,Click);
  //}

  SVolume = String(Volume);
  //Serial.print("Volume ");
  //Serial.println(SVolume);



  //SendToService(SendTS);

  Psw = sw;




  //Stat=LOW;
  //digitalWrite(D7, Stat);

  StatR = StatR + 10*Dir;

  if ( StatR > 255 ) {
    StatR = 255;
    Dir = -1;
  }
  if ( StatR < 0 ) {
    StatR = 0;
    Dir = +1;
  }
  //analogWrite(D7, StatR);
  //Serial.print("D7 LED ");
  //Serial.println(StatR);
  //delay(100);

}

void SendToService(bool mySendTS) {

  if ( mySendTS ) {

    Particle.publish("SendTS SendToService", String(mySendTS));
    //Serial.print("Contacting Publish ");
    //Serial.println(String(millis()));

    //Serial.print("Contacted Publish ");
    //Serial.println(String(millis()));

    //Serial.print("Contacting ThingSpeak " );
    //Serial.println(String(millis()));
    ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
    //Serial.print("Contacted ThingSpeak ");
    //Serial.println(String(millis()));
    pnow = now;

    delay(10);
    Particle.publish("SendTS SendToService End", String(mySendTS));
    SendTSDone = true;
  }
}

//void MessageS(const char* Line){
//    Serial.println(Line);
//}
