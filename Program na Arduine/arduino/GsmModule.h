#ifndef GsmModule
#define GsmModule

#include <GSM.h>

int gsmModuleInit();
int sendSMS(char* phoneNumber, char* smsMessage);

#endif
