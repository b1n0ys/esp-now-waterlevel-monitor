#include <ESP8266WiFi.h>
#include <Wire.h>  // Only needed for Arduino 1.6.5 and earlier
#include "SSD1306Wire.h" // legacy include: `#include "SSD1306.h"`
#include <espnow.h>

const int MAX_DISTANCE = 62; //Sump tank
const int MIN_LEVEL = 2;

const int MIN_DISTANCE = 10; //Overhead tank

// Initialize the OLED display using Wire library
SSD1306Wire  display(0x3c, D2, D1);  //D2=SDK  D1=SCK  As per labeling on NodeMCU
const int buzzPin = D5;

// Structure example to receive data
// Must match the sender structure
typedef struct struct_message {
  int id;
  float distance;
} struct_message;

// Create a struct_message called myData
struct_message myData;

void setup() {

  pinMode(buzzPin, OUTPUT);
  Serial.begin(115200);
  Serial.println("");

  delay(1000);
  Serial.println("Initializing OLED Display");
  display.init();

  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_24);

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

// Callback function that will be executed when data is received
void OnDataRecv(uint8_t * mac, uint8_t *incomingData, uint8_t len) {

  memcpy(&myData, incomingData, sizeof(myData));
  //Serial.print("Bytes received: ");
  //Serial.println(len);

  if ( millis() % 1000 < 10) { //write once in a while
    Serial.print(myData.id);
    Serial.print("]: distance = ");
    Serial.println(myData.distance);
  }
}



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
  //Serial.print("quiet ....");
  //Serial.println(time_elapsed);

  if (time_elapsed > 500) {
    digitalWrite(buzzPin, LOW);
  }

  //leave a delay for buzing to resume
  if (time_elapsed > 1000) {
    in_buzzing_state = false;
  }

}

boolean start_up = true;
int distance = 0;
int level = 0;

int distance_to_empty = 0; // sump tank
int distance_to_fill = 0; // overhead tank

int prev_distance_to_empty = 0;
int prev_distance_to_fill = 0;

void loop() {

  if (start_up) {

    displayWrite("Hello");
    delay(1000);
    start_up = false;

  }else {

    if (myData.id == 1) {

        Serial.print("inside borad 1 reading");

        distance_to_fill = ceil(myData.distance);
        if (distance_to_fill < MIN_DISTANCE) {
          start_buzz();
        }

        Serial.print(", distance_to_fill = " + String(distance_to_fill));
        Serial.print(", prev_distance_to_fill = " + String(prev_distance_to_fill));

        if ( distance_to_fill != prev_distance_to_fill) {
          updateDisplay();
        }
        prev_distance_to_fill = distance_to_fill;
        Serial.println("..done");

    }else if (myData.id == 2) {

      Serial.print("inside borad 2 reading");

      distance_to_empty = MAX_DISTANCE - floor(myData.distance);
      if (distance_to_empty < 0) distance_to_empty = 0;

      if (distance_to_empty < MIN_LEVEL && distance_to_empty != 0) {
        start_buzz();
      }

      Serial.print(", distance_to_empty = " + String(distance_to_empty));
      Serial.print(", prev_distance_to_empty = " + String(prev_distance_to_empty));

      if ( distance_to_empty != prev_distance_to_empty) {
        updateDisplay();
      }
      prev_distance_to_empty = distance_to_empty;

    }

    check_and_quiet();
    delay(500);
  }

}

void displayWrite(String str) {

    display.clear();
    display.drawString(0, 26, str);
    display.display();
}

void updateDisplay() {

    display.clear();
    display.setFont(ArialMT_Plain_10);
    display.drawString(0, 0, "[" +String(myData.id) + "] " + String(myData.distance) + "cm");

    display.setFont(ArialMT_Plain_16);
    display.drawString(0, 20, "to fill:" );
    display.setFont(ArialMT_Plain_24);
    display.drawString(80, 15, String(distance_to_fill));

    display.setFont(ArialMT_Plain_16);
    display.drawString(0, 45, "to empty:" );
    display.setFont(ArialMT_Plain_24);
    display.drawString(80, 40, String(distance_to_empty));

    display.display();
}