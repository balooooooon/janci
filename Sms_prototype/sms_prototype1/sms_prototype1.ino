#include <GSM.h>
#define PINNUMBER "" // SIM Card PIN number
#define STATPIN 4   //Modem Status Pin
#define SIGNALLEDPIN 13 //LED indicator to show when connected to GSM network - treba zmenit cislo ledky
//Global classes
GSM gsmAccess;
GSMScanner scannerNetworks;
GSMModem modemTest;
GSMVoiceCall gsmVC;
GSM_SMS sms;

String currentCarrier = "";

String IMEI = "";


void setup() {
  // put your setup code here, to run once:
  
  Serial.begin(9600);

  Serial.println("Starting Modem...");
  
  pinMode(SIGNALLEDPIN,OUTPUT);
  digitalWrite(SIGNALLEDPIN,LOW);
  
  pinMode(STATPIN,INPUT);
  int iSTAT = 0;
  iSTAT = digitalRead(STATPIN);
 
  if(0 == iSTAT)
  {
     Serial.println("Modem in sleep mode, waiting for it to wake..."); 
   
     //This will cause the Library to start the modem up
     gsmAccess.begin(PINNUMBER);
  }
  
  iSTAT = digitalRead(STATPIN);
  if(0 == iSTAT)
  {
    Serial.println("Modem failed to power up");
  }
  else
  {
    Serial.println("Modem powered up");
  }
  
  
  //Get IMEI
  Serial.print("Modem IMEI: ");
  IMEI = modemTest.getIMEI();
  IMEI.replace("\n","");
  if(IMEI != NULL) Serial.println(IMEI);
  
  
  //Scan for network
  Serial.println("Searching for Network...");
  scannerNetworks.begin();
  bool notConnected = true;
  
  
  //Connect
  while(notConnected)
  {
    if(gsmAccess.begin(PINNUMBER) == GSM_READY)
    {
      notConnected = false;
      digitalWrite(SIGNALLEDPIN,HIGH);
    }
    else
    {
      Serial.println("Not connected");
      delay(1000);
    }
    
  }
    
  Serial.print("Current carrier: ");
  currentCarrier = scannerNetworks.getCurrentCarrier();
  currentCarrier.trim();
 
   
  if(currentCarrier.length() != 0)
  {
    Serial.println(currentCarrier);
  }
  else
  {
    Serial.println("No Carrier");
  }
  
  
  
  Serial.print("Signal Strength: ");
  Serial.print(scannerNetworks.getSignalStrength());
  Serial.println(" [0-31]");
  
  //Hang up any Voice Calls
  gsmVC.hangCall();
  
  
  Serial.println("");
  Serial.println("System Ready");
  Serial.println("Waiting for call");

}

void loop() {
  // put your main code here, to run repeatedly:
  SendSMS("ahoj");
  delay(1000000);
}




void SendSMS(char * ret)
{
 int iRet = 0; 
      sms.beginSMS("+421918084231");
      sms.print(ret);
      iRet = sms.endSMS();
      Serial.print("Sent SMS bytes: ");
      Serial.println(iRet);   
}
