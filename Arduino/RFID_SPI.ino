#include <Wire.h>
#include <Adafruit_PN532.h>

// Define pins for SPI
#define SCK_PIN 13   // SPI Clock
#define MISO_PIN 12  // Master In Slave Out
#define MOSI_PIN 11  // Master Out Slave In
#define SS_PIN 10    // Slave Select (CS)
#define BUZZER_PIN 9
#define LED_1 3
#define LED_2 4



// Create an instance of Adafruit_PN532 using SPI
Adafruit_PN532 nfc(SS_PIN);

// Variables to track the last detected tag
uint8_t lastUid[7] = {0};    // Buffer for last UID
uint8_t lastUidLength = 0;   // Length of last UID
bool tagDetected = false;    // Tracks if the tag is already detected

void setup() {
  Serial.begin(115200);
  Serial.println("Initializing PN532 RFID Module (SPI Mode)...");

  pinMode(BUZZER_PIN,OUTPUT);

  // Initialize the RFID module
  nfc.begin();

  // Check the firmware version
  uint32_t versiondata = nfc.getFirmwareVersion();
  if (!versiondata) {
    Serial.println("Error: Could not find PN532 RFID module. Check your connections.");
    
    while (1); // Halt if the module is not detected
  }

  // Display firmware information
  Serial.print("Found chip PN5"); Serial.println((versiondata >> 24) & 0xFF, HEX);
  Serial.print("Firmware Version: ");
  Serial.print((versiondata >> 16) & 0xFF, DEC);
  Serial.print('.');
  Serial.println((versiondata >> 8) & 0xFF, DEC);

  // Configure the PN532 module to read RFID tags
  nfc.SAMConfig();
  Serial.println("Ready to scan for RFID tags...");
}

void loop() {
  uint8_t success;           // Status flag
  uint8_t uid[7] = {0};      // Buffer for the UID
  uint8_t uidLength;         // Length of the UID

  // Attempt to read an RFID tag
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);


  if (success) {
    // Compare with the last detected UID
    if (!tagDetected || !isSameTag(uid, uidLength)) {
      tagDetected = true;

      // Log the detected tag
      Serial.println("\nNew RFID tag detected!");
      digitalWrite(LED_2, HIGH);
      ringActiveBell();
      ringActiveBell();
      
      printUid(uid, uidLength);

      // Call action for the detected tag
      handleTag(uid, uidLength);

      // Save the detected tag as the last UID
      saveLastUid(uid, uidLength);
    }
  } else {
    // No tag detected, reset flag if necessary
    if (tagDetected) {
      Serial.println("No RFID tag in range...");
      digitalWrite(LED_2, LOW);
      ringPassiveBell();
      tagDetected = false;
    }
  }
  // Optional: Small delay to reduce CPU usage
  delay(200);
}

void printUid(uint8_t *uid, uint8_t uidLength) {
  Serial.print("UID Length: ");
  Serial.print(uidLength);
  Serial.println(" bytes");

  Serial.print("UID Value: ");
  for (uint8_t i = 0; i < uidLength; i++) {
    Serial.print("0x");
    Serial.print(uid[i], HEX);
    if (i < uidLength - 1) {
      Serial.print(" ");
    }
  }
  Serial.println();
}

void handleTag(uint8_t *uid, uint8_t uidLength) {
  // Check for a specific tag (example: 0x04 0x89 0x5A 0x3C)
  if (uidLength == 4 && uid[0] == 0x04 && uid[1] == 0x89 && uid[2] == 0x5A && uid[3] == 0x3C) {
    Serial.println("Action: Recognized authorized tag!");
    // Add custom actions for this tag here
  } else {
    Serial.println("Action: Unrecognized tag. Logging...");
    // Add actions for unrecognized tags here
  }
}
 
bool isSameTag(uint8_t *uid, uint8_t uidLength) {
  if (uidLength != lastUidLength) {
    return false; // Different length means a different tag
  }
  for (uint8_t i = 0; i < uidLength; i++) {
    if (uid[i] != lastUid[i]) {
      return false; // Any mismatch means a different tag
    }
  }
  return true; // All bytes match
}

void saveLastUid(uint8_t *uid, uint8_t uidLength) {
  lastUidLength = uidLength;
  for (uint8_t i = 0; i < uidLength; i++) {
    lastUid[i] = uid[i];
  }
}

void ringPassiveBell(){
  tone(BUZZER_PIN, 1000);  // Play a 1000 Hz tone
  delay(1000);             // Wait for 1 second
  noTone(BUZZER_PIN);      // Stop the tone
  delay(1000);             // Wait for 1 second
}

void ringActiveBell(){
  digitalWrite(BUZZER_PIN, HIGH);
  blinker();
  blinker();  // Turn the buzzer on
  delay(50);                     // Wait for 500 ms
  digitalWrite(BUZZER_PIN, LOW);   // Turn the buzzer off
  delay(50);                     // Wait for 500 ms
}


void blinker(){
  digitalWrite(LED_1,HIGH);
  delay(50);
  digitalWrite(LED_1,LOW);
  delay(50);
}