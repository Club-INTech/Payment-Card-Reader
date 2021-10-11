#include <MFRC522.h>
#include <SPI.h>

MFRC522 mfrc522(10, 9);
MFRC522::MIFARE_Key key;

void setup() {
  Serial.begin(9600);
  SPI.begin();
  mfrc522.PCD_Init();
  Serial.println("BEGIN");

  memset(key.keyByte, 0xff, 6);
}

void loop() {
  delay(500);

  uint8_t buf[18] = {0};
  uint8_t buf_size = sizeof buf;
  Serial.println("hey");

  if (!mfrc522.PICC_IsNewCardPresent()) {
    Serial.println("No card detected");
    return;
  }

  if (!mfrc522.PICC_ReadCardSerial()) {
    Serial.println("Couldn't read card serial");
    return;
  }

  if (mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 0, &key, &(mfrc522.uid)) != MFRC522::STATUS_OK) {
    Serial.println("Couldn't authenticate");
    return;
  }

  for (size_t i = 0; i < 4; i++) {
    if (mfrc522.MIFARE_Read(i, buf, &buf_size) != MFRC522::STATUS_OK) {
      Serial.println("Can't read card content");
      return;
    }

    for (const auto &value : buf) {
      if (value <= 0xf)
        Serial.print(0);
      Serial.print(value, HEX);
    }
    Serial.println();
  }

  mfrc522.PCD_StopCrypto1();
}
