#include "painlessMesh.h"
#include <ESP32Servo.h>  // Library Servo khusus ESP32

#define MESH_PREFIX     "Pswat Store"
#define MESH_PASSWORD   "reborn123"
#define MESH_PORT       5555

#define RELAY_POMPA_PIN  25  // Pin untuk relay pompa air
#define RELAY_HEATER_PIN 26  // Pin untuk relay heater
#define SERVO_PIN        27  // Pin untuk servo

Scheduler userScheduler;
painlessMesh mesh;
Servo servo;  // Object untuk ESP32Servo

bool atapTertutup = false; // Status atap

// Fungsi untuk mengontrol perangkat berdasarkan data sensor
void kontrolPerangkat(float temperature, float humidity, int ldrValue) {
  // Kontrol Pompa Air
  if (humidity < 20) {
    digitalWrite(RELAY_POMPA_PIN, HIGH); // Nyalakan pompa
    Serial.println("Pompa Air: ON (Kelembapan rendah)");
  } else {
    digitalWrite(RELAY_POMPA_PIN, LOW); // Matikan pompa
    Serial.println("Pompa Air: OFF");
  }

  // Kontrol Heater
  if (temperature < 10) {
    digitalWrite(RELAY_HEATER_PIN, HIGH); // Nyalakan heater
    Serial.println("Heater: ON (Suhu rendah)");
  } else {
    digitalWrite(RELAY_HEATER_PIN, LOW); // Matikan heater
    Serial.println("Heater: OFF");
  }

  // Kontrol Atap dengan Servo
  if (ldrValue < 20 && !atapTertutup) {
    servo.write(180); // Putar servo 180 derajat
    atapTertutup = true;
    Serial.println("Atap: Ditutup (LDR rendah - Gelap)");
  } else if (ldrValue >= 20 && atapTertutup) {
    servo.write(0); // Putar servo kembali ke posisi awal
    atapTertutup = false;
    Serial.println("Atap: Dibuka (LDR tinggi - Terang)");
  }
}

// Callback saat pesan diterima
void receivedCallback(uint32_t from, String &msg) {
  Serial.printf("Pesan diterima dari %u: %s\n", from, msg.c_str());

  // Parsing data dari pesan
  float temperature = 0;
  float humidity = 0;
  int ldrValue = 0;

  // Format pesan: "Node: x | Temp: xx.xx°C | Hum: xx.xx% | LDR: xxx"
  sscanf(msg.c_str(), "Node: %*d | Temp: %f°C | Hum: %f%% | LDR: %d", &temperature, &humidity, &ldrValue);

  // Cetak nilai sensor
  Serial.printf("Suhu: %.2f°C, Kelembapan: %.2f%%, LDR: %d\n", temperature, humidity, ldrValue);

  // Kontrol perangkat berdasarkan data yang diterima
  kontrolPerangkat(temperature, humidity, ldrValue);
}

void newConnectionCallback(uint32_t nodeId) {
  Serial.printf("--> Node baru terhubung, ID = %u\n", nodeId);
}

void changedConnectionCallback() {
  Serial.println("Koneksi berubah");
}

void setup() {
  Serial.begin(115200);

  // Inisialisasi pin relay dan servo
  pinMode(RELAY_POMPA_PIN, OUTPUT);
  pinMode(RELAY_HEATER_PIN, OUTPUT);

  servo.attach(SERVO_PIN); // Pasangkan pin servo
  servo.write(0);          // Atur posisi awal servo ke 0 derajat

  // Inisialisasi mesh
  mesh.setDebugMsgTypes(ERROR | STARTUP);
  mesh.init(MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT);
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);

  Serial.println("Node penerima siap!");
}

void loop() {
  mesh.update();
}
