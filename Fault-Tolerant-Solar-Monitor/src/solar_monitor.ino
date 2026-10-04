/*
  Fault-Tolerant Solar Panel Monitoring System
  ------------------------------------------------
  Current prototype:
  - ESP32 reads solar-panel voltage through a voltage divider.
  - ESP32 reads an LDR analog value.
  - Voltage and light level are compared.
  - A large mismatch is reported as a possible sensor fault.
  - During a fault, the LDR is used as a backup reference.
  - A simple Wi-Fi dashboard is provided by the ESP32.

  IMPORTANT:
  1. The GPIO pins below are examples. Change them if your wiring is different.
  2. Do NOT connect the solar-panel output directly to an ESP32 ADC pin.
  3. This example assumes a 10kΩ / 10kΩ voltage divider.
  4. Calibrate the voltage reading with a multimeter before final testing.
  5. Linear Regression / AI prediction using historical data is FUTURE WORK.
*/

#include <WiFi.h>
#include <WebServer.h>

// ---------- Wi-Fi Access Point ----------
const char* AP_SSID = "SolarMonitor";
const char* AP_PASSWORD = "12345678";

WebServer server(80);

// ---------- ESP32 ADC pins ----------
// GPIO 34 and GPIO 35 are input-only ADC pins on common ESP32 boards.
const int VOLTAGE_PIN = 34;
const int LDR_PIN     = 35;

// ---------- Voltage divider ----------
// Example: 10kΩ from solar + to ADC and 10kΩ from ADC to GND.
// Divider ratio = (10k + 10k) / 10k = 2.0
const float R1 = 10000.0;
const float R2 = 10000.0;

// ---------- ADC settings ----------
const float ADC_REFERENCE = 3.3;
const int ADC_MAX = 4095;

// ---------- Calibration ----------
// Change this after comparing the ESP32 reading with your multimeter.
float VOLTAGE_CALIBRATION = 1.00;

// ---------- LDR / fault detection ----------
// These are starting values only. Calibrate using your actual panel and LDR.
const int LDR_DARK = 100;
const int LDR_BRIGHT = 3000;

// Expected panel voltage range for this prototype.
// Change these values after measuring your panel.
const float EXPECTED_VOLTAGE_MIN = 1.0;
const float EXPECTED_VOLTAGE_MAX = 6.0;

// Allowed difference between measured voltage and LDR-based expected voltage.
const float FAULT_THRESHOLD = 1.5;

// ---------- Current readings ----------
float measuredVoltage = 0.0;
float expectedVoltage = 0.0;
int ldrValue = 0;
bool sensorFault = false;
float backupVoltage = 0.0;

// ---------- History ----------
const int HISTORY_SIZE = 20;
float voltageHistory[HISTORY_SIZE];
int historyCount = 0;
int historyIndex = 0;

unsigned long lastRead = 0;
const unsigned long READ_INTERVAL = 2000;


// Convert LDR value into an approximate expected voltage.
// This is NOT ML. It is only the current prototype's backup/reference logic.
float estimateVoltageFromLDR(int lightValue) {
  lightValue = constrain(lightValue, LDR_DARK, LDR_BRIGHT);

  float ratio = (float)(lightValue - LDR_DARK) /
                (float)(LDR_BRIGHT - LDR_DARK);

  float voltage = EXPECTED_VOLTAGE_MIN +
                  ratio * (EXPECTED_VOLTAGE_MAX - EXPECTED_VOLTAGE_MIN);

  return constrain(voltage, EXPECTED_VOLTAGE_MIN, EXPECTED_VOLTAGE_MAX);
}


// Read solar-panel voltage through the voltage divider.
float readSolarVoltage() {
  int raw = analogRead(VOLTAGE_PIN);

  float adcVoltage = ((float)raw / ADC_MAX) * ADC_REFERENCE;

  float panelVoltage = adcVoltage * ((R1 + R2) / R2);

  panelVoltage *= VOLTAGE_CALIBRATION;

  return panelVoltage;
}


// Store voltage history for the dashboard.
// This is only temporary ESP32 RAM storage.
// It is NOT the future AI/ML dataset yet.
void storeHistory(float voltage) {
  voltageHistory[historyIndex] = voltage;
  historyIndex = (historyIndex + 1) % HISTORY_SIZE;

  if (historyCount < HISTORY_SIZE) {
    historyCount++;
  }
}


// Check whether the measured voltage is consistent with light intensity.
void checkFault() {
  expectedVoltage = estimateVoltageFromLDR(ldrValue);

  float difference = abs(measuredVoltage - expectedVoltage);

  sensorFault = difference > FAULT_THRESHOLD;

  if (sensorFault) {
    // Current prototype backup reference.
    backupVoltage = expectedVoltage;
  } else {
    backupVoltage = measuredVoltage;
  }
}


// Read sensors and update system state.
void updateSensors() {
  measuredVoltage = readSolarVoltage();
  ldrValue = analogRead(LDR_PIN);

  checkFault();
  storeHistory(measuredVoltage);
}


// ---------- Web Dashboard ----------
String makeDashboard() {

  String statusText = sensorFault ? "POSSIBLE SENSOR FAULT" : "NORMAL";
  String displayedVoltage = sensorFault
                            ? String(backupVoltage, 2)
                            : String(measuredVoltage, 2);

  String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1">
<meta http-equiv="refresh" content="3">
<title>Solar Monitor</title>
<style>
body {
  margin: 0;
  background: #0d0d12;
  color: white;
  font-family: Arial, sans-serif;
  text-align: center;
}
.container {
  max-width: 700px;
  margin: auto;
  padding: 25px;
}
h1 {
  color: #f5b52b;
  letter-spacing: 4px;
}
.subtitle {
  color: #777;
  letter-spacing: 3px;
}
.card {
  background: #15151c;
  border: 1px solid #444;
  border-radius: 14px;
  padding: 25px;
  margin: 15px 0;
}
.value {
  font-size: 42px;
  font-weight: bold;
}
.label {
  color: #999;
  letter-spacing: 3px;
}
.status {
  font-size: 24px;
  margin-top: 10px;
}
.good {
  color: #65d68a;
}
.bad {
  color: #ff6b6b;
}
.small {
  color: #aaa;
  font-size: 14px;
}
</style>
</head>

<body>
<div class="container">

<h1>☀ SOLAR MONITOR</h1>
<div class="subtitle">REAL-TIME ENERGY MONITORING</div>

<div class="card">
  <div class="label">SYSTEM STATUS</div>
  <div class="status %STATUS_CLASS%">%STATUS%</div>
</div>

<div class="card">
  <div class="label">LDR VALUE</div>
  <div class="value">%LDR%</div>
</div>

<div class="card">
  <div class="label">MEASURED VOLTAGE</div>
  <div class="value">%MEASURED% V</div>
</div>

<div class="card">
  <div class="label">EXPECTED / BACKUP VOLTAGE</div>
  <div class="value">%BACKUP% V</div>
  <div class="small">
    In fault mode this is an LDR-based reference, not an AI prediction.
  </div>
</div>

<div class="card">
  <div class="label">DISPLAYED MONITORING VALUE</div>
  <div class="value">%DISPLAYED% V</div>
</div>

</div>
</body>
</html>
)rawliteral";

  html.replace("%STATUS%", statusText);
  html.replace("%STATUS_CLASS%", sensorFault ? "bad" : "good");
  html.replace("%LDR%", String(ldrValue));
  html.replace("%MEASURED%", String(measuredVoltage, 2));
  html.replace("%BACKUP%", String(backupVoltage, 2));
  html.replace("%DISPLAYED%", displayedVoltage);

  return html;
}


void handleRoot() {
  server.send(200, "text/html", makeDashboard());
}


void setup() {
  Serial.begin(115200);
  delay(1000);

  // ADC configuration
  analogReadResolution(12);

  pinMode(VOLTAGE_PIN, INPUT);
  pinMode(LDR_PIN, INPUT);

  // Start ESP32 as a Wi-Fi Access Point.
  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASSWORD);

  Serial.println();
  Serial.println("==================================");
  Serial.println(" Fault-Tolerant Solar Monitor");
  Serial.println("==================================");
  Serial.print("Wi-Fi name: ");
  Serial.println(AP_SSID);
  Serial.print("Dashboard IP: ");
  Serial.println(WiFi.softAPIP());

  server.on("/", handleRoot);
  server.begin();

  Serial.println("Dashboard started.");
}


void loop() {

  server.handleClient();

  if (millis() - lastRead >= READ_INTERVAL) {

    lastRead = millis();

    updateSensors();

    Serial.println();
    Serial.println("----- Solar Monitor -----");

    Serial.print("LDR Value       : ");
    Serial.println(ldrValue);

    Serial.print("Measured Voltage: ");
    Serial.print(measuredVoltage, 2);
    Serial.println(" V");

    Serial.print("Expected Voltage: ");
    Serial.print(expectedVoltage, 2);
    Serial.println(" V");

    if (sensorFault) {
      Serial.println("Status          : POSSIBLE SENSOR FAULT");
      Serial.print("Backup Reference: ");
      Serial.print(backupVoltage, 2);
      Serial.println(" V");
    } else {
      Serial.println("Status          : NORMAL");
    }
  }
}
