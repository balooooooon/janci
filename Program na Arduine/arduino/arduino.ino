#include <TimerOne.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP085_U.h>
#include <TinyGPS++.h> // Include the TinyGPS++ library
#include <SD.h>
#include <SoftwareSerial.h>

#define GPS_BAUD 9600 // GPS module baud rate. GP3906 defaults to 9600.
#define ARDUINO_GPS_RX 9 // GPS TX, Arduino RX pin
#define ARDUINO_GPS_TX 8 // GPS RX, Arduino TX pin
#define gpsPort ssGPS  // Alternatively, use Serial1 on the Leonardo
#define ref 0.0001259 // 14 meters

#define CS_PIN 10

SoftwareSerial ssGPS(ARDUINO_GPS_TX, ARDUINO_GPS_RX); // Create a SoftwareSerial
TinyGPSPlus tinyGPS; // Create a TinyGPSPlus object

float oldlat = 0.0;
float oldlng = 0.0;
float aktlat = 0.0;
float aktlng = 0.0;
float aktalt = 0.0;

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

void Time_function()
{
  timer2++;
}

void initializeSD()
{
  pinMode(CS_PIN, OUTPUT);
  SD.begin();
}

int createFile(char filename[])
{
  file = SD.open(filename, FILE_WRITE);

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
{
  isOk = false;
  reftimes = ref * times;
    
  if ((((oldlat - reftimes) <= aktlat) && (aktlat <= (oldlat + reftimes))) || (oldlat == 0))
  {
    if ((((oldlng - reftimes) <= aktlng) && (aktlng <= (oldlng + reftimes))) || (oldlng == 0))
    {
      isOk = true;
      times = 1;
    }
    else
      times++;
  }
  
  oldlat = aktlat;
  oldlng = aktlng;
  return isOk;
}

void printTime()
{
  Serial.print(tinyGPS.time.hour());
  Serial.print(":");
  Serial.print(tinyGPS.time.minute());
  Serial.print(":");
  Serial.println(tinyGPS.time.second());
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

  if (isGpsValid())
  {
    Serial.print(aktlat, 6); //lat
    Serial.print(",");
    Serial.print(aktlng, 6); //long
    Serial.print(",");
    aktalt = tinyGPS.altitude.feet();
    Serial.print(aktalt, 6); //alt
  }
  else
  {
    Serial.print("fail"); //lat
    Serial.print(",");
    Serial.print("fail"); //long
    Serial.print(",");
    Serial.print("fail"); //alt
  }
  
  Serial.print(",");
  printTime();
  
  logGPSData();
}
 
void setup()
{
  Serial.begin(9600);
  
  Timer1.initialize(500000);         // initialize timer1, and set a 1/2 second period
  //Timer1.pwm(9, 512);                // setup pwm on pin 9, 50% duty cycle
  Timer1.attachInterrupt(Time_function);  // attaches callback() as a timer overflow interrupt

  initializeSD();
  createFile("test.txt");
  file.println("**************************************************\n");
  closeFile();
  
  gpsPort.begin(GPS_BAUD);  
  bmp.begin();
}
 
void loop()
{
  timer = timer2 / 2;
  
  if ((timer != oldTime) || firstTime)
  {
    firstTime = false;
    oldTime = timer;
    temp = kty(0);
    bmp.getEvent(&event);
    bmp.getTemperature(&temp2);
    
    while (gpsPort.available())
      tinyGPS.encode(gpsPort.read());

    aktlat = tinyGPS.location.lat();
    aktlng = tinyGPS.location.lng();
      
    Print_function();
  }
}

void logGPSData()
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
  writeToFileFloat(tinyGPS.satellites.value());
  file.println("\n");
  closeFile(); 
}
