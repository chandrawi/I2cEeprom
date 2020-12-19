#include <I2cEeprom.h>
#define EEPROM_ADDRESS 0x50           // default I2C address of 24LC256 Eeprom
#define EEPROM_SIZE 256               // 256 kbit size

// Initialize Eeprom library to use
I2cEeprom Eeprom(EEPROM_ADDRESS);
//I2cEeprom Eeprom(EEPROM_ADDRESS, EEPROM_SIZE);    // initialize with known Eeprom size

void setup() {
  // Begin serial communication
  Serial.begin(38400);
  
  // Begin the Eeprom
  Eeprom.begin();
  //Eeprom.begin(400000);                           // for use in fast wire (400 kHz)
  
  // Uncomment code below to set page size and maximum writing delay time
  //Eeprom.setPageSize(64);                         // 64 bytes page size
  //Eeprom.setWriteTime(5000);                      // 5000 microsecond write time delay
  
  // eprom address
  unsigned int addressInit = 0x0040;
  unsigned int address = addressInit;
  byte i;
  
  // Single character to write
  char theChar = 'C';
  
  // Array of byte number to write
  byte dataNum[10] = {0, 11, 22, 33, 44, 55, 66, 77, 88, 99};
  const byte lenNum = sizeof(dataNum);
  // Variable to contain reading number data
  byte readNum[lenNum];
  
  // Array of character (string) to write
  char dataChar[] = "qwertyuiopasdfghjklzxcvbnm";
  const byte lenChar = sizeof(dataChar) - 1;
  // Variable to contain reading character data
  char readChar[lenChar];
  
  // Backup original data, so don't worry to losing data in Eeprom
  Serial.println("--- BACKUP ORIGINAL DATA ---");
  const byte orgLen = lenNum + lenChar + 1;
  byte orgData[orgLen];
  if ((Eeprom.read(addressInit, orgData, orgLen)) == orgLen){
    Serial.print("Original data: ");
    for (i=0; i<orgLen; i++) {
      Serial.write(" ");
      Serial.write(orgData[i]);
    }
  }else{
    Serial.print("Reading from Eeprom for backup data failed. Something wrong with Eeprom!");
    while(1);
  }
  Serial.println();
  
  // Write and read single character
  Serial.println("\n--- WRITE AND READ SINGLE CHARACTER ---");
  Serial.print("Character to write: ");
  Serial.println(theChar);
  Eeprom.write(address, theChar);                                // Write character 'C' to Eeprom
  Serial.print("Character read from Eeprom: ");
  theChar = Eeprom.read(address);                                // Read character previously written in Eeprom
  Serial.println(theChar);
  address++;                                                     // Move address for next writing
  
  // Write and read array of number
  Serial.println("\n--- WRITE AND READ ARRAY OF NUMBER ---");
  Serial.println("Byte numbers to write: ");
  for (i=0; i<lenNum; i++) {
    Serial.write("\t");
    Serial.print(dataNum[i], DEC);
  }
  Serial.println();
  Eeprom.write(address, dataNum, lenNum);                        // Write array of number to Eeprom
  Serial.println("Byte numbers read from Eeprom: ");
  Eeprom.read(address, readNum, lenNum);                         // Read array of number previously written in Eeprom
  for (i=0; i<lenNum; i++) {
    Serial.write("\t");
    Serial.print(readNum[i], DEC);
  }
  Serial.println();
  address+=lenNum;                                               // Move address for next writing
  
  // Write and read string
  Serial.println("\n--- WRITE AND READ STRING ---");
  Serial.print("String to write:         ");
  Serial.println(dataChar);
  Eeprom.write(address, dataChar, lenChar);                      // Write string to Eeprom
  Serial.print("String read from Eeprom: ");
  Eeprom.read(address, readChar, lenChar);                       // Read string previously written in Eeprom
  Serial.println(readChar);
  
  // Restore original data
  Serial.println("\n--- RESTORE ORIGINAL DATA ---");
  if (Eeprom.write(addressInit, orgData, orgLen)){
    Serial.print("Restore data success!");
  }else{
    Serial.print("Restore data failed. Something wrong with Eeprom!");
  }
}

void loop() {
  // Nothing to do in loop
}
