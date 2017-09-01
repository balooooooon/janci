#define DEBUG

#ifdef DEBUG
  #define debug_println(x) Serial.println(x)
  #define debug_print(x) Serial.print(x)
#else
  #define debug_println(x)
  #define debug_print(x)
#endif

#define error_log(x) Serial.println(x)


/* -- GSM Module -- */
#include "GsmModule.h"

/* -- File Handler -- */
#include "FileHandler.h"
extern File file;

#include <TimerOne.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP085_U.h>
#include <TinyGPS++.h> // Include the TinyGPS++ library

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


struct position {
  float lat = 0.0;
  float lng = 0.0;
  float alt = 0.0;
  uint32_t sat = 0;
  // experimental values, but possibly interesting
  double speed = 0.0; // speed in meters per second
  double course = 0.0; // Course in degrees
} oldPosition, actPosition;

struct temperature {
  float internal = 0.0; // kty()
  float external = 0.0; // BMP085
} actTemperature;

struct time {
  float raw = 0.0;
  int hour = 0;
  int minute = 0;
  int second = 0;
} actTime;


// isGpsValid() variables
boolean validGPS = false;
int times = 1;
float reftimes;
boolean isOk = false;
float oldlat = 0.0;
float oldlng = 0.0;

Adafruit_BMP085_Unified bmp = Adafruit_BMP085_Unified(10085);
sensors_event_t event;

// Watchdog & Timer variables
boolean wdt = false;
float timer = 0.0;
float oldTime =0.0;
boolean firstTime = true;

void(* resetFunc) (void) = 0;

void Time_function() {
  if (wdt) resetFunc();
  timer += 5;
}

float kty(unsigned int port) {
  float temp = 82;
  ADCSRA = 0x00;
  ADCSRA = (1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);
  ADMUX = 0x00;
  ADMUX = (1<<REFS0);
  ADMUX |= port;   

  for (int i=0;i<=64;i++) {
    ADCSRA|=(1<<ADSC);
    while (ADCSRA & (1<<ADSC));
    temp += (ADCL + ADCH*256);
  }

  temp /= 101;
  temp -= 156;
  return (temp);
}

boolean isGpsValid() { 
  //treba doriesit osetrenie prvej suradnice - prvu povazujem za spravnu - resp spravna je aj 0.0.0 - osetrenie v pythone
  isOk = false;
  reftimes = ref * times;

  if ((aktlat == 0) && (aktlng == 0)) {
    isOk = true;
    times = 1;
    return isOk;
  }
    
  if (((oldlat - reftimes) <= aktlat) && (aktlat <= (oldlat + reftimes))) {
    if (((oldlng - reftimes) <= aktlng) && (aktlng <= (oldlng + reftimes))) {
      isOk = true;
      times = 1;
    }
    else {
      times++;
      if (times == 13) {  
        //v pripade ze sa nacita odveci prva suradnica, tak aby aspon po minute sa suradnica poslala
        isOk = true;
        times = 1;
      }
    }
  }
  return isOk;
}

void printTime() {
  Serial.print(actTime.hour);
  Serial.print(":");
  Serial.print(actTime.minute);
  Serial.print(":");
  Serial.print(actTime.second);
}

void Serial1_debug_print() {
  Serial.print(timer);  //cas
  Serial.print(",");
  Serial.print(actTemperature.internal);   //teplota von
  Serial.print(",");
  Serial.print(actTemperature.external);  //teplota dnu
  Serial.print(",");
  Serial.print(event.pressure);   //tlak
  Serial.print(",");
  Serial.print(actPosition.lat, 6); //lat
  Serial.print(",");
  Serial.print(actPosition.lng, 6); //long
  Serial.print(",");
  Serial.print(actPosition.alt, 6); //alt
  Serial.print(",");
  printTime();
  Serial.print(",");
  Serial.println(actPosition.sat);
}

static void smartDelay(unsigned long ms) {
  unsigned long start = millis();
  do {
    // If data has come in from the GPS module
    while ( gpsPort.available() )
      tinyGPS.encode(gpsPort.read()); 
    // Send it to the encode function
    // tinyGPS.encode(char) continues to "load" the tinGPS object with new
    // data coming in from the GPS module. As full NMEA strings begin to come in
    // the tinyGPS library will be able to start parsing them for pertinent info
  } while (millis() - start < ms);
}
 
void setup() {
  
  // Debug serial
  Serial.begin(9600);
  
  // Communication with second Arduino for Radio
  Serial2.begin(9600);

  pinMode(4, INPUT);  //testovanie 4teho pinu na zasunutie karty
  digitalWrite(4, HIGH);

  //pinMode(RADIOPIN,OUTPUT);
  
  Timer1.initialize(5000000);         // initialize timer1, and set a 1/2 second period
  //Timer1.pwm(9, 512);                // setup pwm on pin 9, 50% duty cycle
  Timer1.attachInterrupt(Time_function);  // attaches callback() as a timer overflow interrupt

  wdt = false;
  
  if ( !initializeSD() ) {
    // TODO: Ak zlyha SD.begin()
    error_log("ERROR: Cannot initialize SD Card");
  }
   
  if ( !createFile("test.txt") ) {
    // TODO: Ak zlyha vytvorenie suboru
    error_log("ERROR: Cannot create file on SD Card");
  }
  
  file.println("**************************************************\n");
  closeFile();

  gpsPort.begin(GPS_BAUD);
//  if ( ! ) {
//    error_log("ERROR: Cannot initialize GPS Serial.");  
//  }

  if ( !bmp.begin() ) {
    error_log("ERROR: Cannot initialize BMP085 Sensor.");  
  }

  if ( gsmModuleInit() ) {
    error_log("ERROR: Cannot initialize GSM Module.");  
  }
}
 
void loop() {  
  //+20 - pre 20 sekundove merania
  if ( ( timer == ( oldTime + 5 ) ) || firstTime ) {

    //nastavenie WDT - v pripade ze by telo funkcie trvalo 5 sekund - program bude resetovany
    wdt = true; 
    
    debug_println("som v loope");
    
    firstTime = false;
    oldTime = timer;
    
    /*while (gpsPort.available())
      tinyGPS.encode(gpsPort.read());*/
    smartDelay(1000); //delay + get GPS

    debug_println("pred printom");

    int res;

    if ( !(res = retrieveAtmosphericData()) ) {
      // TODO: Whatever / res - error code
      error_log("ERROR: Cannot retrieve all atmospheric data");
    }
    
    if ( !(res = retrieveGPSData()) ) {
      // TODO: Whatever / res - error code
      error_log("ERROR: Cannot retrieve GPS data");
    }

    #ifdef DEBUG
      Serial1_debug_print();
    #endif

    // Log all data to SD card
    logDataToSD();

    // Send data to Raduino ;)
    sendToSlave();

    wdt = false;
    debug_println("vychadzam z loopu");
  }
}

/*
 * Retrieve all atmospheric data from different sensors
 * @return int 0 if OK, >0 if error
 */
int retrieveAtmosphericData() {
  actTemperature.internal = kty(0);

  // TODO: event by mal v sebe obsahovat aj teplotu
  bmp.getEvent(&event);
  bmp.getTemperature(&(actTemperature.external));

  return 0;
}

/*
 * Retrieve data from GPS sensor
 * @return int 0 if OK, >0 if error
 */
int retrieveGPSData() {
  actPosition.sat = tinyGPS.satellites.value();
  // TODO: if isUpdated/isValid
    
  if( tinyGPS.location.isUpdated() ) {
    actPosition.lat = tinyGPS.location.lat();
    actPosition.lng = tinyGPS.location.lng();
  } else return 1;

  if ( tinyGPS.altitude.isUpdated() ) {
    actPosition.alt = tinyGPS.altitude.meters();
  } else return 2;

  actPosition.speed = tinyGPS.speed.mps();
  // TODO: if isUpdated/isValid

  actPosition.course = tinyGPS.course.deg();
  // TODO: if isUpdated/isValid

  actTime.raw = tinyGPS.time.value();
  actTime.hour = tinyGPS.time.hour();
  actTime.minute = tinyGPS.time.minute();
  actTime.second = tinyGPS.time.second();
  // TODO: if isUpdated/isValid

  return 0;
}

void logDataToSD() {
  int connected_card = digitalRead(4);
  if (connected_card) {
    createFile("test.txt");
    writeToFileFloat(timer);
    writeToFileFloat(actPosition.lat);
    writeToFileFloat(actPosition.lng);
    writeToFileFloat(actPosition.alt);
    writeToFileFloat(actTemperature.internal);
    writeToFileFloat(actTemperature.external);
    writeToFileFloat(event.pressure);
    writeToFileFloat(actTime.raw);
    writeToFileFloat(actPosition.sat);
    file.println("\n");
    closeFile();
  } else initializeSD();
}

void sendToSlave() {
  // int timer_int = (int) timer;
  Serial2.print((int) timer);  //cas
  Serial2.print(",");
  Serial2.print(actTemperature.internal);   //teplota von
  Serial2.print(",");
  Serial2.print(actTemperature.external);  //teplota dnu
  Serial2.print(",");
  Serial2.print(((int)event.pressure));   //tlak
  Serial2.print(",");
    
  Serial2.print(actPosition.lat, 6); //lat
  Serial2.print(",");
  Serial2.print(actPosition.lng, 6); //long
  Serial2.print(",");
  Serial2.print(actPosition.alt, 6); //alt
   
  Serial2.print(",");
  
  Serial2.print(actTime.hour);
  Serial2.print(":");
  Serial2.print(actTime.minute);
  Serial2.print(":");
  Serial2.print(actTime.second);

  Serial2.print(",");
  Serial2.print(actPosition.sat);
  Serial2.print("$");
}
