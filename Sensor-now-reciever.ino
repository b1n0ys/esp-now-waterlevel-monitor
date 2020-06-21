#include <ESP8266WiFi.h>
#include <espnow.h>

// Structure example to receive data
// Must match the sender structure
typedef struct struct_message {
    int id;
    float distance;
} struct_message;

// Create a struct_message called myData
struct_message myData;

// Create a structure to hold the readings from each board
struct_message board1;
struct_message board2;

// Create an array with all the structures
struct_message boardsStruct[2] = {board1, board2};

// Callback function that will be executed when data is received
void OnDataRecv(uint8_t * mac_addr, uint8_t *incomingData, uint8_t len) {

  char macStr[18];
  Serial.print("Packet received from: ");
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
    mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.println(macStr);
  
  
  memcpy(&myData, incomingData, sizeof(myData));
  
  Serial.printf("Board ID %u: %u bytes\n", myData.id, len);;

  boardsStruct[myData.id - 1].distance = myData.distance;
  
  Serial.print("distance: ");
  Serial.println(myData.distance);

  if (myData.distance < 200) {
    start_buzz();  
  }
  
  check_and_quiet();
  

}

const int buzzPin = D2;
uint16_t time_elapsed = 0;
uint16_t buzz_start = 0;
boolean in_buzzing_state = false;

void start_buzz() {

  if (!in_buzzing_state) {

    in_buzzing_state = true;
    buzz_start = millis();
    Serial.println("buzing ....");
    digitalWrite(buzzPin, HIGH);
  }
  
}

void check_and_quiet() {

  time_elapsed = millis() - buzz_start;
  Serial.print("quiet ....");
  Serial.println(time_elapsed);
  if (time_elapsed > 500) {
    digitalWrite(buzzPin, LOW);
  }

  if (time_elapsed > 1000) {
    digitalWrite(buzzPin, HIGH);
  }

  if (time_elapsed > 1500) {
    digitalWrite(buzzPin, LOW);
    in_buzzing_state = false;
  }
  
}

 
void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);
  pinMode(buzzPin, OUTPUT);
  
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  // Init ESP-NOW
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  
  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info
  esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
  esp_now_register_recv_cb(OnDataRecv);
}

void loop() {
  
}
