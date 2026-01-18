#include <SoftwareSerial.h>

#include "wheels.h"
#include "sensors.h"
#include "gps_sd.h"
#include "bluetooth.h"
#include "wifi_control.h"

// Bluetooth settings
SoftwareSerial bluetooth(0, 1); // RX, TX
const byte STATE_PIN = 7;
char data = '0';

// Speed
byte Ena = 6;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT); // LED

  Serial.begin(9600);
  bluetooth.begin(38400);
  pinMode(STATE_PIN, INPUT);

  wheels_init(Ena);
  sensors_init();
  gps_sd_init();
  wifi_init();
}

void loop() {
  static double sensors_data[3];

  sensors_read(sensors_data);

  wifi_send_data(sensors_data);

if (digitalRead(State)) {
  digitalWrite(LED_BUILTIN, HIGH);

  wifi_handle_incoming_commands();

} else {
 
  digitalWrite(LED_BUILTIN, LOW);

  if (bluetooth.available()) {
    char cmd = bluetooth.read();
    handle_wheel_command(cmd);
  }
}
gps_sd_update(sensors_data);
}
