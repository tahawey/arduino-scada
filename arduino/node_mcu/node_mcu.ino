#include<SoftwareSerial.h>
SoftwareSerial mySUART(D2, D4);  //D2, D1
int ready_pin = D7;int clockPin = D1;int dataPin = D8;
 byte x = 0b10000000;
String tt="00000000";String old_tt="00000000";
int interval=3000;unsigned long elasped=0;int time_out=3000;String message,old_message; String s="";String reg_temp[8];
bool moh[60];int lengthh=0;
char outs[8]={'0','0','0','0','0','0','0','0'};
#include <Arduino.h>
#include "fs.h"
#include <ESP8266WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

byte LED_PIN=D0;byte BTN_PIN= 22;byte HTTP_PORT=80;
const uint8_t DEBOUNCE_DELAY = 10; 
String reg[11];
String tag1;
String myarr[11];
String ss;
String inputs;
String old_inputs;

const char *WIFI_SSID      = "ssid";const char *WIFI_PASS= "password";

struct Led {
    uint8_t pin;
    bool    on;
    void update() {
        digitalWrite(pin, on ? HIGH : LOW);
    }
};


struct Button {

    uint8_t  pin;
    bool     lastReading;
    uint32_t lastDebounceTime;
    uint16_t state;


    bool pressed()                { return state == 1; }
    bool released()               { return state == 0xffff; }
    bool held(uint16_t count = 0) { return state > 1 + count && state < 0xffff; }

   
    void read() {

        bool reading = digitalRead(pin);
        if (reading != lastReading) { lastDebounceTime = millis(); }
        if (millis() - lastDebounceTime > DEBOUNCE_DELAY) {
            bool pressed = reading == LOW;
            if (pressed) {
                     if (state  < 0xfffe) state++;
                else if (state == 0xfffe) state = 2;
            } else if (state) {
                state = state == 0xffff ? 0 : 0xffff;
            }
        }

        lastReading = reading;
    }
};

Led    onboard_led = { LED_BUILTIN, false };
Led    led         = { LED_PIN, false };
Button button      = { BTN_PIN, HIGH, 0, 0 };

AsyncWebServer server(HTTP_PORT);AsyncWebSocket ws("/ws");
void initSPIFFS() {  if (!SPIFFS.begin()) {    Serial.println("Cannot mount SPIFFS volume...");   while (1) { onboard_led.on = millis() % 200 < 50; onboard_led.update(); }}}
void initWiFi() {
  WiFi.mode(WIFI_STA); IPAddress subnet(255, 255, 255, 0);	IPAddress gateway(192, 168, 1, 1);	IPAddress local_IP(192, 168, 1, 38);
  //if (WiFi.config(local_IP, gateway, gateway, subnet)) { Serial.println("Static IP Configured");  }  else { Serial.println("Static IP Configuration Failed");  }
  
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) { Serial.println(".");delay(500);  }
  Serial.printf(" %s\n", WiFi.localIP().toString().c_str());
}
String processor(const String &var) {    return String(var == "STATE" && led.on ? "on" : "off");}
void onRootRequest(AsyncWebServerRequest *request) {    request->send(SPIFFS, "/index.html");}
void initWebServer() {    server.on("/", onRootRequest);    server.serveStatic("/", SPIFFS, "/");    server.begin();}
void notifyClients() {
    const uint8_t size = JSON_OBJECT_SIZE(15);
    StaticJsonDocument<size> json;String s;
    json["status"] = led.on ? "on" : "off";
    for(int i=1;i<10;i++){json["reg"+String(i)] =myarr[i-1]; }
    char data[200];
    size_t len = serializeJson(json, data);
    ws.textAll(data, len);
     

    // json["reg"]=1;json["reg2"]=2;json["reg3"]=3;   char data2[200];    size_t len2 = serializeJson(json, data2);    ws.textAll(data2, len2);
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
    AwsFrameInfo *info = (AwsFrameInfo*)arg;
    if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {

        const uint8_t size = JSON_OBJECT_SIZE(1);
        StaticJsonDocument<size> json;
        DeserializationError err = deserializeJson(json, data);
        if (err) {Serial.print(F("deserializeJson() failed with code ")); Serial.println(err.c_str());return;}

        const char *action = json["action"];

        if (strcmp(action, "toggle") == 0) {led.on = !led.on;notifyClients();}
        int t=0;String s=action;
        int l=s.length();
        if (l>0) { t=s.substring(l, l-1).toInt();
        
        if (action[0]=='o' && action[1]=='n') { outs[t-1]='1';bitWrite(x, t-1, 1);}
        if (action[0]=='o' && action[1]=='f' && action[2]=='f'){outs[t-1]='0';bitWrite(x, t-1, 0);}
        }
         Serial.println(action);
         Serial.println(outs);
            for (int i=0;i<9;i++)tt[i]=outs[7-i];
            Serial.println (tt);
            int tt_int=bintoint(tt);
            Serial.println (tt_int);
            Serial.println (x);
            Serial.println (x,BIN);
     
            //send_ser(x);
            send_tah(x);            
    }
}

void onEvent(AsyncWebSocket       *server,
             AsyncWebSocketClient *client,
             AwsEventType          type,
             void                 *arg,
             uint8_t              *data,
             size_t                len) {

    switch (type) {case WS_EVT_CONNECT:break;case WS_EVT_DISCONNECT:break;
        case WS_EVT_DATA:handleWebSocketMessage(arg, data, len);    break;
        case WS_EVT_PONG:
        case WS_EVT_ERROR:break;
    }
}

void initWebSocket() {    ws.onEvent(onEvent);    server.addHandler(&ws);}


void(* resetFunc) (void) = 0;

void send_ser(byte data) {
   // digitalWrite(latchPin, HIGH);
   // shiftOut(dataPin, clockPin, LSBFIRST, data);
   // digitalWrite(latchPin, LOW);
}
void send_tah(byte data){
digitalWrite(ready_pin,HIGH);
digitalWrite(clockPin,HIGH);delay(100);
for (int i=0;i<8;i++){digitalWrite(dataPin,bitRead(data,i) ? HIGH : LOW);delay(50);digitalWrite(clockPin,LOW);delay(50);digitalWrite(clockPin,HIGH);}
}
void setup() {
  pinMode(ready_pin, OUTPUT);    pinMode(clockPin, OUTPUT);  pinMode(dataPin, OUTPUT);  
  digitalWrite(clockPin,LOW);digitalWrite(ready_pin,LOW);

    pinMode(onboard_led.pin, OUTPUT);
    pinMode(led.pin,         OUTPUT);
    pinMode(button.pin,      INPUT);

    Serial.begin(115200); delay(2000);mySUART.begin(19200);
   
    initSPIFFS();
    initWiFi();
    initWebSocket();
    initWebServer();
     //mySUART.println("status,Node-MCU restarted");
}

String inttonbin(byte num){  int data = num;char strn[33];itoa(data, strn, 2); String str1=strn;int m=8-str1.length();for (int i=1;i<m;i++){  str1="0"+str1;}return str1;}
byte bintoint(String num){ int m=num.length(); char s[m] ; for (int i=0;i<m;i++){  s[i]=num[i];}int value = 0;for (int i=0; i< strlen(s); i++){  value *= 2;  if (s[i] == '1') value++; }return value;}
void send_message(){
 
    //  String s;      s="outputs,";      s+="127";      s+= mySUART.println(s);
      }
void send_outputs(){
  // mySUART.println("outputs,11,22,33,44,55,66,77,88,"+millis());

String url ="" ;
url+="%20ali";
    WiFiClientSecure client;client.setInsecure();
    HTTPClient https;
    if (https.begin(client, url)) {
      int httpCode = https.GET();
     // Serial.println("============== Response code: " + String(httpCode));if (httpCode > 0) {Serial.println(https.getString());}https.end();} else {Serial.printf("[HTTPS] Unable to connect\n");}
    }
}

bool chk_message(){
message="";
 if (mySUART.available() )  { message=mySUART.readStringUntil('\n');  }
 //if (message!="\n" && message!=old_message) {if (validate(message)==true) {send_message(message);message+=",vaild";}/*else message+=",not valid";*/ }/*Serial.println(message);*/ old_message=message; 
 if ( validate(message)==true ) {send_message(message);Serial.println(message);old_message=message;return true;}else return false;
 }
void send_message(String mess){
 s="outputs,"; //uno data to node-mcu
 //for (int i=1;i<9;i++){s+=String(random(127))+",";} 
 mess+=",";
 mess+=String(millis()/1000);
 tag1=mess;
 split(mess);
 //mySUART.println(split(mess)); 
 send_message();
}
bool  chek_char_valid(char str){  bool v=true;int n;  n=int(str);   if (n<44  )  v=false ;  if (n>57 && n<65) v=false ;  if (n>90 && n<97) v=false ;  if (n>123) v=false ;  return v;}
void loop() {
  
 // if(mySUART.available()>0)  {   char h=(char)mySUART.read();    Serial.write(h);   if (h!='\n' && chek_char_valid(h)) inputs+=h;  else {tag1=inputs;Serial.println(tag1);}}
    ws.cleanupClients();
    if (chk_message()==true) {led.on = !led.on; notifyClients();}
 if  (millis() > (interval+ elasped)  ){
   if (check_change()==true) {led.on = !led.on; notifyClients();} 
   //send_outputs();
   send_message();
  elasped=millis();
}  
    onboard_led.on = millis() % 1000 < 50;
    led.update();
    onboard_led.update();
}
void copy_inputs(){
  int count=0; for ( int i=0;i<old_message.length();i++){ if (old_message[i]==',') count++;  if (count==6 ) lengthh=i; }
}
bool check_change(){
if (inputs!=old_inputs){ 
if (validate(inputs)==true){split(tag1);old_inputs=inputs;

return true;}else inputs="";} 
}

bool validate(String message){
  int count=0; for ( int i=0;i<message.length();i++){ if (message[i]==',' )count++; }
   if (message.substring(0, 7)=="inputs," && count==8) return true;else return false;  
}
void split(String sstr){ int sizee=100;char *sPtr [sizee];int N = separate (sstr, sPtr, sizee);      for (int n = 0; n < N; n++){myarr[n]=sPtr [n];} }
int separate ( String str,char **p,int size ){  int  n;char s [100]; strcpy (s, str.c_str ());*p++ = strtok (s, ",");  for (n = 1; NULL != (*p++ = strtok (NULL, ",")); n++) if (size == n)  break;return n;}
void toggle(int pin){
  digitalWrite(pin,!digitalRead(pin));
}
