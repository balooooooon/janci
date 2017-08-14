void initializeSD() {
  pinMode(CS_PIN, OUTPUT);
  SD.begin(CS_PIN);
}

int createFile(char filename[]) {
  file = SD.open(filename, FILE_WRITE);

  //Serial.println(file);
  if (file) {
    return 1;
  } else {
    return 0;
  }
}

int writeToFileFloat(float number) {
  if (file) {
    file.println(number);
    return 1;
  } else {
    return 0;
  }
}

int writeToFile(char text[]) {
  if (file) {
    file.print(text);
    return 1;
  } else {
    return 0;
  }
}

void closeFile() {
  if (file) file.close();
}
