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

// ===== READ PAGES 4-6 =====
void readPages4Thru6() {
  // ==============================
  // MIFARE_Read(byte blockAddr, byte *buffer, byte *bufferSize)
  // -- byte blockAddr:   replace with page num for NTAG203 tags
  // -- byte *buffer:     a buffer is a temporary array in RAM holding the bytes we read from the tag,
  //                      pass in a pointer to the first element of this array, so the function can fill
  //                      it with the tag's data
  // -- byte *bufferSize: size of the buffer, passed as reference to allow func to update it and tell us
  //                      how many bites it wrote into our buffer array
  //
  // NOTE: function returns 16 bytes (+ 2 bytes CRC_A) from the active PICC
  // -  active PICC means tag must be in the selected state (awake and communicating)
  // -  since function reads 16 bytes, and pages on our NTAG203 tags have 4 bytes each, this will read
  // -    4 pages on one go and store the information in our buffer (make it at least 18 though for the CRC bytes)
  //
  // RETURNS: a status code if read was successful
  // ==============================
  byte buffer[18];
  byte bufferSize = sizeof(buffer);

  Serial.println("Reading Pages 4,5,6");
  if (reader.MIFARE_Read(4, buffer, &bufferSize) == MFRC522::StatusCode::STATUS_OK) {
    for (byte page = 0; page < 3; page++) {  // only need pages 4,5,6
      Serial.print("Page ");
      Serial.print(4 + page);
      Serial.print(": ");
      for (byte j = 0; j < 4; j++) {
        Serial.print(buffer[page * 4 + j], HEX);
        Serial.print(" ");
      }
      Serial.println();
    }
  }
}

void setup() {
  Serial.begin(115200);
  Wire.begin();  // Initialize I2C with default SDA and SCL pins.

  reader.PCD_Init();  // Init MFRC522 board.
}

void loop() {
  // if no card OR can't read card, skip everything else (return early)
  if (!reader.PICC_IsNewCardPresent() || !reader.PICC_ReadCardSerial()) {
    return;
  }

  // if we make it here, a card was found and read successfully
  Serial.println("Card, detected!");

  readPages4Thru6();

  // more detailed output to serial monitor, use:
  MFRC522Debug::PICC_DumpToSerial(reader, Serial, &(reader.uid));
}
