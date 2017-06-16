#include <Arduino.h>
#include <ArduinoJson.h>
#include <RTClib.h>
#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include <SoftwareSerial.h>
#define LENG 31   //0x42 + 31 bytes equal to 32 bytes

unsigned char buf[LENG];
#define SSID "C414";
#define password "0874006333";

String Str_data = "";

File myFile;
const int chipSelect = 53; //4 in UNO if Mega chage pls
unsigned long previousMillis = 0;
const long interval = 5000;

int PM01Value[3];          //define PM1.0 value of the air detector module
int PM2_5Value[3];         //define PM2.5 value of the air detector module
int PM10Value[3];         //define PM10 value of the air detector module

void setup()
{
  Serial.begin(9600);   //use serial0
  Serial1.begin(9600);
  Serial1.setTimeout(1500);
  Serial2.begin(9600);
  Serial2.setTimeout(1500);
  Serial3.begin(9600);

  Serial.println("AT");
   delay(7000);
   if (Serial.find("OK")) 
    {
    Serial.println("Device OK");
    connectWifi();
    delay(1000); 
    }
    else 
    { 
      Serial.println("!OK");
    }
  Serial.println("");
  Serial.print("Initializing SD card...");
  if (!SD.begin(chipSelect)) {
    Serial.println("initialization failed!");
  }else Serial.println("initialization done.");
}

void loop()
{
  Dustsensor();
  writesd();
  PrintDust();
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    createJson();
  }  
 // senddata(); 
  delay(1000);
}

void createJson(){
  StaticJsonBuffer<300> JSONbuffer;
  JsonObject& JSONencoder = JSONbuffer.createObject();
  JsonArray& PM1A = JSONencoder.createNestedArray("PM1A");
  JsonArray& PM25A = JSONencoder.createNestedArray("PM25A");
  JsonArray& PM10A = JSONencoder.createNestedArray("PM10A");
  JsonArray& PM1B = JSONencoder.createNestedArray("PM1B");
  JsonArray& PM25B = JSONencoder.createNestedArray("PM25B");
  JsonArray& PM10B = JSONencoder.createNestedArray("PM10B");
  JsonArray& PM1C = JSONencoder.createNestedArray("PM1C");
  JsonArray& PM25C = JSONencoder.createNestedArray("PM25C");
  JsonArray& PM10C = JSONencoder.createNestedArray("PM10C");
  PM1A.add(PM01Value[0]);
  PM25A.add(PM2_5Value[0]);
  PM10A.add(PM10Value[0]);
  PM1B.add(PM01Value[1]);
  PM25B.add(PM2_5Value[1]);
  PM10B.add(PM10Value[1]);
  PM1C.add(PM01Value[2]);
  PM25C.add(PM2_5Value[2]);
  PM10C.add(PM10Value[2]);
  JSONencoder.prettyPrintTo(Serial);
}

void connectWifi()
{
  Serial.println("AT+CWMODE=1");
  delay(2000);
  String cmd = "AT+CWJAP=\"";
  cmd += SSID;
  cmd += "\",\"";
  cmd += password;
  cmd += "\"";
  Serial.println(cmd);
  delay(5000);
  if (Serial.find("OK")) {
    digitalWrite(8, HIGH);
    Serial.print("OK");
    delay(1000);
  } else {
    digitalWrite(8, LOW);
    Serial.print("FAIL");
    //return false;
  }
}

void writesd(){
  myFile = SD.open("DataDust.txt", FILE_WRITE);
  if (myFile) {
    Serial.print("Writing to DataDust.txt...");
    myFile.print("1.0(1) ");
    myFile.print(PM01Value[0]);
    myFile.print("\t");
      
    myFile.print("2.5(1) ");
    myFile.print(PM2_5Value[0]);
    myFile.print("\t");

    myFile.print("10(1) ");
    myFile.print(PM10Value[0]);
    myFile.println("\t");
    
    myFile.print("1.0(2) ");
    myFile.print(PM01Value[1]);
    myFile.print("\t");
      
    myFile.print("2.5(2) ");
    myFile.print(PM2_5Value[1]);
    myFile.print("\t");

    myFile.print("10(2) ");
    myFile.print(PM10Value[1]);
    myFile.println("\t");
    
    myFile.print("1.0(3) ");
    myFile.print(PM01Value[2]);
    myFile.print("\t");
      
    myFile.print("2.5(3) ");
    myFile.print(PM2_5Value[2]);
    myFile.print("\t");

    myFile.print("10(3) ");
    myFile.print(PM10Value[2]);
    myFile.println("\t");
    myFile.close(); 
    Serial.println("done.");
  } else {
    Serial.println();
    Serial.println("error opening DataDust.txt");
  }
}

void senddata()
{
  String con = "AT+CIPSTART=\"TCP\",\"";
  con += "202.28.24.69";
  con += "\",80";
  Serial.println(con);
   
  if(Serial.find("Error")){
    Serial.println("AT+CIPSTART error");
    return;
  }
  
  // prepare GET string
  //getStr += apiKey;
  /*Str_data +="GET /~oasys5/d.php?";
  Str_data +="T="+String(temp_act)+"&";
  Str_data +="H="+String(hum_act)+"&";
  Str_data +="P="+String(press_act)+"&";
  Str_data +="p1="+String(PM2_5Value)+"&";
  Str_data +="p2="+String(PM10Value)+"&";
  Str_data +="p3="+String(PM01Value);
  Str_data += "\r\n\r\n";*/
  // send data length
  con = "AT+CIPSEND=";
  con += String(Str_data.length());
  Serial.println(con);
  Serial.print(Str_data);

  if(Serial.find(">")){
    //Serial.print(getStr);
  }
  else{
    Serial.println("AT+CIPCLOSE");
  }
}

void read(){
  myFile = SD.open("DataDust.txt"); 
  if (myFile) {
    Serial.println("DataDust.txt:");
    while (myFile.available()) {
    Serial.write(myFile.read());
    }
    myFile.close(); 
  } else {
    Serial.println("error opening DataDust.txt");
  }
}

void Dustsensor(){
if(Serial3.find(0x42)){    //start to read when detect 0x42
    Serial3.readBytes(buf,LENG);

    if(buf[0] == 0x4d){
      if(checkValue(buf,LENG)){
        PM01Value[0]=transmitPM01(buf); //count PM1.0 value of the air detector module
        PM2_5Value[0]=transmitPM2_5(buf);//count PM2.5 value of the air detector module
        PM10Value[0]=transmitPM10(buf); //count PM10 value of the air detector module 
      }           
     }
    } 
    if(Serial1.find(0x42)){    //start to read when detect 0x42
    Serial1.readBytes(buf,LENG);

    if(buf[0] == 0x4d){
      if(checkValue(buf,LENG)){
        PM01Value[1]=transmitPM01(buf); //count PM1.0 value of the air detector module
        PM2_5Value[1]=transmitPM2_5(buf);//count PM2.5 value of the air detector module
        PM10Value[1]=transmitPM10(buf); //count PM10 value of the air detector module 
      }           
     }
    } 
    if(Serial2.find(0x42)){    //start to read when detect 0x42
    Serial2.readBytes(buf,LENG);

    if(buf[0] == 0x4d){
      if(checkValue(buf,LENG)){
        PM01Value[2]=transmitPM01(buf); //count PM1.0 value of the air detector module
        PM2_5Value[2]=transmitPM2_5(buf);//count PM2.5 value of the air detector module
        PM10Value[2]=transmitPM10(buf); //count PM10 value of the air detector module 
      }           
    } 
  }
}

void PrintDust(){
  static unsigned long OledTimer=millis();  
    if (millis() - OledTimer >=1000) 
    {
      OledTimer=millis(); 
      
      Serial.print("PM1.0D1: ");  
      Serial.print(PM01Value[0]);
      Serial.println("  ug/m3");            
    
      Serial.print("PM2.5D1: ");  
      Serial.print(PM2_5Value[0]);
      Serial.println("  ug/m3");     
      
      Serial.print("PM1 0D1: ");  
      Serial.print(PM10Value[0]);
      Serial.println("  ug/m3");   
      Serial.println("*******************");

      Serial.print("PM1.0D2: ");  
      Serial.print(PM01Value[1]);
      Serial.println("  ug/m3");            
    
      Serial.print("PM2.5D2: ");  
      Serial.print(PM2_5Value[1]);
      Serial.println("  ug/m3");     
      
      Serial.print("PM1 0D2: ");  
      Serial.print(PM10Value[1]);
      Serial.println("  ug/m3");   
      Serial.println("*******************");

      Serial.print("PM1.0D3: ");  
      Serial.print(PM01Value[2]);
      Serial.println("  ug/m3");            
    
      Serial.print("PM2.5D3: ");  
      Serial.print(PM2_5Value[2]);
      Serial.println("  ug/m3");     
      
      Serial.print("PM10D3: ");  
      Serial.print(PM10Value[2]);
      Serial.println("  ug/m3");   
      Serial.println("*******************");
    }
}

char checkValue(unsigned char *thebuf, char leng)
{  
  char receiveflag=0;
  int receiveSum=0;

  for(int i=0; i<(leng-2); i++){
  receiveSum=receiveSum+thebuf[i];
  }
  receiveSum=receiveSum + 0x42;
 
  if(receiveSum == ((thebuf[leng-2]<<8)+thebuf[leng-1]))  //check the serial data 
  {
    receiveSum = 0;
    receiveflag = 1;
  }
  return receiveflag;
}

int transmitPM01(unsigned char *thebuf)
{
  int PM01Val;
  PM01Val=((thebuf[3]<<8) + thebuf[4]); //count PM1.0 value of the air detector module
  return PM01Val;
}

//transmit PM Value to PC
int transmitPM2_5(unsigned char *thebuf)
{
  int PM2_5Val;
  PM2_5Val=((thebuf[5]<<8) + thebuf[6]);//count PM2.5 value of the air detector module
  return PM2_5Val;
  }

//transmit PM Value to PC
int transmitPM10(unsigned char *thebuf)
{
  int PM10Val;
  PM10Val=((thebuf[7]<<8) + thebuf[8]); //count PM10 value of the air detector module  
  return PM10Val;
}


