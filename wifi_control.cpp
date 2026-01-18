#include "wifi_control.h"
#include <SoftwareSerial.h>
#include <Arduino.h>
#include "sensors.h"

SoftwareSerial wifiSerial(15, 14);
static bool destination_set = false;
static double dest_lat = 0.0, dest_lon = 0.0;

// Объявления внешних переменных
extern File file;
extern TinyGPS gps;
extern SoftwareSerial ssgps;

void send_at_command(const char* cmd, unsigned long timeout) {
  wifiSerial.println(cmd);
  delay(100);
}

bool wait_for_response(const char* expected, unsigned long timeout) {
  unsigned long start = millis();
  String response = "";
  while (millis() - start < timeout) {
    if (wifiSerial.available()) {
      char c = wifiSerial.read();
      response += c;
      if (response.indexOf(expected) >= 0) {
        return true;
      }
    }
    delay(10);
  }
  return false;
}

void wifi_init() {
  wifiSerial.begin(115200);
  delay(1000);
  send_at_command("AT", 2000);
  if (!wait_for_response("OK")) {
    Serial.println("ESP8266 not responding to AT command");
    return;
  }
  send_at_command("AT+CWMODE=1", 2000);
  if (!wait_for_response("OK")) {
    Serial.println("Failed to set station mode");
    return;
  }
  send_at_command("AT+CWJAP=\"YunKage\",\"12141250\"", 10000);
  if (!wait_for_response("OK")) {
    Serial.println("Failed to connect to WiFi");
    return;
  }
  // Запуск TCP-сервера
  send_at_command("AT+CIPMUX=1", 2000);
  send_at_command("AT+CIPSERVER=1,8080", 2000);
  Serial.println("WiFi connected via ESP8266");
}

void wifi_handle_incoming_commands() {
  if (wifiSerial.available()) {
    String cmd = wifiSerial.readStringUntil('\n');
    cmd.trim();
    if (cmd.startsWith("CMD:")) {
      char motor_cmd = cmd.charAt(4);
      Serial.print("Received motor command via WiFi: ");
      Serial.println(motor_cmd);
      handle_wheel_command(motor_cmd); // ← исправлено!
    }
    else if (cmd == "SAVE_NOW") {
      double sens[3];
      sensors_read(sens);
      float dist = read_ultrasonic_cm();
      if (gps.location.isValid()) {
        file.print(gps.location.lat(), 6);
        file.print(",");
        file.print(gps.location.lng(), 6);
        file.print(",");
        file.print(gps.speed.kmph());
        file.print(",");
        file.print(gps.altitude.meters());
        file.print(",");
        file.print(sens[0]);
        file.print(",");
        file.print(sens[1]);
        file.print(",");
        file.print(sens[2]);
        file.print(",");
        file.println(dist);
        file.flush();
        Serial.println("Manual save triggered");
      }
    }
    else if (cmd.startsWith("SET_DEST:")) {
      int sep = cmd.indexOf(',');
      if (sep > 0) {
        dest_lat = cmd.substring(9, sep).toDouble();
        dest_lon = cmd.substring(sep + 1).toDouble();
        destination_set = true;
        Serial.println("Destination set via WiFi.");
      }
    }
  }
}

void wifi_send_data(double sensors_data[3], double gps_lat, double gps_lon, float gps_speed, float gps_alt) {
  float dist = read_ultrasonic_cm();
  String data = "DATA:";
  data += String(gps_lat, 6);
  data += ",";
  data += String(gps_lon, 6);
  data += ",";
  data += String(gps_speed, 2);
  data += ",";
  data += String(gps_alt, 2);
  data += ",";
  data += String(sensors_data[0], 2);
  data += ",";
  data += String(sensors_data[1], 2);
  data += ",";
  data += String(sensors_data[2], 2);
  data += ",";
  data += String(dist, 1);
  data += "\n";
  wifiSerial.print(data);
}

bool wifi_has_destination() {
  return destination_set;
}

void wifi_request_destination(double* lat, double* lon) {
  if (destination_set) {
    *lat = dest_lat;
    *lon = dest_lon;
    destination_set = false;
  }
}
