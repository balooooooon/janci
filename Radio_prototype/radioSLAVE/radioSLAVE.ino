#include <string.h>
#include <util/crc16.h>

#define RADIOPIN 12

char strBuffer[100];
char dataString[100];

void f_blink(int count) {
  for ( int i = 0; i < count; i++ ) {
    digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
    delay(500);                       // wait for a second
    digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
    delay(500);
  }    
}

void rtty_txstring (char * string) {
 
  /* Simple function to sent a char at a time to 
     ** rtty_txbyte function. 
    ** NB Each char is one byte (8 Bits)
    */
 
  char c;
 
  c = *string++;
 
  while ( c != '\0') {
    rtty_txbyte (c);
    c = *string++;
  }
}

void rtty_txbit (int bit)
{
  if (bit) { // high
    digitalWrite(RADIOPIN, HIGH);
  } else { // low
    digitalWrite(RADIOPIN, LOW);
  }
 
  delayMicroseconds(3370); // 300 baud
  
  //delayMicroseconds(10000); 
  //delayMicroseconds(10150); 

  // For 50 Baud uncomment this and the line below. 
  // You can't do 20150 it just doesn't work as the
  // largest value that will produce an accurate delay is 16383
  // See : http://arduino.cc/en/Reference/DelayMicroseconds
}
 
void rtty_txbyte (char c)
{
  /* Simple function to sent each bit of a char to 
    ** rtty_txbit function. 
    ** NB The bits are sent Least Significant Bit first
    **
    ** All chars should be preceded with a 0 and 
    ** proceded with a 1. 0 = Start bit; 1 = Stop bit
    **
    */
 
  int i;
 
  rtty_txbit (0); // Start bit
 
  // Send bits for for char LSB first 
 
  for (i=0;i<7;i++) // Change this here 7 or 8 for ASCII-7 / ASCII-8 
  {
    if (c & 1) rtty_txbit(1); 
 
    else rtty_txbit(0); 
 
    c = c >> 1;
 
  }
 
  rtty_txbit (1); // Stop bit
  rtty_txbit (1); // Stop bit
}
 

 
uint16_t gps_CRC16_checksum (char *string) {
  size_t i;
  uint16_t crc;
  uint8_t c;
 
  crc = 0xFFFF;
 
  // Calculate checksum ignoring the first two $s
  for (i = 2; i < strlen(string); i++) {
    c = string[i];
    crc = _crc_xmodem_update (crc, c);
  }
 
  return crc;
} 

void sendRadio(int msgLength) {
  sprintf(dataString,strBuffer); // Puts the text in the datastring
  
  unsigned int CHECKSUM = gps_CRC16_checksum(dataString);  // Calculates the checksum for this datastring
  char checksum_str[6];
  sprintf(checksum_str, "*%04X\n", CHECKSUM);
  strcat(dataString,checksum_str);
 
  rtty_txstring (dataString);
  
  delay(1000);
}

void setup() {
  Serial.begin(9600);
  pinMode(RADIOPIN,OUTPUT);
}

void loop() {
  
  int i = 0;
  if (Serial.available()) {
    delay(100); //allows all serial sent to be received together
    
    while(Serial.available()) {
      strBuffer[i++] = Serial.read();
      if( strBuffer[i-1] == '\0' ) break;
    }
  }

  if( i > 0 ) {
    f_blink(2);
    sendRadio(i);
  } else {
    f_blink(3);
   }
}

  


