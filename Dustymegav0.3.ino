#include <Arduino.h>
#include <ArduinoJson.h>
#include <DS3231_Simple.h>
#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include <SoftwareSerial.h>
#include "Adafruit_GFX.h"
#include "Adafruit_SH1106.h"

#define LENG 31   //0x42 + 31 bytes equal to 32 bytes
#define BME280_ADDRESS 0x76
#define TRUE 1
#define FLASE 0
#define OLED_RESET 4

Adafruit_SH1106 display(OLED_RESET); 
DS3231_Simple Clock;

int sen1 = 6;
int sen2 = 7;
int sen3 = 8;
int led1 = 10;
int led2 = 11;
int led3 = 12;
unsigned char buf[LENG];
unsigned long int hum_raw,temp_raw,pres_raw;
signed long int t_fine;

#define SSID "Fablab";
#define password "ilovecpe";

String Str_data = "";

uint16_t dig_T1;
 int16_t dig_T2;
 int16_t dig_T3;
uint16_t dig_P1;
 int16_t dig_P2;
 int16_t dig_P3;
 int16_t dig_P4;
 int16_t dig_P5;
 int16_t dig_P6;
 int16_t dig_P7;
 int16_t dig_P8;
 int16_t dig_P9;
 int8_t  dig_H1;
 int16_t dig_H2;
 int8_t  dig_H3;
 int16_t dig_H4;
 int16_t dig_H5;
 int8_t  dig_H6;


File myFile;
const int chipSelect = 53; //4 in UNO if Mega change pls
unsigned long previousMillis = 0;
const long interval = 900000;
double temp_act = 0.0, press_act = 0.0,hum_act=0.0;

int PM01Value[3];          //define PM1.0 value of the air detector module
int PM2_5Value[3];         //define PM2.5 value of the air detector module
int PM10Value[3];         //define PM10 value of the air detector module

void setup()
{
  uint8_t osrs_t = 1;             //Temperature oversampling x 1
  uint8_t osrs_p = 1;             //Pressure oversampling x 1
  uint8_t osrs_h = 1;             //Humidity oversampling x 1
  uint8_t mode = 3;               //Normal mode
  uint8_t t_sb = 5;               //Tstandby 1000ms
  uint8_t filter = 0;             //Filter off 
  uint8_t spi3w_en = 0;           //3-wire SPI Disable
    
  uint8_t ctrl_meas_reg = (osrs_t << 5) | (osrs_p << 2) | mode;
  uint8_t config_reg    = (t_sb << 5) | (filter << 2) | spi3w_en;
  uint8_t ctrl_hum_reg  = osrs_h;
  Serial.begin(9600);   //use serial0
  Serial1.begin(9600);
  Serial1.setTimeout(1500);
  Serial2.begin(9600);
  Serial2.setTimeout(1500);
  Serial3.begin(9600);
  display.begin(SH1106_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.display();
  display.setTextColor(WHITE);
  display.setCursor(0, 5);
  pinMode(sen1,OUTPUT);
  pinMode(sen2,OUTPUT);
  pinMode(sen3,OUTPUT);
 // pinMode(led1,OUTPUT);
  //pinMode(led2,OUTPUT);
  //pinMode(led3,OUTPUT);
  Clock.begin(); 
  Wire.begin();

  display.setTextSize(2);
  display.println("Dusty");
  display.println("Wake Up..");
  display.display();
  delay(2000);
  Serial.println("AT");
   display.clearDisplay();
   display.setCursor(0, 5);
   display.setTextSize(2);
   display.println("Conn WIFI");
   display.display();
   delay(7000);
   if (Serial.find("OK")) 
    {
    Serial.println("Device OK");
    display.clearDisplay();
    display.setCursor(0, 5);
    display.setTextSize(2);
    display.println("Device OK");
    display.display();
    connectWifi();
    delay(1000); 
    }
    else 
    { 
      Serial.println("!OK");
      display.clearDisplay();
      display.setCursor(0, 5);
      display.setTextSize(2);
      display.println("!OK");
      display.display();
      delay(1000);
      digitalWrite(led1,HIGH);
    }
  Serial.println("");
  Serial.print("Initializing SD card...");
  if (!SD.begin(chipSelect)) {
    Serial.println("initialization failed!");
    display.clearDisplay();
    display.setCursor(0, 5);
    display.setTextSize(2);
    display.println("SD CARD");
    display.println("failed!");
    display.display();
    digitalWrite(led2,HIGH);
  }else {
    Serial.println("initialization done.");
    display.clearDisplay();
    display.setCursor(0, 5);
    display.setTextSize(2);
    display.println("SD CARD");
    display.println("Done.");
    display.display();
  }

  
  Serial.println("DS3231 Test.");
  Serial.println("BME280 Test.");
  delay(1000);
  for(int i = 0; i < 80; i++)
  {
    Serial.print("-");
    delay(50);
  }
    Serial.println();
        
    writeReg(0xF2,ctrl_hum_reg);
    writeReg(0xF4,ctrl_meas_reg);
    writeReg(0xF5,config_reg);
    readTrim();  
}

void loop()
{
  signed long int temp_cal;
  unsigned long int press_cal,hum_cal;
  readData();
  DateTime MyDateAndTime;
  
  MyDateAndTime = Clock.read();
  display.clearDisplay();
  if((MyDateAndTime.Minute >= 00) && (MyDateAndTime.Minute <= 60) ){ 
    display.setCursor(0, 5);
    display.setTextSize(2);
    display.println("Read Data");
    display.display();
    digitalWrite(sen1,HIGH);
    digitalWrite(sen2,HIGH);
    digitalWrite(sen3,HIGH);
    digitalWrite(led3,HIGH);
    Dustsensor();  
    writesd();
    PrintDust();
  temp_cal = calibration_T(temp_raw);
  press_cal = calibration_P(pres_raw);
  hum_cal = calibration_H(hum_raw);
  temp_act = (double)temp_cal / 100.0;
  press_act = (double)press_cal / 100.0;
  hum_act = (double)hum_cal / 1024.0;
  Serial.print("TEMP : ");
  Serial.print(temp_act);
  Serial.print(" DegC  PRESS : ");
  Serial.print(press_act);
  Serial.print(" hPa  HUM : ");
  Serial.print(hum_act);
  Serial.println(" %");    
  Serial.println("--------------------------------------------------------------");
  delay(1000);

  Serial.print("Hour: "); Serial.print("Minute: ");  Serial.println("Second: ");  
  Serial.print(MyDateAndTime.Hour);
  Serial.print(":");
  Serial.print(MyDateAndTime.Minute);
  Serial.print(":");
  Serial.println(MyDateAndTime.Second);
  Serial.print("Day: ");  Serial.print("Month: ");Serial.println("Year: ");  
 
  Serial.print(MyDateAndTime.Day); Serial.print("/");Serial.print(MyDateAndTime.Month); Serial.print("/");Serial.println(MyDateAndTime.Year);
  
  Serial.println("-----------------------------------------------------------------");
  display.clearDisplay();
  ShowDisplay();
  delay(1000);
    
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    createJson();
  }  
 // senddata(); 
  } 
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
  JsonArray& temp = JSONencoder.createNestedArray("temp");
  JsonArray& pressa = JSONencoder.createNestedArray("pressa");
  JsonArray& hum = JSONencoder.createNestedArray("hum");
  PM1A.add(PM01Value[0]);
  PM25A.add(PM2_5Value[0]);
  PM10A.add(PM10Value[0]);
  PM1B.add(PM01Value[1]);
  PM25B.add(PM2_5Value[1]);
  PM10B.add(PM10Value[1]);
  PM1C.add(PM01Value[2]);
  PM25C.add(PM2_5Value[2]);
  PM10C.add(PM10Value[2]);
  temp.add(temp_act);
  pressa.add(press_act);
  hum.add(hum_act);
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

/*void senddata()
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
  Str_data +="GET /~oasys5/d.php?JSONencoder\r\n\r\n";
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
}*/

void ShowDisplay(){
  DateTime MyDateAndTime;
  MyDateAndTime = Clock.read();
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  static unsigned long OledTimer=millis();  
    if (millis() - OledTimer >=1000) 
    {
      OledTimer=millis(); 
      display.setCursor(0, 5);
      display.setTextSize(2);
      display.println("PM1.0");
      display.setTextSize(1);
      display.println("");
      display.print("D1 : ");  
      display.print(PM01Value[0]);
      display.println("  ug/m3");            
    
      display.print("D2 : ");  
      display.print(PM01Value[1]);
      display.println("  ug/m3");     
      
      display.print("D3 : ");  
      display.print(PM01Value[2]);
      display.println("  ug/m3");
      display.display();
      delay(2000);   
      display.clearDisplay();

      display.setCursor(0, 5);
      display.setTextSize(2);
      display.println("PM2.5");
      display.setTextSize(1);
      display.println("");
      display.print("D1 : ");  
      display.print(PM2_5Value[0]);
      display.println("  ug/m3");            
      
      display.print("D2 : ");  
      display.print(PM2_5Value[1]);
      display.println("  ug/m3");     
      
      display.print("D3 : ");  
      display.print(PM2_5Value[2]);
      display.println("  ug/m3");
      display.display();
      delay(2000);
      display.clearDisplay();   
      
      display.setCursor(0, 5);
      display.setTextSize(2);
      display.println("PM10");
      display.setTextSize(1);
      display.println("");
      display.print("D1 : ");  
      display.print(PM10Value[0]);
      display.println("  ug/m3");            
    
      display.print("D2 : ");  
      display.print(PM10Value[1]);
      display.println("  ug/m3");     
      
      display.print("D3 : ");  
      display.print(PM10Value[2]);
      display.println("  ug/m3");
      display.display();
      delay(2000);
      display.clearDisplay();

      display.setCursor(1, 6); 
      display.setTextSize(2);
      display.print("BME280");
      display.println("");
      display.setTextSize(1);
      display.println("");
      display.print("TEMP : ");
      display.print(temp_act);
      display.println(" DegC");
      display.print("PRESS : ");
      display.print(press_act);
      display.println(" hPa");
      display.print("Huminity : ");
      display.print(hum_act);
      display.println(" %");
      display.display();    
      delay(2000);
      display.clearDisplay();

      display.setCursor(1, 6);
      display.setTextSize(1);
      display.print("Hour: "); display.print("Minute: ");  display.println("Second: ");  
      display.print(MyDateAndTime.Hour);
      display.print("  :  ");
      display.print(MyDateAndTime.Minute);
      display.print("  :  ");
      display.println(MyDateAndTime.Second);
      display.println("");
      display.print("Day: ");  display.print("Month: ");display.println("Year: ");  
      display.println("");
      display.print(MyDateAndTime.Day); display.print(" / "); display.print(MyDateAndTime.Month); display.print(" / "); display.println(MyDateAndTime.Year);
      display.display();
      delay(2000);
      display.clearDisplay();
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

void readTrim()
{
    uint8_t data[32],i=0;
    Wire.beginTransmission(BME280_ADDRESS);
    Wire.write(0x88);
    Wire.endTransmission();
    Wire.requestFrom(BME280_ADDRESS,24);
    while(Wire.available()){
        data[i] = Wire.read();
        i++;
    }
    
    Wire.beginTransmission(BME280_ADDRESS);
    Wire.write(0xA1);
    Wire.endTransmission();
    Wire.requestFrom(BME280_ADDRESS,1);
    data[i] = Wire.read();
    i++;
    
    Wire.beginTransmission(BME280_ADDRESS);
    Wire.write(0xE1);
    Wire.endTransmission();
    Wire.requestFrom(BME280_ADDRESS,7);
    while(Wire.available()){
        data[i] = Wire.read();
        i++;    
    }
    dig_T1 = (data[1] << 8) | data[0];
    dig_T2 = (data[3] << 8) | data[2];
    dig_T3 = (data[5] << 8) | data[4];
    dig_P1 = (data[7] << 8) | data[6];
    dig_P2 = (data[9] << 8) | data[8];
    dig_P3 = (data[11]<< 8) | data[10];
    dig_P4 = (data[13]<< 8) | data[12];
    dig_P5 = (data[15]<< 8) | data[14];
    dig_P6 = (data[17]<< 8) | data[16];
    dig_P7 = (data[19]<< 8) | data[18];
    dig_P8 = (data[21]<< 8) | data[20];
    dig_P9 = (data[23]<< 8) | data[22];
    dig_H1 = data[24];
    dig_H2 = (data[26]<< 8) | data[25];
    dig_H3 = data[27];
    dig_H4 = (data[28]<< 4) | (0x0F & data[29]);
    dig_H5 = (data[30] << 4) | ((data[29] >> 4) & 0x0F);
    dig_H6 = data[31];   
}
void writeReg(uint8_t reg_address, uint8_t data)
{
    Wire.beginTransmission(BME280_ADDRESS);
    Wire.write(reg_address);
    Wire.write(data);
    Wire.endTransmission();    
}


void readData()
{
    int i = 0;
    uint32_t data[8];
    Wire.beginTransmission(BME280_ADDRESS);
    Wire.write(0xF7);
    Wire.endTransmission();
    Wire.requestFrom(BME280_ADDRESS,8);
    while(Wire.available()){
        data[i] = Wire.read();
        i++;
    }
    pres_raw = (data[0] << 12) | (data[1] << 4) | (data[2] >> 4);
    temp_raw = (data[3] << 12) | (data[4] << 4) | (data[5] >> 4);
    hum_raw  = (data[6] << 8) | data[7];
}


signed long int calibration_T(signed long int adc_T)
{
    
    signed long int var1, var2, T;
    var1 = ((((adc_T >> 3) - ((signed long int)dig_T1<<1))) * ((signed long int)dig_T2)) >> 11;
    var2 = (((((adc_T >> 4) - ((signed long int)dig_T1)) * ((adc_T>>4) - ((signed long int)dig_T1))) >> 12) * ((signed long int)dig_T3)) >> 14;
    
    t_fine = var1 + var2;
    T = (t_fine * 5 + 128) >> 8;
    return T; 
}

unsigned long int calibration_P(signed long int adc_P)
{
    signed long int var1, var2;
    unsigned long int P;
    var1 = (((signed long int)t_fine)>>1) - (signed long int)64000;
    var2 = (((var1>>2) * (var1>>2)) >> 11) * ((signed long int)dig_P6);
    var2 = var2 + ((var1*((signed long int)dig_P5))<<1);
    var2 = (var2>>2)+(((signed long int)dig_P4)<<16);
    var1 = (((dig_P3 * (((var1>>2)*(var1>>2)) >> 13)) >>3) + ((((signed long int)dig_P2) * var1)>>1))>>18;
    var1 = ((((32768+var1))*((signed long int)dig_P1))>>15);
    if (var1 == 0)
    {
        return 0;
    }    
    P = (((unsigned long int)(((signed long int)1048576)-adc_P)-(var2>>12)))*3125;
    if(P<0x80000000)
    {
       P = (P << 1) / ((unsigned long int) var1);   
    }
    else
    {
        P = (P / (unsigned long int)var1) * 2;    
    }
    var1 = (((signed long int)dig_P9) * ((signed long int)(((P>>3) * (P>>3))>>13)))>>12;
    var2 = (((signed long int)(P>>2)) * ((signed long int)dig_P8))>>13;
    P = (unsigned long int)((signed long int)P + ((var1 + var2 + dig_P7) >> 4));
    return P;
}

unsigned long int calibration_H(signed long int adc_H)
{
    signed long int v_x1;
    
    v_x1 = (t_fine - ((signed long int)76800));
    v_x1 = (((((adc_H << 14) -(((signed long int)dig_H4) << 20) - (((signed long int)dig_H5) * v_x1)) + 
              ((signed long int)16384)) >> 15) * (((((((v_x1 * ((signed long int)dig_H6)) >> 10) * 
              (((v_x1 * ((signed long int)dig_H3)) >> 11) + ((signed long int) 32768))) >> 10) + (( signed long int)2097152)) * 
              ((signed long int) dig_H2) + 8192) >> 14));
   v_x1 = (v_x1 - (((((v_x1 >> 15) * (v_x1 >> 15)) >> 7) * ((signed long int)dig_H1)) >> 4));
   v_x1 = (v_x1 < 0 ? 0 : v_x1);
   v_x1 = (v_x1 > 419430400 ? 419430400 : v_x1);
   return (unsigned long int)(v_x1 >> 12);   
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


