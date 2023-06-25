#include <Arduino.h>

#define BUTTON 8
#define RELAY 9
#define PRESSURE_SENSOR A0
#define LED 13
#define DEBUG true
#define DEBOUNCE_TIME 500
#define HYSTERESIS 10

// Note : Lowest pressure = -0.85bar , sensor = 146
//        Highest prssure = 0 bar, sensor = 897
#define MIN_PRESSURE 146
#define MAX_PRESSURE 897


void debugPrint(String s){
  if (DEBUG)
    Serial.println(s);
}

class Pressure_Regulator {
  public:
      enum Status {OFF, ON};
      uint8_t sensorPin;
      uint8_t relayPin;
      uint8_t ledPin;
      uint8_t buttonPin;
      Status regulatorStatus;
      Status relayStatus;
      int measure;
      int trigger;
      long lastCheckTime;

      Pressure_Regulator(uint8_t sensorPin, uint8_t relayPin, uint8_t buttonPin, uint8_t ledPin): 
          sensorPin(sensorPin),
          relayPin(relayPin), 
          buttonPin(buttonPin),
          ledPin(ledPin) {};

      void begin() {
          measure = 0;
          trigger = 0;
          lastCheckTime = 0;
          regulatorStatus = OFF; 
          relayStatus = ON;
          updateRelay();
          updateLed();
      }

      void handle(){
          measure = analogRead(sensorPin);
          checkButton();
          regulate();
          updateRelay();
          updateLed();
          show();
      }

      void checkButton() {
          long checktime = millis();
          if ((checktime-lastCheckTime) > DEBOUNCE_TIME) {
            if (digitalRead(buttonPin)==LOW) {
              if (regulatorStatus == OFF) {
                  regulatorStatus = ON;
                  trigger = measure;
              } else {
                  regulatorStatus = OFF;
                  trigger = 0;
              }
              debugPrint(String("Button was pressed.       regulator=")+regulatorStatus+"trigger="+trigger);
            }
            lastCheckTime = checktime;
        }
      }

      void regulate() {
          if (regulatorStatus == ON) {
            if (relayStatus == OFF and measure >= trigger + HYSTERESIS)   //  max(MAX_PRESSURE, trigger+HYSTERESIS))
                relayStatus = ON;
            if (relayStatus == ON and measure <= trigger)  // min(MIN_PRESSURE, trigger-HYSTERESIS))
                relayStatus = OFF;
          } else {
              relayStatus = ON;
          }
      }

      void show() {
          debugPrint(String("sensor=")+measure+" regulator="+regulatorStatus+" trigger="+trigger+" relay="+relayStatus);
      }

      void updateRelay(){
          if (relayStatus == ON){
            digitalWrite(relayPin, HIGH);
          } else {
            digitalWrite(relayPin, LOW);
          }
      }
      
      void updateLed(){
        if (regulatorStatus == ON){
          digitalWrite(ledPin, HIGH);
        } else {
          digitalWrite(ledPin, LOW);
        }
      }

};

Pressure_Regulator pressure_regulator(PRESSURE_SENSOR, RELAY, BUTTON, LED);

void setup() {
  // put your setup code here, to run once:
   Serial.begin(115200);
   Serial.println("Starting up...");
   pinMode(RELAY, OUTPUT);
   pinMode(LED, OUTPUT);
   pinMode(BUTTON, INPUT_PULLUP);
   pressure_regulator.begin();
}

void loop() {
  pressure_regulator.handle();
  delay(100);
}
