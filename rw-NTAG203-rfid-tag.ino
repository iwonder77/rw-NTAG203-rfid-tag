/* 
* ----------------------------------------------
* PROJECT NAME: writing-to-rfid-tag
* Description: programming RFID tags with pos/neg cable identification for jumper cable interactive
* 
* Author: Isai Sanchez
* Date: 8-13-25
* Board Used: Arduino Nano
* Libraries:
*   - Wire.h (I2C communication library): https://docs.arduino.cc/language-reference/en/functions/communication/wire/
*   - MFRC522v2.h (Main RFID library): https://github.com/OSSLibraries/Arduino_MFRC522v2
* Hardware:
*   - Aruino Nano
*   - M5Stack RFID2 reader
* Notes:
*   - Version checking of RFID2 reader bypassed due to WS1850S/MFRC522 differences
* ----------------------------------------------
*/

#include <Wire.h>
#include <MFRC522v2.h>
#include <MFRC522DriverI2C.h>
#include <MFRC522Debug.h>

// ----- RFID setup  -----
const uint8_t RFID2_WS1850S_ADDR = 0x28;

MFRC522DriverI2C driver{ RFID2_WS1850S_ADDR, Wire };
MFRC522 reader{ driver };

struct JumperCableData {
  char type[8];      // either "POS" or "NEG"
  uint8_t id;        // 1, 2, 3, or 4 (for the 4 cable ends)
  uint8_t checksum;  // simple validation
};

// ===== UTILITY FUNCTIONS =====
uint8_t calculateChecksum(const uint8_t* data, uint8_t length) {
  uint8_t sum = 0;
  for (uint8_t i = 0; i < length; i++) {
    sum ^= data[i];  // XDR checksum
  }
  return sum;
}

bool isCardPresent() {
  return (reader.PICC_IsNewCardPresent() && reader.PICC_ReadCardSerial());
}

// ===== READ

void setup() {
  Serial.begin(115200);
  Wire.begin();  // Initialize I2C with default SDA and SCL pins.

  reader.PCD_Init();                                      // Init MFRC522 board.
  MFRC522Debug::PCD_DumpVersionToSerial(reader, Serial);  // Show details of PCD - MFRC522 Card Reader details.
}

void loop() {
  // if no card OR can't read card, skip everything else (return early)
  if (!reader.PICC_IsNewCardPresent() || !reader.PICC_ReadCardSerial()) {
    return;
  }

  // if we make it here, a card was found and read successfully
  Serial.println("Card, detected!");

  // more detailed output to serial monitor, use:
  MFRC522Debug::PICC_DumpToSerial(reader, Serial, &(reader.uid));
}
