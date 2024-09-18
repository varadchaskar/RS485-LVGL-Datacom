#include <Arduino.h>
#include <ModbusMaster.h>
#include <FS.h>
#include <SPI.h>
#include <lvgl.h>
#include <TFT_eSPI.h>

// Define RS-485 pins
#define RXD_PIN 9
#define TXD_PIN 10

// Create a ModbusMaster object
ModbusMaster node;

void readPLCData() {
  uint8_t result;
  uint16_t data;

  // Read a holding register (e.g., address 0x02)
  result = node.readHoldingRegisters(0x02, 1);

  if (result == node.ku8MBSuccess) {
    data = node.getResponseBuffer(0); // Get data from the register
    Serial.print("PLC Data: ");
    Serial.println(data); // Output data to Serial Monitor
  } else {
    Serial.println("Error reading data");
  }
}

void setup() {
  Serial.begin(115200); // For debugging to Serial Monitor

  // Initialize RS-485 serial communication
  Serial1.begin(115200, SERIAL_8N1, RXD_PIN, TXD_PIN); // RXD_PIN and TXD_PIN are connected to RS-485 converter

  // Initialize Modbus communication with slave ID 1
  node.begin(1, Serial1); // 1 is the Modbus slave ID (change if needed)
}

void loop() {
  readPLCData();
}

