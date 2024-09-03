// Feather9x_TX
// -*- mode: C++ -*-
// Example sketch showing how to create a simple messaging client (transmitter)
// with the RH_RF95 class. RH_RF95 class does not provide for addressing or
// reliability, so you should only use RH_RF95 if you do not need the higher
// level messaging abilities.
// It is designed to work with the other example Feather9x_RX

#include <SPI.h>
#include <RH_RF95.h>

// for feather32u4 
#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 7

#define PHOTOCELL_PIN 20

#define DANGER_THRESHOLD 40
#define DANGER_FADE_TIME 2000
#define CLEAR_SEND_WAIT 500
#define DANGER_SEND_WAIT 50

// Change to 434.0 or other frequency, must match RX's freq!
#define RF95_FREQ 433.0

// Singleton instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);

enum Message 
{
  CLEAR = 0,
  DANGER = 1
};

void setup() 
{
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


bool danger = false; 
unsigned long lastDangerTime = 0;
unsigned long lastSendTime = 0;

void sendMessage(Message message)
{
  uint8_t data = message;
  rf95.send(&data, sizeof(data));
  rf95.waitPacketSent();
  lastSendTime = millis();
}

void loop()
{
  uint16_t lightAmount = analogRead(PHOTOCELL_PIN);
  if(Serial)
  {
    Serial.print("Analog reading = ");
    Serial.println(lightAmount);     // the raw analog reading
  }

  if(lightAmount > DANGER_THRESHOLD) {
    danger = true;
    lastDangerTime = millis();
  }
  else if(millis() - lastDangerTime > DANGER_FADE_TIME) {
    danger = false;
  }

  if(danger && millis() - lastSendTime > DANGER_SEND_WAIT) {
    Serial.println("sending DANGER");
    sendMessage(Message::DANGER);
  }
  else if(millis() - lastSendTime > CLEAR_SEND_WAIT){
    Serial.println("sending CLEAR");
    sendMessage(Message::CLEAR);
  }
}
