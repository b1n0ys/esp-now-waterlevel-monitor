//ESP8266 Node MCU - V3
#include <ESP8266WiFi.h>
#include <espnow.h>

#define BOARD_ID 1

const int trigPin = D2 ; 
const int echoPin = D0;

float duration, distance;

// REPLACE WITH RECEIVER MAC Address
uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00};

// Structure example to send data
// Must match the receiver structure
typedef struct struct_message {
  int id;
  float distance;
} struct_message;

// Create a struct_message called myData
struct_message myData;

// Callback when data is sent
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  Serial.print("Last Packet Send Status: ");
  if (sendStatus == 0){
    Serial.println("Delivery success");
  }
  else{
    Serial.println("Delivery fail");
  }
}
 
void setup() {
  // Init Serial Monitor
  Serial.begin(115200);
  Serial.println("");

  Serial.println("Initializing ESP-NOW ..");
 
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  // Init ESP-NOW
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  Serial.println("... done");

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
  esp_now_register_send_cb(OnDataSent);
  
  // Register peer
  Serial.println("Register peer ..");
  esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);
  Serial.println("... done");
 
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  Serial.println("Setup completed !");
}
 
void loop() {

  digitalWrite(trigPin, LOW); 
  delayMicroseconds(2); 
  digitalWrite(trigPin, HIGH); 
  delayMicroseconds(10); 
  digitalWrite(trigPin, LOW);
  
  duration = pulseIn(echoPin, HIGH); 

  myData.id = BOARD_ID;
  myData.distance = (duration*.0343)/2; 
  Serial.println(myData.distance);
  
  // Send message via ESP-NOW
  esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));

  delay(300);
}
