#ifndef FileHandler
#define FileHandler

#include <SD.h>

File file;

boolean initializeSD();
boolean createFile(char filename[]);
int writeToFileFloat(float number);
int writeToFile(char text[]);
boolean closeFile();

#endif
