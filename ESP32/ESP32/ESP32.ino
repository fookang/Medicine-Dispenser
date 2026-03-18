#include <Arduino.h>
#include "constant.h"

const int NEW_TX_PIN = 17;
const int NEW_RX_PIN = 18;

void printPacket(const TPacket &pkt)
{
  Serial.println("Packet received:");
  Serial.print("  device_type = ");
  Serial.println(pkt.device_type);

  Serial.print("  command = ");
  Serial.println(pkt.command);

  Serial.print("  data = ");
  Serial.println((const char *)pkt.data);
  Serial.println();
}

void sendTestPacket()
{
  TPacket pkt = {};
  pkt.device_type = HB_SENSOR_DEV;
  pkt.command = CMD_NONE;

  snprintf((char *)pkt.data, MAX_DATA_LEN, "%d", 72);

  Serial1.write((uint8_t *)&pkt, sizeof(TPacket));

  Serial.println("Packet sent:");
  Serial.print("  device_type = ");
  Serial.println(pkt.device_type);
  Serial.print("  command = ");
  Serial.println(pkt.command);
  Serial.print("  data = ");
  Serial.println((char *)pkt.data);
  Serial.println();
}

void setup()
{
  Serial.begin(9600);
  Serial1.begin(9600, SERIAL_8N1, NEW_RX_PIN, NEW_TX_PIN);
  delay(1000);
  Serial.println("ESP32 UART packet test");
}

void loop()
{
  static unsigned long lastSend = 0;

  if (Serial1.available() >= sizeof(TPacket))
  {
    TPacket pkt;
    Serial1.readBytes((char *)&pkt, sizeof(TPacket));

    pkt.data[MAX_DATA_LEN - 1] = '\0';
    printPacket(pkt);
  }

  if (millis() - lastSend > 2000)
  {
    lastSend = millis();
    sendTestPacket();
  }
}
