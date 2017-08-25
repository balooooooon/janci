#ifndef FileHandler
#define FileHandler

#include <SD.h>

void initializeSD();
int createFile(char filename[]);
int writeToFileFloat(float number);
int writeToFile(char text[]);
void closeFile();

#endif
