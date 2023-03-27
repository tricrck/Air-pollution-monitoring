#include <MQ135.h>
#include <DHT.h>
#include <ThingSpeak.h>
#include <SoftwareSerial.h>
#include <LiquidCrystal.h>
#define TYPE DHT21
#define PIN  3
#define RX 11
#define TX 10
String AP = "Topper";       // AP NAME
String PASS = "seeksmee"; // AP PASSWORD
String API = "Z9HVU6BY133YHCSS";   // Write API KEY
String field = "field1";
int countTrueCommand;
int countTimeCommand; 
boolean found = false; 
float valSensor_hum;
float valSensor_temp;
float air_quality;
const int buzzer = 2;
const int sensorPin= 0;
long writingTimer = 17; 
long startTime = 0;
long waitTime = 0;

boolean relay1_st = false; 
boolean relay2_st = false; 
boolean error;
SoftwareSerial esp8266(RX,TX);
// object initialization
DHT dht(PIN, TYPE,21);
const int rs = 9, en = 8, d4 = 4, d5 = 5, d6 = 6, d7 = 7;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
 
void sendCommand(String command, int maxTime, char readReplay[])
{
  Serial.print(countTrueCommand);
  Serial.print(". at command => ");
  Serial.print(command);
  Serial.print(" ");
  while (countTimeCommand < (maxTime * 1))
  {
    esp8266.println(command);//at+cipsend
    if (esp8266.find(readReplay)) //ok
    {
      found = true;
      break;
    }
    countTimeCommand++;
  }
  if (found == true)
  {
    Serial.println("OYI");
    countTrueCommand++;
    countTimeCommand = 0;
  }
  if (found == false)
  {
    Serial.println("Fail");
    countTrueCommand = 0;
    countTimeCommand = 0;
  }
  found = false;
} 
void setup() {
  Serial.begin(9600);
  esp8266.begin(115200);
   dht.begin();
  startTime = millis();
  sendCommand("AT",5,"OK");
  sendCommand("AT+CWMODE=1",5,"OK");
  lcd.begin(16,2);
  lcd.setCursor (0,0);
  lcd.print (" Connecting to wifi .... ");
  lcd.scrollDisplayLeft();
  delay(1000);
  sendCommand("AT+CWJAP=\""+ AP +"\",\""+ PASS +"\"",20,"OK");\
  lcd.setCursor (0,1);
  lcd.print ("CONNECTED ");
  delay(1000);
  pinMode(en, OUTPUT);
  lcd.begin(16,2);
  lcd.setCursor (0,0);
  lcd.print (" IoT Based Air Monitoring System ");
  lcd.scrollDisplayLeft();
  lcd.setCursor (0,1);
  lcd.print ("Sensor Warming ");
  delay(1000);
  pinMode(sensorPin, INPUT);
  pinMode(buzzer, OUTPUT);
  lcd.clear();
}


void loop() {
waitTime = millis()-startTime;
// Serial.println(waitTime);
// Serial.println(writingTimer);

if (waitTime > (writingTimer*1000)) 
  {
    readSensors_writeThingSpeak();
    startTime = millis();   

  }
}

void readSensors_writeThingSpeak(void)
{
  MQ135 gasSensor = MQ135(A3);
  float air_quality = gasSensor.getPPM();
  Serial.println(air_quality);
  float valSensor_temp = dht.readTemperature();
  Serial.println(valSensor_temp);
  float valSensor_hum = dht.readHumidity();
  Serial.println(valSensor_hum);
  
  // Write LCD Crystal Display
   lcd.setCursor (0, 0);
   lcd.print ("Air Quality is ");
   lcd.print (air_quality);
   lcd.print (" PPM ");
   lcd.scrollDisplayLeft();
   delay(1000);
   lcd.setCursor (0,1);
   if (air_quality<=1000)
     {
   lcd.print("Fresh Air");
   noTone(buzzer);
   delay(1000); 
     }
   else if( air_quality>=1000 && air_quality<=2000 )
    {
    lcd.print("Poor Air, Open Windows");
    tone(buzzer, 2000);
    delay(1000); 
    }
    else if (air_quality>=2000 )
    {
    lcd.print("Danger! Move to Fresh Air");
    tone(buzzer, 3000);
    delay(1000); 
    }
    lcd.scrollDisplayLeft();
    delay(1000);
  
// write ThingSpeak
  
    startThingSpeakCmd();
    String getStr = "GET /update?api_key=";
    getStr += API;
    getStr +="&field1=";
    getStr += String(air_quality);
    getStr +="&field2=";
    getStr += String(valSensor_temp);
    getStr +="&field3=";
    getStr += String(valSensor_hum);
    getStr += "\r\n\r\n\r\n";
    GetThingspeakcmd(getStr); 
   
}

void startThingSpeakCmd()
    {
      esp8266.flush();
      String cmd = "AT+CIPSTART=\"TCP\",\"";
      cmd += "184.106.153.149"; // api.thingspeak.com IP address
      cmd += "\",80";
      esp8266.println(cmd);
      Serial.print("Start Commands: ");
      Serial.println(cmd);
    
      if(esp8266.find("Error"))
      {
        Serial.println("AT+CIPSTART error");
        return;
      }
    }
    
    String GetThingspeakcmd(String getStr)
    {
      String cmd = "AT+CIPSEND=";
      cmd += String(getStr.length());
      esp8266.println(cmd);
      Serial.println(cmd);
    
        esp8266.print(getStr);
        Serial.println(getStr);
        delay(500);
        String messageBody = "";
        while (esp8266.available()) 
        {
          String line = esp8266.readStringUntil('\n');
          if (line.length() == 1) 
          { 
            messageBody = esp8266.readStringUntil('\n');
          }
        }
        Serial.print("MessageBody received: ");
        Serial.println(messageBody);
        return messageBody;
    }
