#include <Arduino.h>
#include <SoftwareSerial.h>
#include <CheckDust.h>

#define LENG 31   //0x42 + 31 bytes equal to 32 bytes
unsigned char buf[LENG];

int PM01Value[3];          //define PM1.0 value of the air detector module
int PM2_5Value[3];         //define PM2.5 value of the air detector module
int PM10Value[3];         //define PM10 value of the air detector module
int Trans1 = 5;
int Trans2 = 8;
int Trans3 = 10;
int i = 0;
CheckDust keep;

long previousMillis = 0;
long previousMillis2 = 0;
long previousMillis3 = 0;
long previousMillis4 = 0;

const long interval = 30000;
const long interval2 = 60000;
const long interval3 = 90000;
const long interval4 = 120000;

void setup()
{
  Serial.begin(9600);   //use serial0
  Serial.setTimeout(1500);    //set the Timeout to 1500ms, longer than the data transmission periodic time of the sensor
  pinMode(Trans1,OUTPUT);
  pinMode(Trans2,OUTPUT);
  pinMode(Trans3,OUTPUT);
  
} 

void loop()
{  
  unsigned long currentMillis = millis();
  /*if(currentMillis - previousMillis4 >= 1000){
    previousMillis4 = currentMillis+92000;
    PM01Value[0] = 0;          
    PM2_5Value[0] = 0;           
    PM10Value[0] = 0;
    PM01Value[1] = 0;          
    PM2_5Value[1] = 0;           
    PM10Value[1] = 0;
    PM01Value[2] = 0;          
    PM2_5Value[2] = 0;           
    PM10Value[2] = 0;
    i = 0;
  }    */
  if((currentMillis - previousMillis >= interval)){
    previousMillis = currentMillis+92000;
      i = 0;
      digitalWrite(Trans1, HIGH);
      digitalWrite(Trans2, LOW);
      digitalWrite(Trans3, LOW);
    }
    if((currentMillis - previousMillis2 >= interval2)){
      previousMillis2 = currentMillis+92000;
      i = 1;
      digitalWrite(Trans1, LOW);
      digitalWrite(Trans2, HIGH);
      digitalWrite(Trans3, LOW);
     }
      if((currentMillis - previousMillis3 >= interval3)){
      previousMillis3 = currentMillis+92000;
      i = 2;
      digitalWrite(Trans1, LOW);
      digitalWrite(Trans2, LOW);
      digitalWrite(Trans3, HIGH);
      }
      if(Serial.find(0x42)){    //start to read when detect 0x42
      Serial.readBytes(buf,LENG);
       if(buf[0] == 0x4d){
         if(keep.checkValue(buf,LENG)){
           PM01Value[i] = keep.transmitPM01(buf); //count PM1.0 value of the air detector module
           PM2_5Value[i] = keep.transmitPM2_5(buf);//count PM2.5 value of the air detector module
           PM10Value[i] = keep.transmitPM10(buf); //count PM10 value of the air detector module 
          }           
        } 
      }
      
  
  static unsigned long OledTimer=millis();  
    if (millis() - OledTimer >=1000) 
    {
      OledTimer=millis(); 
      
      Serial.print("PM1.0: ");  
      Serial.print(PM01Value[0]);
      Serial.println("  ug/m3");            

    
      Serial.print("PM1.0D2: ");  
      Serial.print(PM01Value[1]);
      Serial.println("  ug/m3");

      
      Serial.print("PM1.0D3: ");  
      Serial.print(PM01Value[2]);
      Serial.println("  ug/m3");
      
   
      
      Serial.print("PM2.5: ");  
      Serial.print(PM2_5Value[0]);
      Serial.println("  ug/m3");     

      Serial.print("PM2.5D2: ");  
      Serial.print(PM2_5Value[1]);
      Serial.println("  ug/m3");

      Serial.print("PM2.5D3: ");  
      Serial.print(PM2_5Value[2]);
      Serial.println("  ug/m3");
      
   
      
      Serial.print("PM10: ");  
      Serial.print(PM10Value[0]);
      Serial.println("  ug/m3");   
     
      Serial.print("PM10D2: ");  
      Serial.print(PM10Value[1]);
      Serial.println("  ug/m3");   
      
      Serial.print("PM10D3: ");  
      Serial.print(PM10Value[2]);
      Serial.println("  ug/m3");   
      
    }
  
}

