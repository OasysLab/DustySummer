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
int n = 0;

CheckDust keep;

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
  if(n == 3){
    PM01Value[0] = 0;          
    PM2_5Value[0] = 0;         
    PM10Value[0] = 0;
    PM01Value[1] = 0;          
    PM2_5Value[1] = 0;         
    PM10Value[1] = 0;
    PM01Value[2] = 0;          
    PM2_5Value[2] = 0;         
    PM10Value[2] = 0;
    n = 0;
  }
  for(int i = 0;i < 3;i++){
    if(i == 0){
      digitalWrite(Trans1, HIGH);
      digitalWrite(Trans2, LOW);
      digitalWrite(Trans3, LOW);
      delay(10000);
    }
    if(i == 1){
      digitalWrite(Trans1, LOW);
      digitalWrite(Trans2, HIGH);
      digitalWrite(Trans3, LOW);
      delay(10000);
    }
    if(i == 2){
      digitalWrite(Trans1, LOW);
      digitalWrite(Trans2, LOW);
      digitalWrite(Trans3, HIGH);
      delay(10000);
    }
    for(int j = 0;j < 20;j++){
    if(Serial.find(0x42)){    
      Serial.readBytes(buf,LENG);
       if(buf[0] == 0x4d){
         if(keep.checkValue(buf,LENG)){
           PM01Value[j] = keep.transmitPM01(buf); 
           PM2_5Value[j] = keep.transmitPM2_5(buf);
           PM10Value[j] = keep.transmitPM10(buf); 
          }           
        } 
      }
    /*static unsigned long OledTimer=millis();  
    if (millis() - OledTimer >=1000) 
    {
      OledTimer=millis(); */
      
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
      
    //}
    delay(1000);
    }
    n++;
  }
}

