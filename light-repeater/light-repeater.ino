// Feather9x_RX
// -*- mode: C++ -*-
// Example sketch showing how to create a simple messaging client (receiver)
// with the RH_RF95 class. RH_RF95 class does not provide for addressing or
// reliability, so you should only use RH_RF95 if you do not need the higher
// level messaging abilities.
// It is designed to work with the other example Feather9x_TX

#include <SPI.h>
#include <RH_RF95.h>

// for Feather32u4 RFM9x
#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 7


// Change to 434.0 or other frequency, must match RX's freq!
#define RF95_FREQ 433.0

// Singleton instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);

// Blinky on receipt
#define LED A3

unsigned long lastRecievedTime = 0;

enum Message 
{
  CLEAR = 0,
  DANGER = 1
};


bool danger = false;

void setup()
{
  pinMode(LED, OUTPUT);
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);

  Serial.begin(115200);

  // manual reset
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);

  while (!rf95.init()) {
    Serial.println("LoRa radio init failed");
    Serial.println("Uncomment '#define SERIAL_DEBUG' in RH_RF95.cpp for detailed debug info");
    while (1);
  }
  Serial.println("LoRa radio init OK!");

  // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM
  if (!rf95.setFrequency(RF95_FREQ)) {
    Serial.println("setFrequency failed");
    while (1);
  }
  Serial.print("Set Freq to: "); Serial.println(RF95_FREQ);

  // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on

  // The default transmitter power is 13dBm, using PA_BOOST.
  // If you are using RFM95/96/97/98 modules which uses the PA_BOOST transmitter pin, then
  // you can set transmitter powers from 5 to 23 dBm:
  rf95.setTxPower(23, false);
}


void loop()
{
  if (rf95.available())
  {
    // Should be a message for us now
    uint8_t data[1];
    uint8_t len = sizeof(data);

    if (rf95.recv(data, &len))
    {
      Message message = data[0];
      if(message == Message::CLEAR)
      {
        Serial.println("received CLEAR");
        danger = false;
      }
      else if(message == Message::DANGER)
      {
        Serial.println("received DANGER");
        danger = true;
      }

      lastRecievedTime = millis();

      sendMessage(message);
    }
    else
    {
      Serial.println("RECEIVE FAILED");
    }
  }

  // Update status LED if we haven't got data in a while.
  if(millis() - lastRecievedTime > 550)
  {
    digitalWrite(LED, LOW);
  }
  else {
    digitalWrite(LED, HIGH);
  }
}

void sendMessage(Message message)
{
  uint8_t data = message;
  rf95.send(&data, sizeof(data));
  rf95.waitPacketSent();
}
