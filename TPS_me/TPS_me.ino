#include <SoftwareSerial.h>
#include <String.h>
#include <SFE_BMP180.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#include <LiquidCrystal_I2C.h>
#include "RTClib.h"

RTC_DS1307 rtc;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
int i;
LiquidCrystal_I2C lcd(0x27, 16, 4);

SoftwareSerial SIM800L(8,9);  //Serial SIM800L pin

// Kabel data terhubung dengan pin 3
#define ONE_WIRE_BUS 3

// Setting oneWire untuk siap berkomunikasi
OneWire oneWire(ONE_WIRE_BUS);

DallasTemperature sensors(&oneWire);

float RH, TD, BK, BB, Min, Max;
SFE_BMP180 pressure;

char status;
double T,P;
String Write_API_key = "DOD2N9U4N179Y089";  //Thingspeak API Key
String apn = "xlgprs";   
float suhu; 
void setup(){
Serial.begin(9600); 
sensors.begin(); 
  SIM800L.begin(9600);
lcd.begin(); // or lcd.init();
lcd.backlight();  

  if (rtc.begin()) {
lcd.setCursor(0,0);
lcd.print("RTC oke");
  }

  if (rtc.isrunning()) {
    //Serial.println("RTC is running, let's set the time!");
    //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    //rtc.adjust(DateTime(2021, 10, 11, 19, 15, 0));
  }
  if (pressure.begin()){ //status modul
    lcd.setCursor(0,1);
    lcd.print("BMP180 Oke");
      }
      if (SIM800L.available()){
    Serial.write(SIM800L.read());
    lcd.setCursor(-4,2);
    lcd.print("SIM800L oke");
    }
    delay(2000);
    
    DateTime now = rtc.now();
    lcd.setCursor(-4,3);
    lcd.print(now.year(), DEC);
    lcd.print('/');
    lcd.print(now.month(), DEC);
    lcd.print('/');
    lcd.print(now.day(), DEC);
    lcd.print(" ");
    lcd.print(now.hour(), DEC);
    lcd.print(':');
    lcd.print(now.minute(), DEC);
    lcd.print(':');
    lcd.print(now.second(), DEC);
    delay(2000);
     }
    
    void loop(){
    ReadSensor();
      //perhitungan
      float e1 = 6.112*exp ((17.67*BK)/(BK+273.5));
      float e2 = 6.112*exp ((17.67*BB)/(BB+273.5));
      float e = e2-0.00066*P*(BK-BB); 
      float a = 273.5*log(e/6.112);
      float b = (17.67)-(log(e/6.112));
          RH = (e/e1)*100;
          TD = (a/b);
    
    SetupModule();
    SIM800L.println("AT+CIPSTART=\"TCP\",\"api.Thingspeak.com\",\"80\"");
    delay(7000);
    ShowSerialData();
    
      SIM800L.println("AT+CIPSEND");//perintahpengiriman data
    delay(2000);
    Serial.println();
    ShowSerialData();
      String str = "GET https://api.Thingspeak.com/update?api_key="+Write_API_key+"&field1=" + String(BK)+"&field2=" + String(BB)+"&field3=" + String(P)+"&field4=" + String(RH)+"&field5=" + String(TD);  
    Serial.println(str);   
    delay(2000);
      SIM800L.println(str); 
    delay(2000);
    ShowSerialData();
    
      SIM800L.println((char)26);
    delay(2000); 
      SIM800L.println();
    ShowSerialData();
      SIM800L.println("AT+CIPSHUT");//close the connection
    delay(200000); 
    ShowSerialData();
      str="";
    
      //total delay looping 5 mnt
    delay(10000);
    
      //set lcd untukmenampilkan data dan rtc
    DateTime now = rtc.now();
    lcd.setCursor(-4,3);
    lcd.print(now.year(), DEC);
    lcd.print('/');
    lcd.print(now.month(), DEC);
    lcd.print('/');
    lcd.print(now.day(), DEC);
    lcd.print(" ");
    lcd.print(now.hour(), DEC);
    lcd.print(':');
    lcd.print(now.minute(), DEC);
    lcd.print(':');
    lcd.print(now.second(), DEC);
    
    lcd.setCursor(0,0);
    lcd.print("Pr = ");
    lcd.print(P,2);
    lcd.print(" mb ");
    lcd.setCursor(0,1);
    lcd.print("RH = ");
    lcd.print(RH);
    lcd.print(" % ");
    lcd.setCursor(-4,2);
    lcd.print("TD = ");
    lcd.print(TD);
    lcd.print(" C ");
    } 
    
    void ReadSensor(){ //looping proses pembacaan sensor
      status = pressure.startTemperature();
      if (status != 0)
      {
        delay(status);
    
        status = pressure.getTemperature(T);
        if (status != 0)
        {          
          status = pressure.startPressure(3);
          if (status != 0)
          {
            delay(status);
            status = pressure.getPressure(P,T);
            }
          }
         }
      // request data sensor
    sensors.requestTemperatures(); 
      BK = (sensors.getTempCByIndex(0));
      BB = (sensors.getTempCByIndex(1)- 0.25);
    delay(500); 
    Serial.println();    
    Serial.print("BK = ");
    Serial.print(BK); 
    Serial.print(" °C");
    Serial.print(" ");
    Serial.print("BB = ");
    Serial.print(BB); 
    Serial.print(" °C");
    Serial.print(" ");
    Serial.print("P = ");
    Serial.print(P); 
    Serial.print(" mbar");
    Serial.println(); 
    }
    
    void SetupModule(){  //konfigurasi SIM card sebelumdigunakan
      if (SIM800L.available())
    Serial.write(SIM800L.read());
      SIM800L.println("AT"); 
    delay(1000);
    //setup sim800l
     /* SIM800L.println("AT+CPIN?"); delay(1000);
      SIM800L.println("AT+CREG?"); delay(1000);
      SIM800L.println("AT+CGATT?"); delay(1000);
      SIM800L.println("AT+CIPSHUT");delay(1000);
      SIM800L.println("AT+CIPSTATUS"); delay(2000);
      SIM800L.println("AT+CIPMUX=0");  delay(2000);
    
     //setting the APN,
      SIM800L.println("AT+CSTT=\""+apn+"\"");delay(1000);
    ShowSerialData();
      SIM800L.println("AT+CIICR");    delay(2000);
    ShowSerialData();
    
      //get local IP adress
      SIM800L.println("AT+CIFSR");    delay(2000);
    ShowSerialData();
    
      SIM800L.println("AT+CIPSPRT=0");delay(2000);
    ShowSerialData();*/
    }
    
    void ShowSerialData(){
    while(SIM800L.available()!=0)
    Serial.write(SIM800L.read());
    delay(2000); 
    }

