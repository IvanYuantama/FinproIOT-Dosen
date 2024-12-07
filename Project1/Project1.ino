#include "painlessMesh.h"
#include "DHT.h"  // Library untuk sensor DHT11

#define MESH_PREFIX     "Pswat Store"
#define MESH_PASSWORD   "reborn123"
#define MESH_PORT       5555

#define DHTPIN          4          // Pin GPIO untuk DHT11
#define DHTTYPE         DHT11      // Jenis sensor DHT
#define LDRPIN          34         // Pin ADC untuk LDR

Scheduler userScheduler; // untuk mengontrol task user
painlessMesh mesh;

// Inisialisasi DHT
DHT dht(DHTPIN, DHTTYPE);

// Prototype fungsi
void sendMessage();

Task taskSendMessage(TASK_SECOND * 2, TASK_FOREVER, &sendMessage);

// Fungsi untuk mengirim data sensor
void sendMessage() {
  // // Membaca suhu dan kelembapan dari DHT11
  // float temperature = dht.readTemperature();  
  // float humidity = dht.readHumidity();        
  // int ldrValue = analogRead(LDRPIN);    

  // For testing
  float temperature = 20.00;  
  float humidity = 31.00;        
  int ldrValue = 40;       

  // Memeriksa apakah pembacaan sensor valid
  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // Format pesan
  String msg = "Node: " + String(mesh.getNodeId());
  msg += " | Temp: " + String(temperature) + "°C";
  msg += " | Hum: " + String(humidity) + "%";
  msg += " | LDR: " + String(ldrValue);

  // Kirim pesan ke semua node
  mesh.sendBroadcast(msg);
  Serial.println("Sent: " + msg);

  // Atur interval pengiriman secara random
  taskSendMessage.setInterval(random(TASK_SECOND * 2, TASK_SECOND * 5));
}

// Callback ketika pesan diterima
void receivedCallback(uint32_t from, String &msg) {
  Serial.printf("Received from %u: %s\n", from, msg.c_str());
}

void newConnectionCallback(uint32_t nodeId) {
  Serial.printf("--> New Connection, nodeId = %u\n", nodeId);
}

void changedConnectionCallback() {
  Serial.println("Changed connections");
}

void nodeTimeAdjustedCallback(int32_t offset) {
  Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(), offset);
}

void setup() {
  Serial.begin(115200);

  // Inisialisasi DHT
  dht.begin();

  // Inisialisasi mesh
  mesh.setDebugMsgTypes(ERROR | STARTUP); // Debug message
  mesh.init(MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT);
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);

  // Tambahkan task pengiriman data
  userScheduler.addTask(taskSendMessage);
  taskSendMessage.enable();
}

void loop() {
  // Update mesh dan scheduler
  mesh.update();
}
