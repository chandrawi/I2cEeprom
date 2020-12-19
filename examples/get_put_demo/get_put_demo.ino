#include <I2cEeprom.h>
#define EEPROM_ADDRESS 0x50           // default I2C address of 24LC256 Eeprom
#define EEPROM_SIZE 256               // 256 kbit size

// Initialize Eeprom library to use
I2cEeprom Eeprom(EEPROM_ADDRESS);
//I2cEeprom Eeprom(EEPROM_ADDRESS, EEPROM_SIZE);    // initialize with known Eeprom size

struct myObject {
  float field0;
  int field1;
  char field2[10];
};

void setup() {
  // Begin serial communication
  Serial.begin(38400);
  
  // Begin the Eeprom
  Eeprom.begin();
  //Eeprom.begin(400000L);                          // for use in fast wire (400 kHz)
  
  // Uncomment code below to set page size and maximum writing delay time
  //Eeprom.setPageSize(64);                         // 64 bytes page size
  //Eeprom.setWriteTime(5000);                      // 5000 microsecond write time delay
  
  // eprom address
  unsigned int addressInit = 0x0080;
  unsigned int address = addressInit;
  byte i;

  // Backup original data, so don't worry to losing data in Eeprom
  Serial.println("--- BACKUP ORIGINAL DATA ---");
  const byte orgLen = 20;
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

  double dbl = 99.99;
  double dblRead;
  Serial.println("\n--- GET AND PUT DOUBLE VARIABLE ---");
  Serial.print("Number to write: ");
  Serial.println(dbl);
  Eeprom.put(address, dbl);
  Eeprom.get(address, dblRead);
  Serial.print("Number read from Eeprom: ");
  Serial.println(dblRead);
  address += sizeof(double);
  
  myObject writeVar = {
    3.14f,
    6285,
    "Working!"
  };
  myObject readVar;
  Serial.println("\n--- GET AND PUT CUSTOM OBJECT ---");
  Eeprom.put(address, writeVar);
  Eeprom.get(address, readVar);
  Serial.println("Get custom object from Eeprom: ");
  Serial.println(readVar.field0);
  Serial.println(readVar.field1);
  Serial.println(readVar.field2);
  
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