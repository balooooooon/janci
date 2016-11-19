#include <TimerOne.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP085_U.h>
#include <TinyGPS++.h> // Include the TinyGPS++ library
#include <SoftwareSerial.h>

#define GPS_BAUD 9600 // GPS module baud rate. GP3906 defaults to 9600.
#define ARDUINO_GPS_RX 9 // GPS TX, Arduino RX pin
#define ARDUINO_GPS_TX 8 // GPS RX, Arduino TX pin
#define gpsPort ssGPS  // Alternatively, use Serial1 on the Leonardo

SoftwareSerial ssGPS(ARDUINO_GPS_TX, ARDUINO_GPS_RX); // Create a SoftwareSerial
TinyGPSPlus tinyGPS; // Create a TinyGPSPlus object
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

void printTime()
{
  Serial.print(tinyGPS.time.hour());
  Serial.print(":");
  Serial.print(tinyGPS.time.minute());
  Serial.print(":");
  Serial.println(tinyGPS.time.second());
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

  Serial.print(tinyGPS.location.lat(), 6); //lat
  Serial.print(",");
  Serial.print(tinyGPS.location.lng(), 6); //long
  Serial.print(",");
  Serial.print(tinyGPS.altitude.feet(), 6); //alt

  Serial.print(",");
  printTime();
}
 
void setup()
{
  Serial.begin(9600);
  
  Timer1.initialize(500000);         // initialize timer1, and set a 1/2 second period
  //Timer1.pwm(9, 512);                // setup pwm on pin 9, 50% duty cycle
  Timer1.attachInterrupt(Time_function);  // attaches callback() as a timer overflow interrupt
  
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
      
    Print_function();
  }
}
