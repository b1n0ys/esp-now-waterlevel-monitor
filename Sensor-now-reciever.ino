#include <ESP8266WiFi.h>
#include <Wire.h>  // Only needed for Arduino 1.6.5 and earlier
#include "SSD1306Wire.h" // legacy include: `#include "SSD1306.h"`
#include <espnow.h>

const int MAX_DISTANCE = 62; //Sump tank
const int MIN_LEVEL = 2;

const int MIN_DISTANCE = 15; //Overhead tank

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

int distance_to_fill = 0; // overhead tank
int distance_to_empty = 0; // sump tank

// Callback function that will be executed when data is received
void OnDataRecv(uint8_t * mac, uint8_t *incomingData, uint8_t len) {

    memcpy(&myData, incomingData, sizeof(myData));

    if (myData.id == 1) {

        distance_to_fill = ceil(myData.distance);

    } else if (myData.id == 2) {

        int level = MAX_DISTANCE - ceil(myData.distance);
        distance_to_empty = max(level, 0);
    }

}

boolean start_up = true;
uint16_t buzz_start = 0;
boolean in_buzzing_state = false;
int prev_distance_to_fill = 0;
int prev_distance_to_empty = 0;


void loop() {

    if (start_up) {

        displayWrite("Hello");
        delay(1000);
        start_up = false;

    } else {


        boolean refresh_display = false;

        if ( hasChanged(distance_to_fill, prev_distance_to_fill)) {

            Serial.println("distance_to_fill = " + String(distance_to_fill));
            refresh_display = true;

            if (distance_to_fill < MIN_DISTANCE) {
                start_buzz();
            }

            prev_distance_to_fill = distance_to_fill;
        }


        if ( hasChanged(distance_to_empty, prev_distance_to_empty)) {

            Serial.println("distance_to_empty = " + String(distance_to_empty));
            refresh_display = true;

            if (distance_to_empty < MIN_LEVEL && distance_to_empty != 0) {
                start_buzz();
            }

            prev_distance_to_empty = distance_to_empty;
        }

        if (refresh_display) {
            updateDisplay();
        }

        check_and_quiet();
        delay(500);
    }

}

boolean hasChanged( int currentValue, int prevValue) {

   //a tolerance of 1
    if ( abs(currentValue - prevValue) > 1) {
        return true;
    }
    return false;
}


void start_buzz() {

    if (!in_buzzing_state) {

        in_buzzing_state = true;
        buzz_start = millis();
        Serial.println("buzing ....");
        digitalWrite(buzzPin, HIGH);
    }

}

void check_and_quiet() {

    if (in_buzzing_state) {

        uint16_t time_elapsed = millis() - buzz_start;
        if (time_elapsed > 500 ) {

            Serial.println("turning buzzer off for 0.5 sec");
            digitalWrite(buzzPin, LOW);
        }

        //leave a delay for buzing to resume
        if (time_elapsed > 1000) {

            Serial.println("turning buzzing state off");
            in_buzzing_state = false;
        }

    }
}

void displayWrite(String str) {

    display.clear();
    display.drawString(0, 26, str);
    display.display();
}

void updateDisplay() {

    display.clear();
    //display.setFont(ArialMT_Plain_10);
    //display.drawString(0, 0, "[" + String(myData.id) + "] " + String(myData.distance) + "cm");

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