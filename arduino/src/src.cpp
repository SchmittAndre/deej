
#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

#define LED_COUNT   1
#define HYSTERESIS  25
#define LED_PIN     8

const int NUM_SLIDERS = 1;
const int analogInputs[NUM_SLIDERS] = {A0}; //, A1, A2, A3, A4};
const int NUM_BUTTONS = 1;
const int buttonInputs[NUM_BUTTONS] = {11};//,10,9,6,5};
const int MAX_FADER_VAL = 1024;

const int motorUp = 12;
const int motorDown = 13;

int analogSliderValues[NUM_SLIDERS];
int buttonValues[NUM_BUTTONS];
int requestedMotorValue = INT16_MAX;
uint nutralcounter = 0;
int loopCounter = 0;
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  for (int i = 0; i < NUM_SLIDERS; i++) {
    pinMode(analogInputs[i], INPUT);
  }

  for (int i = 0; i < NUM_BUTTONS; i++) {
    pinMode(buttonInputs[i], INPUT_PULLUP);
  }
  pinMode(motorUp, OUTPUT);
  pinMode(motorDown, OUTPUT);
  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();            // Turn OFF all pixels ASAP
  strip.setBrightness(50);
  Serial.begin(9600);
}

void adjustMotor() {
  digitalWrite(motorDown, false);
  digitalWrite(motorUp, false);
  if (requestedMotorValue != INT16_MAX) {
    strip.setPixelColor(0, 30, 0, 0);
    strip.show();
    if (analogSliderValues[0] >= 2 && analogSliderValues[0] <= 1022){
      int diff = requestedMotorValue - analogSliderValues[0];
      if (diff > HYSTERESIS)
      {
        Serial.printf("up: %d\n", diff);
        digitalWrite(motorUp, true);
        nutralcounter = 0;
      }
      else if (diff < -HYSTERESIS)
      {
        Serial.printf("down: %d\n", diff);
        digitalWrite(motorDown, true);
        nutralcounter = 0;
      }
      else
      {
        Serial.printf("nutral: %d\n", diff);
          strip.setPixelColor(0, 0, 50, 0);
          strip.show();
        if (++nutralcounter == 20)
        {
          requestedMotorValue = INT16_MAX;
          nutralcounter = 0;
          strip.setPixelColor(0, 0, 0, 0);
          strip.show();
        }

      }
    }
  }
  
}

void receiveNewValues() {
    if (Serial.available() > 0) {
    String incomingString = Serial.readStringUntil('\n');
    requestedMotorValue = incomingString.toInt();
    // prints the received data
    Serial.print("I received: ");
    Serial.println(requestedMotorValue);
    }
}

void updateSliderValues() {
  for (int i = 0; i < NUM_SLIDERS; i++) {
    analogSliderValues[i] = analogRead(analogInputs[i]);
  }
  for (int i = 0; i < NUM_BUTTONS; i++) {
    buttonValues[i] = digitalRead(buttonInputs[i]);
  }
}

void sendSliderValues() {
  String builtString = String("");

  for (int i = 0; i < NUM_SLIDERS; i++) {
    builtString += "s";
    builtString += String((int)analogSliderValues[i]);

    if (i < NUM_SLIDERS - 1) {
      builtString += String("|");
    }
  }

  if(NUM_BUTTONS > 0){
    builtString += String("|");
  }

  for (int i = 0; i < NUM_BUTTONS; i++) {
    builtString += "b";
    builtString += String((int)buttonValues[i]);

    if (i < NUM_BUTTONS - 1) {
      builtString += String("|");
    }
  }
  if(requestedMotorValue != INT16_MAX){
    builtString += String("<<|>>");
    builtString += String((int)requestedMotorValue);
  }
  Serial.println(builtString);
}

void printSliderValues() {
  for (int i = 0; i < NUM_SLIDERS; i++) {
    String printedString = String("Slider #") + String(i + 1) + String(": ") + String(analogSliderValues[i]) + String(" mV");
    Serial.write(printedString.c_str());

    if (i < NUM_SLIDERS - 1) {
      Serial.write(" | ");
    } else {
      Serial.write("\n");
    }
  }
}

void loop() {
  adjustMotor();
  if (++loopCounter == 10)
  {
    receiveNewValues();
    updateSliderValues();
    sendSliderValues(); // Actually send data (all the time)
    //printSliderValues(); // For debug
    loopCounter = 0;
  }
  delay(1);
}
