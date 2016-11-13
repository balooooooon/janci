#include <TimerOne.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP085_U.h>

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

void Print_function()
{
  Serial.print("Cas: ");
  Serial.print(timer);  //cas
  Serial.print("s, Teplota1: ");
  Serial.print(temp);   //teplota von
  Serial.print("C, Teplota2: ");
  Serial.print(temp2);  //teplota dnu
  Serial.print("C, Tlak: ");
  Serial.print(event.pressure);   //tlak
  Serial.println("hPa");
                                  //xGPS
                                  //yGPS
                                  //zGPS  
}
 
void setup()
{
  Serial.begin(9600);
  
  Timer1.initialize(500000);         // initialize timer1, and set a 1/2 second period
  //Timer1.pwm(9, 512);                // setup pwm on pin 9, 50% duty cycle
  Timer1.attachInterrupt(Time_function);  // attaches callback() as a timer overflow interrupt

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
    Print_function();
  }
}
