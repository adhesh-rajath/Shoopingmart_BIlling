/*
 * --------------------------------------------------------------------------------------------------------------------
 * Example sketch/program showing how to read new NUID from a PICC to serial.
 * --------------------------------------------------------------------------------------------------------------------
 * This is a MFRC522 library example; for further details and other examples see: https://github.com/miguelbalboa/rfid
 * 
 * Example sketch/program showing how to the read data from a PICC (that is: a RFID Tag or Card) using a MFRC522 based RFID
 * Reader on the Arduino SPI interface.
 * 
 * When the Arduino and the MFRC522 module are connected (see the pin layout below), load this sketch into Arduino IDE
 * then verify/compile and upload it. To see the output: use Tools, Serial Monitor of the IDE (hit Ctrl+Shft+M). When
 * you present a PICC (that is: a RFID Tag or Card) at reading distance of the MFRC522 Reader/PCD, the serial output
 * will show the type, and the NUID if a new card has been detected. Note: you may see "Timeout in communication" messages
 * when removing the PICC from reading distance too early.
 * 
 * @license Released into the public domain.
 * 
 * Typical pin layout used:
 * -----------------------------------------------------------------------------------------
 *             MFRC522      Arduino       Arduino   Arduino    Arduino          Arduino
 *             Reader/PCD   Uno/101       Mega      Nano v3    Leonardo/Micro   Pro Micro
 * Signal      Pin          Pin           Pin       Pin        Pin              Pin
 * -----------------------------------------------------------------------------------------
 * RST/Reset   RST          9             5         D9         RESET/ICSP-5     RST
 * SPI SS      SDA(SS)      10            53        D10        10               10
 * SPI MOSI    MOSI         11 / ICSP-4   51        D11        ICSP-4           16
 * SPI MISO    MISO         12 / ICSP-1   50        D12        ICSP-1           14
 * SPI SCK     SCK          13 / ICSP-3   52        D13        ICSP-3           15
 *
 * More pin layouts for other boards can be found here: https://github.com/miguelbalboa/rfid#pin-layout
 */

#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 10
#define RST_PIN 9
#define TRIG_PIN 7
#define ECHO_PIN 6
#define BUZZER_PIN 4
MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class

MFRC522::MIFARE_Key key; 

// Init array that will store new NUID 
byte nuidPICC[4];

struct Map {
  String key;
  String value;
  int rate;
};

int toStart=1;
int toStop=1;

Map item_list[4] = {
  {"690075B2", "Pen", 30},
  {"B3E537FB", "Bottle", 8000},
  {"63AC98C5", "Book", 6000},
  {"F99BE4D5", "Burger", 10000}
};
long distance = 1000;
int check=1;
void setup() { 
  Serial.begin(9600);
  SPI.begin(); // Init SPI bus
  rfid.PCD_Init(); // Init MFRC522 

  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  Serial.println(F("This code scan the MIFARE Classsic NUID."));
  Serial.print(F("Using the following key:"));
  printHex(key.keyByte, MFRC522::MF_KEY_SIZE);

}
 
void loop() {
long duration;
  if(check)
  {
  digitalWrite(TRIG_PIN, LOW);  
  delayMicroseconds(2); 
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10); 
  digitalWrite(TRIG_PIN, LOW);
  duration = pulseIn(ECHO_PIN, HIGH);
  distance = (duration/2) / 29.1;
  toStart=1;
  toStop=1;
  if (distance >30){
    Serial.println("Out of range");
  }
  else {
    Serial.println("Distance: "+String(distance));
  }
  }
  if(distance<=30 && distance!=0)
  {
    if(toStart)
    {
      Serial.println("open");
      toStart=0;
    }
    if (Serial.available()) {
    // Read the incoming line
    String line = Serial.readStringUntil('\n');

    // If the line is "button clicked", perform the action
    if (line == "button clicked") {
      check=1;
    }
  }
  // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
  if ( ! rfid.PICC_IsNewCardPresent())
    return;

  // Verify if the NUID has been readed
  if (rfid.PICC_ReadCardSerial()) {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(1000); // Beep for 100 ms
    digitalWrite(BUZZER_PIN, LOW);
    check=0;
    }
    else
    {
      return;
    }
  Serial.print(F("PICC type: "));
  MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
  Serial.println(rfid.PICC_GetTypeName(piccType));

  // Check is the PICC of Classic MIFARE type
  if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&  
    piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
    piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
    Serial.println(F("Your tag is not of type MIFARE Classic."));
    return;
  }
    
  if (rfid.uid.uidByte[0] != nuidPICC[0] || 
    rfid.uid.uidByte[1] != nuidPICC[1] || 
    rfid.uid.uidByte[2] != nuidPICC[2] || 
    rfid.uid.uidByte[3] != nuidPICC[3] ) {
    Serial.println(F("A new card has been detected."));

    // Store NUID into nuidPICC array
    for (byte i = 0; i < 4; i++) {
      nuidPICC[i] = rfid.uid.uidByte[i];
    }
   
    Serial.println(F("The NUID tag is:"));
    String cardID=ToHex(rfid.uid.uidByte, rfid.uid.size);
    cardID.toUpperCase();
    Serial.println(cardID);
  for (int i = 0; i < 4; i++) {
    if (cardID == item_list[i].key) {
      Serial.println("Item: " + item_list[i].value);
      Serial.println("Rate: " + String(item_list[i].rate));
    }
  }
  }
  else Serial.println(F("Card read previously."));

  // Halt PICC
  rfid.PICC_HaltA();

  // Stop encryption on PCD
  rfid.PCD_StopCrypto1();
  }
  else
  {
    if(distance!=0)
    {
    if(toStop)
    {
      Serial.println("close");
      toStop=0;
    }
    }
  }
}


/**
 * Helper routine to dump a byte array as hex values to Serial. 
 */
String ToHex(byte *buffer, byte bufferSize) {
  String hexStr = "";
  for (byte i = 0; i < bufferSize; i++) {
    if (buffer[i] < 0x10)
      hexStr += "0";
    hexStr += String(buffer[i], HEX);
  }
    return hexStr;
}
void printHex(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}
/**
 * Helper routine to dump a byte array as dec values to Serial.
 */