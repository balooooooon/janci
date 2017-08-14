#include "GsmModule.h"

#define PINNUMBER "" // SIM Card PIN number
#define STATPIN 4   //Modem Status Pin
#define SIGNALLEDPIN 13 //LED indicator to show when connected to GSM network - treba zmenit cislo ledky
#define MAX_TRIES 5 // Max tries when connecting to GSM network

//Global classes
GSM gsmAccess;
GSMScanner scannerNetworks;
GSMModem modemTest;
GSMVoiceCall gsmVC;
GSM_SMS sms;

int gsmModuleInit() {
  String currentCarrier = "";
  String IMEI = "";

  debug_println("Starting Modem...");
  
  pinMode(SIGNALLEDPIN,OUTPUT);
  digitalWrite(SIGNALLEDPIN,LOW);

  // Try if GSM modem is working
  if( modemTest.begin() ) {
    debug_println("GSM Modem started.");
  } else {
    debug_println("ERROR: GSM Modem not working.");
    // return 2;
  }

  // Try to get IMEI
  IMEI = modemTest.getIMEI();
  IMEI.replace("\n","");
  if(IMEI != NULL) 
    debug_println("Modem IMEI: " + IMEI);
  else 
    debug_println("ERROR: Can't load IMEI.");

  debug_println("GSM Modem initialized.");

  bool notConnected = true;
  int gsmConnectTryCount = 0;

  // Connect to GSM network
  while( notConnected && gsmConnectTryCount < MAX_TRIES) {
    if( gsmAccess.begin(PINNUMBER) == GSM_READY ) {
      notConnected = false;
      digitalWrite(SIGNALLEDPIN,HIGH);
      debug_println("Connected to GSM Network");
    } else {
      debug_println("Connecting to GSM Network...");
      delay(500);
      gsmConnectTryCount++;
    }
  }

  pinMode(STATPIN,INPUT);
  int gsmStat = 0;
  gsmStat = digitalRead(STATPIN);

  if( gsmStat == 0 ) {
    debug_println("ERROR: Could't connect to network");
  } else {
    debug_println("Modem started and connected to network.");
  }

  scannerNetworks.begin();
  currentCarrier = scannerNetworks.getCurrentCarrier();
  currentCarrier.trim();
   
  if(currentCarrier.length() != 0) {
    debug_println("Current carrier: " + currentCarrier);
  } else {
    debug_println("No Carrier");
  }

  debug_print("Signal Strength: ");
  debug_print(scannerNetworks.getSignalStrength());
  debug_println(" [0-31]");

  debug_println("GSM Modem connected to network");

  return 1;
}

/*
  Send SMS Message 
 */
int sendSMS(char* phoneNumber, char* smsMessage) {
  int sent = 0;
  sms.beginSMS(phoneNumber);
  sms.print(smsMessage);
  sent = sms.endSMS();

  if ( sent == 1 ) {
    debug_println("SMS sent.");
  } else {
    debug_println("ERROR: SMS not sent");
  }
  
  return sent;
}

