#include <TimerOne.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP085_U.h>
#include <TinyGPS++.h> // Include the TinyGPS++ library
#include <SD.h>
#include <SoftwareSerial.h>

#define GPS_BAUD 9600 // GPS module baud rate. GP3906 defaults to 9600.
#define ARDUINO_GPS_RX A9 // GPS TX, Arduino RX pin
#define ARDUINO_GPS_TX A8 // GPS RX, Arduino TX pin
#define gpsPort ssGPS  // Alternatively, use Serial1 on the Leonardo
//#define ref 0.0001259 // 14 meters - pre 0,5 sekundove merania
//#define ref 0.005036 // 556 meters (pre 20 sekundove intervaly - 100km/h
#define ref 0.00125 // 139 meters (pre 5 sekundove intervaly - 100km/h

#define CS_PIN 53

SoftwareSerial ssGPS(ARDUINO_GPS_TX, ARDUINO_GPS_RX); // Create a SoftwareSerial
TinyGPSPlus tinyGPS; // Create a TinyGPSPlus object

float failgps = 0.0;
float oldlat = 0.0;
float oldlng = 0.0;
float aktlat = 0.0;
float aktlng = 0.0;
float aktalt = 0.0;
int aktsat = 0;

boolean wdt = false;

boolean validGPS = false;
int times = 1;
float reftimes;
boolean isOk = false;

float urtime = 0.0;
File file;

float timer = 0.0;
float timer2 = 0.0;
float oldTime =0.0;
boolean firstTime = true;
float temp;
float temp2;
Adafruit_BMP085_Unified bmp = Adafruit_BMP085_Unified(10085);
sensors_event_t event;

void(* resetFunc) (void) = 0;

void Time_function()
{
  if (wdt)
  {
    resetFunc();
  }
  timer+=5;
}

void initializeSD()
{
  pinMode(CS_PIN, OUTPUT);
  SD.begin(CS_PIN);
}

int createFile(char filename[])
{
  file = SD.open(filename, FILE_WRITE);

  //Serial.println(file);

  if (file)
  {
    return 1;
  } else
  {
    return 0;
  }
}

int writeToFileFloat(float number)
{
  if (file)
  {
    file.println(number);
    return 1;
  } else
  {
    return 0;
  }
}

int writeToFile(char text[])
{
  if (file)
  {
    file.print(text);
    return 1;
  } else
  {
    return 0;
  }
}

void closeFile()
{
  if (file)
  {
    file.close();
  }
}

float kty(unsigned int port) {
         float temp              = 82;
         ADCSRA = 0x00;
         ADCSRA = (1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);
         ADMUX = 0x00;
         ADMUX = (1<<REFS0);
         ADMUX |= port;   

         for (int i=0;i<=64;i++)
         {
                 ADCSRA|=(1<<ADSC);
                 while (ADCSRA & (1<<ADSC));
                 temp += (ADCL + ADCH*256);
         }

         temp /= 101;
         temp -= 156;
       return (temp);
}

boolean isGpsValid()
{ //treba doriesit osetrenie prvej suradnice - prvu povazujem za spravnu - resp spravna je aj 0.0.0 - osetrenie v pythone
  isOk = false;
  reftimes = ref * times;

  if ((aktlat == 0) && (aktlng == 0))
  {
    isOk = true;
    times = 1;
    return isOk;
  }
    
  if (((oldlat - reftimes) <= aktlat) && (aktlat <= (oldlat + reftimes)))
  {
    if (((oldlng - reftimes) <= aktlng) && (aktlng <= (oldlng + reftimes)))
    {
      isOk = true;
      times = 1;
    }
    else
    {
      times++;
      if (times == 13)  //v pripade ze sa nacita odveci prva suradnica, tak aby aspon po minute sa suradnica poslala
      {
        isOk = true;
        times = 1;
      }
    }
  }
  return isOk;
}

void printTime()
{
  Serial.print(tinyGPS.time.hour());
  Serial.print(":");
  Serial.print(tinyGPS.time.minute());
  Serial.print(":");
  Serial.print(tinyGPS.time.second());
  urtime = tinyGPS.time.value();
}

void Print_function()
{
  Serial.print(timer);  //cas
  Serial.print(",");
  Serial.print(temp);   //teplota von
  Serial.print(",");
  Serial.print(temp2);  //teplota dnu
  Serial.print(",");
  Serial.print(event.pressure);   //tlak
  Serial.print(",");

  /*if (isGpsValid())
  {*/
    //oldlat = aktlat;  //lastvalidGPSposition
    //oldlng = aktlng;
    
    Serial.print(aktlat, 6); //lat
    Serial.print(",");
    Serial.print(aktlng, 6); //long
    Serial.print(",");
    aktalt = tinyGPS.altitude.meters();
    Serial.print(aktalt, 6); //alt
  /*}
  else
  {*/
    //Serial.print(failgps, 6); //lat
    //Serial.print(",");
    //Serial.print(failgps, 6); //long
    //Serial.print(",");
    //Serial.print(failgps, 6); //alt
  //}
   
  Serial.print(",");
  printTime();

  Serial.print(",");
  Serial.println(aktsat);

  sendToSlave();
  logGPSData();
}

static void smartDelay(unsigned long ms)
{
  unsigned long start = millis();
  do
  {
    // If data has come in from the GPS module
    while (gpsPort.available())
      tinyGPS.encode(gpsPort.read()); // Send it to the encode function
    // tinyGPS.encode(char) continues to "load" the tinGPS object with new
    // data coming in from the GPS module. As full NMEA strings begin to come in
    // the tinyGPS library will be able to start parsing them for pertinent info
  } while (millis() - start < ms);
}
 
void setup()
{
  Serial.begin(9600);
  Serial2.begin(9600);

  pinMode(4, INPUT);  //testovanie 4teho pinu na zasunutie karty
  digitalWrite(4, HIGH);

  //pinMode(RADIOPIN,OUTPUT);
  
  Timer1.initialize(5000000);         // initialize timer1, and set a 1/2 second period
  //Timer1.pwm(9, 512);                // setup pwm on pin 9, 50% duty cycle
  Timer1.attachInterrupt(Time_function);  // attaches callback() as a timer overflow interrupt

  wdt = false;
  
  initializeSD();
  createFile("test.txt");
  file.println("**************************************************\n");
  closeFile();
  
  gpsPort.begin(GPS_BAUD);  
  bmp.begin();
}
 
void loop()
{  
  if ((timer == (oldTime+5)) || firstTime) //+20 - pre 20 sekundove merania
  {
    wdt = true; //nastavenie WDT - v pripade ze by telo funkcie trvalo 5 sekund - program bude resetovany
    Serial.print("som v loope");
    firstTime = false;
    oldTime = timer;
    temp = kty(0);
    bmp.getEvent(&event);
    bmp.getTemperature(&temp2);
    
    /*while (gpsPort.available())
      tinyGPS.encode(gpsPort.read());*/
    smartDelay(1000); //delay + get GPS

    Serial.println("pred printom");
    
    aktlat = tinyGPS.location.lat();
    aktlng = tinyGPS.location.lng();
    aktsat = tinyGPS.satellites.value();
      
    Print_function();

    wdt = false;
    Serial.println("vychadzam z loopu");
  }
}

void logGPSData()
{
  int connected_card = digitalRead(4);
  if (connected_card)
  {
  createFile("test.txt");
  writeToFileFloat(timer);
  writeToFileFloat(aktlng);
  writeToFileFloat(aktlat);
  writeToFileFloat(aktalt);
  writeToFileFloat(temp);
  writeToFileFloat(temp2);
  writeToFileFloat(event.pressure);
  writeToFileFloat(urtime);
  writeToFileFloat(aktsat);
  file.println("\n");
  closeFile();
  }
  else
    initializeSD();
}

void sendToSlave()
{
  Serial2.print(timer);  //cas
  Serial2.print(",");
  Serial2.print(temp);   //teplota von
  Serial2.print(",");
  Serial2.print(temp2);  //teplota dnu
  Serial2.print(",");
  Serial2.print(event.pressure);   //tlak
  Serial2.print(",");
    
    Serial2.print(aktlat, 6); //lat
    Serial2.print(",");
    Serial2.print(aktlng, 6); //long
    Serial2.print(",");
    Serial2.print(aktalt, 6); //alt
   
  Serial2.print(",");
  
  Serial2.print(tinyGPS.time.hour());
  Serial2.print(":");
  Serial2.print(tinyGPS.time.minute());
  Serial2.print(":");
  Serial2.print(tinyGPS.time.second());

  Serial2.print(",");
  Serial2.print(aktsat);
  Serial2.print("$");
}
