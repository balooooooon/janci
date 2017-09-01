/*
 * @return boolean
 */
boolean initializeSD() {
  pinMode(CS_PIN, OUTPUT);
  return SD.begin(CS_PIN);
}

/*
 * Open file on SD card. Create if file does not exist.
 * @return boolean
 */
boolean createFile(char filename[]) {
  file = SD.open(filename, FILE_WRITE);
  debug_println(file);
  if(file) return true;
  else return false;
}

/* 
 * @return int Number of bytes written
 */
int writeToFileFloat(float number) {
  if (file) return (int)file.println(number);
  else return 0;
}

/* 
 * @return int Number of bytes written
 */
int writeToFile(char text[]) {
  if (file) return (int)file.print(text);
  else return 0;
}

/* 
 *  @return boolean 
 */
boolean closeFile() {
  if (file) {
    file.close();
    return true;
  } else return false;
}
