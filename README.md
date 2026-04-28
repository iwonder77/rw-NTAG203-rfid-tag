# RFID Tag Programmer — Jumper Cable Interactive

Utility sketch for programming and verifying NTAG213 RFID tags used in the **Jumper Cable Interactive** exhibit (Auto Shop, Kidopolis — Thanksgiving Point Museum of Natural Curiosity).

The exhibit teaches guests how to jump-start a car. Each physical jumper cable end carries an NTAG213 tag that identifies it as positive or negative. This tool writes and verifies that identity data before tags are deployed.

Exhibit firmware repo: [jumper-cable-rfid-interactive](https://github.com/iwonder77/jumper-cable-rfid-interactive)

## Overview

The sketch runs on an **Adafruit Trinket M0** connected to an **M5Stack RFID2 reader** over I2C. On power-up it presents a serial menu. The technician selects which cable end to program (POS-1, POS-2, NEG-3, or NEG-4), places a blank NTAG213 tag on the reader, and the sketch writes a 6-byte struct to the tag's user memory, then reads it back to verify byte-for-byte before returning to the menu. A separate scan mode lets you inspect any already-programmed tag.

## Hardware List

| Component       | Part                                                                                  |
| --------------- | ------------------------------------------------------------------------------------- |
| Microcontroller | [Adafruit Trinket M0](https://www.adafruit.com/product/3500)                          |
| RFID Reader     | [M5Stack RFID2 Unit (WS1850S)](https://shop.m5stack.com/products/rfid-unit-2-ws1850s) |
| RFID Tags       | [Adafruit NTAG213 sticker tags](https://www.adafruit.com/product/5458)                |
| Cable           | USB-A to Micro-USB (programming + power)                                              |

### Wiring (I2C)

| RFID2 Grove Pin | Trinket M0 Pin |
| --------------- | -------------- |
| GND             | GND            |
| VCC             | 3V             |
| SDA             | 0 (SDA)        |
| SCL             | 2 (SCL)        |

## Software Architecture

### Board

Board package: **Adafruit SAMD Boards** → `Adafruit Trinket M0`

Install via **Boards Manager**: search `Adafruit SAMD` and install the Adafruit SAMD package.

### Tag Data Layout

Each tag stores a single 6-byte struct starting at NTAG213 **page 4** (first writable user-data page):

```cpp
struct JumperCableData {
    char    type[4];    // "POS\0" or "NEG\0"
    uint8_t id;         // 1, 2, 3, or 4
    uint8_t checksum;   // XOR of the preceding 5 bytes
};
```

The struct spans **pages 4 and 5** (4 bytes per page). The XOR checksum covers all fields except itself, so any single-byte corruption is detectable on read-back.

### Firmware State Machine

```
MENU ──1──▶ WRITE_POS1 ──done──▶ MENU
     ──2──▶ WRITE_POS2 ──done──▶ MENU
     ──3──▶ WRITE_NEG3 ──done──▶ MENU
     ──4──▶ WRITE_NEG4 ──done──▶ MENU
     ──5──▶ SCAN ────────'m'───▶ MENU
```

Write modes block until a tag is detected, write the struct, verify, then return to MENU automatically. Scan mode loops continuously until you send `m`.

### Key Implementation Notes

- The RFID2 module uses the WS1850S chip at I2C address `0x28`. This chip is functionally compatible with MFRC522 command set but reports a different version register, so PCD version checking is bypassed in the driver.
- `MIFARE_Read` always returns 16 data bytes + 2 CRC bytes — the read buffer must be **at least 18 bytes**.
- `MIFARE_Ultralight_Write` writes exactly **4 bytes per call** (one page). The 6-byte struct requires two write calls (pages 4 and 5).

## Step-by-Step: Programming Tags

1. Wire the RFID2 module to the Trinket M0 per the table above.
2. Open this sketch in the Arduino IDE.
3. Install the `MFRC522v2` library (Library Manager → search `MFRC522v2` by OSSLibraries).
4. Select **Tools → Board → Adafruit SAMD → Adafruit Trinket M0**.
5. Select the correct port under **Tools → Port**.
6. Upload the sketch.
7. Open **Serial Monitor** at **115200 baud**, line ending set to **No line ending** (or Newline — the sketch strips `\r`/`\n`).
8. The menu appears:
   ```
   ====== RFID Tag Writer ======
   1) Write POS-1 tag
   2) Write POS-2 tag
   3) Write NEG-3 tag
   4) Write NEG-4 tag
   5) Scan tag
   =============================
   ```
9. Send `1` through `4` to program a tag. Place the blank NTAG213 tag flat on the RFID2 reader when prompted.
10. Wait for `Verifying... OK` before removing the tag.
11. Repeat for each of the 4 cable ends (POS-1, POS-2, NEG-3, NEG-4).
12. Use option `5` (Scan) to confirm any tag reads back correctly. Send `m` to exit scan mode.

> **Tag programming order:** The exhibit has 4 jumper cable ends — 2 positive (red, IDs 1 & 2) and 2 negative (black, IDs 3 & 4). Program one tag per cable end and attach it before reinstalling into the exhibit.

## Troubleshooting

**`Failed to write page X`**

- Tag not fully seated on the reader — reposition and retry.
- Tag may already be locked (write-protected). Use a fresh tag.

**`Verifying... FAILED — Read-back does not match`**

- Intermittent RF contact. Keep the tag still during the write + verify sequence.
- Try a different tag; occasionally tags arrive defective.

**`Checksum mismatch — data may be corrupted`** (in Scan mode)

- Tag was only partially written (e.g., removed early). Reprogram it.
- Tag is blank or holds unrelated data from a previous use.

**Menu doesn't appear / no serial output**

- Confirm baud rate is **115200** in Serial Monitor.
- The Trinket M0 waits for a serial connection before proceeding (`while (!Serial);`). Some serial monitors require you to open the port before output appears.

**I2C device not found / reader doesn't initialize**

- Check wiring: SDA → pin 0, SCL → pin 2 on the Trinket M0.
- Confirm 3V supply (do not connect VCC to 5V — the WS1850S is 3.3V).
- Run an I2C scanner sketch to confirm the module appears at address `0x28`.

**Board not detected by computer**

- The Trinket M0 requires the **Adafruit SAMD** board package. If the port doesn't appear, double-tap the reset button to enter the UF2 bootloader and try again.
