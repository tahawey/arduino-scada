int ready_pin = 5;int dataPin = 6;int clockPin = 2;unsigned long elaspedd=0;int time_out=1000; 
int moh[]={14,16,18,20,21,15,17,19};byte ret = 0x00;
#include "dht.h"
dht DHT; 
#include<SoftwareSerial.h>
#include "LedControl.h"
//LedControl lc=LedControl(DATAIN,CLK,CS);
LedControl lc=LedControl(6,10,8);
SoftwareSerial mySUART(5, 3); //rx,tx
float tt,old_tt;
bool clk=false; 
String message,old_message;
bool sts=true;
int interval=1000;unsigned long elasped=0;
bool laststate;int p_from=22;int p_to=49;
int input[]={22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,14,15,16,17,18,19,20};
//int output[]={14,15,16,17,18,19,20};
int out_order[]={2,4,5};
int bt;int offset=p_from;byte data[7];byte data2[7];
String temp[2];String temp2[2];
String get_temp(int vd_power_pin,int ntc_pin){
int nominal_resistance=10000 ;int nominal_temeprature=25;int samplingrate=5;int beta=3950 ;int Rref=10000;  
  float average;  float temperature; int  samples = 0;
  pinMode(vd_power_pin, OUTPUT);
  digitalWrite(vd_power_pin, HIGH);
  for (uint8_t i = 0; i < samplingrate; i++) { samples += analogRead(ntc_pin); /*delay(10);*/}
  digitalWrite(vd_power_pin, LOW);  average = 0;  average = samples / samplingrate;  average = 1023 / average - 1;  average = Rref / average;  temperature = average / nominal_resistance;  temperature = log(temperature);  temperature /= beta; temperature += 1.0 / (nominal_temeprature + 273.15);  temperature = 1.0 / temperature;  temperature -= 273.15;                        
  return String(temperature) ;
}
float temp_h(int pin=A1){  DHT.read11(pin);  return DHT.temperature;}
float hum(int pin=A1){  DHT.read11(pin);  return DHT.humidity;}
void start_up(){
  int m[]={0,1,3,7,15,31,63,127,255};
  for (int j=0 ;j<9;j++){for (int i=0 ;i<9;i++){lc.setRow(0,j,m[i]);delay(30);}}
  for (int j=0 ;j<8;j++){for (int i=0 ;i<8;i++){lc.setRow(0,7-j,m[7-i]);delay(30);}}
  }
bool validate(String message){
  int count=0; for ( int i=0;i<message.length();i++){ if (message[i]==',' ){count++;}}
   if ((message.substring(0, 8)=="outputs,") && (count==9)) return true;else return false;  
}

bool chk_message(){
message="";
 
 if (mySUART.available() )  { message=mySUART.readStringUntil('\n'); Serial.println(mySUART.readStringUntil('\n')); }
 //if (message!="\n" && message!=old_message) {if (validate(message)==true) {send_message(message);message+=",vaild";}/*else message+=",not valid";*/ }/*Serial.println(message);*/ old_message=message; 
 if ( validate(message)==true ) {Serial.println(message);old_message=message;return true;}else return false;
 }
 void toggle_sts(){   sts=!sts;  lc.setRow(7,0,sts);  }
void send_message(){
      String s;
      s="inputs,";
      for(int j=0;j<5;j++ ){s+=data[j];s+=',';}
      s+=temp[0]+','+temp[1]+',';
      s+=String(millis()/1000); mySUART.println(s); //s="";
      }
void Get_points()
{
  //temp[0]=get_temp(12,A0);temp[1]=get_temp(11,A1);  
 // temp[0]=temp_h(A3);temp[1]=hum(A3);  
 temp[0]=get_temp(12,A0);temp[1]=hum(A3);  
  int po=0;
  for (int i=0;i<5;i++)
  { 
    for(int j=0;j<7;j++){bitWrite(data[i], j,read_pin(input[po]));po++;}}
    }
void data_copy(){lc.clearDisplay(0);for (int i=0;i<5;i++) {data2[i]=data[i];lc.setRow(0,i,data[i]); }temp2[0]=temp[0];temp2[1]=temp[1]; }
byte read_pin(byte pin){ if (digitalRead(pin)==HIGH) return 0 ;else return 1;}
void check_point(){   if (data[0]!=data2[0] || data[1]!=data2[1] || data[2]!=data2[2] || data[3]!=data2[3] || data[4]!=data2[4] ){send_message();data_copy();print_data(); }}




void print_data(){
  for (int i=0;i<5;i++){Serial.print(i+1);Serial.print("  ");Serial.print(data[i],BIN);Serial.print("  ");Serial.println(data[i]); }
}

  
  
void setup() {
   pinMode(ready_pin, INPUT); pinMode(clockPin, INPUT);  pinMode(dataPin, INPUT);
   for ( int i=14;i<22;i++){pinMode(i,OUTPUT);digitalWrite(i, LOW);}
lc.shutdown(0,false);   lc.setIntensity(0,3);  lc.clearDisplay(0);
 Serial.begin(115200);
  for (int i=p_from;i<p_to+1;i++){pinMode(i,INPUT_PULLUP);}
 // for ( int i=14;i<21;i++){pinMode(i,OUTPUT);digitalWrite(i, HIGH);}
//2,4,5
pinMode(out_order[0],INPUT_PULLUP);pinMode(out_order[1],INPUT_PULLUP);pinMode(out_order[2],INPUT_PULLUP);
//  start_up();
  //lc.clearDisplay(0);
  mySUART.begin(19200);
 }
void chk_outs(){
  float duration_us=0;
  //out_order 4 signal,5 Enable
  while(digitalRead(out_order[0])==LOW ){
    duration_us++;
  }
  tt=duration_us;
  if(duration_us>0){Serial.println("pulses "+String(tt));old_tt=tt;}
}

void loop() {
  //check_outs();
  //if (millis()<1000)Serial.println(millis());
  //chk_outs();
  if (mySUART.available() )  {Serial.println(mySUART.read());}
//  for (int i=0;i<3;i++){if (digitalRead(out_order[i])==HIGH)digitalWrite(output[i],HIGH);else digitalWrite(output[i],LOW);}
  if (chk_message()==true){lc.setRow(7,1,!sts);}else {lc.setRow(7,1,sts);}
 
   if  (millis() > (interval+ elasped)  ){
    toggle_sts();
    send_message();
 //   chk_message();
    elasped=millis();
  
   }
    Get_points(); 
 check_point();
  }
  void check(){
  elasped=millis();
   if (digitalRead( ready_pin)== HIGH && millis() < (time_out+ elasped) ) {
    Serial.println("data_begin");
    byte t=read_tah();
    Serial.println(t);
    Serial.println(t,BIN);
    write_data();
    String t2=""; for (int i=0;i<8;i++){t2+=bitRead(t,i)? "1":"0";}Serial.println(t2);  }
}
byte read_tah(){

for( int i=7; i>=0; i--){while (digitalRead( clockPin)== HIGH){}if (digitalRead( dataPin) == HIGH)bitWrite( ret, i,1);else bitWrite( ret, i,0);while (digitalRead( clockPin)== LOW){}}return( ret);}

void write_data(){
  for( int i=1; i<7; i++)digitalWrite( moh[i],bitRead(ret, i));
}






 
