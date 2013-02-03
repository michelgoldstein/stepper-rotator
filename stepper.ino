#include <Keypad.h>
#include <AFMotor.h>

/**
 * This program depends on the motor shield from Adafruit
 * http://www.ladyada.net/make/mshield/index.html
 * and the keypad library from 
 * https://sites.google.com/site/arduinomega2560projects/home/analog/4x4-matrix-16-key-membrane
 * See the constants below for information on pins to connect everything
 */

// SONAR CONFIG

/** pin used to receive the analog sonar data */
const int SENSOR_PIN = 8;

// KEYPAD CONFIG

const byte ROWS = 4; // Four rows
const byte COLS = 4; // Three columns
// Define the Keymap - use "-" for the rest to be used as 
// moving backward
char keys[ROWS][COLS] = {
  {'1','2','3','-'},
  {'4','5','6','-'},
  {'7','8','9','-'},
  {'-','0','-','-'}
};
// Connect keypad ROW0, ROW1, ROW2 and ROW3 to these Arduino pins.
byte rowPins[ROWS] = { 22, 24, 26, 28 };
// Connect keypad COL0, COL1 and COL2 to these Arduino pins.
byte colPins[COLS] = { 30, 32, 34, 36 }; 

// Create the Keypad
Keypad kpd = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

// STEPPER CONFIG
const int STEPS_PER_REVOLUTION = 48;
/** output set 1 is M1 & M2 */
const int STEPPER_OUTPUT_SET = 1;

AF_Stepper motor(STEPS_PER_REVOLUTION, STEPPER_OUTPUT_SET);

// PROGRAM CONSTANTS

const int SPEED_IN_RPM = 50;
const int SERIAL_SPEED = 9600;
/** Lower than this distance is considered that it latched */
const int SENSOR_DISTANCE_THRESHOLD = 20;
/** Time between starting the move and waiting to latch */
const long WAIT_TIME_TO_SENSE_IN_MS = 2000;

// PROGRAM STATE

/** Should it be moving? */
boolean moving = true;
/** Time to start checking the sensor for latch */
long toWait = 0;
/** How many latches to skip */
int count;
/** Whether it's in a latch state */
boolean inLatch = false;
/** Moving forward? */
boolean forwardDirection = true;

/** Setup */
void setup() {
  Serial.begin(SERIAL_SPEED);
  motor.setSpeed(SPEED_IN_RPM);
}

/** Main loop */
void loop() {
  if(moving) {
    motor.step(1, forwardDirection ? FORWARD : BACKWARD, DOUBLE);
    // read the sensor only after the wait
    if(toWait < millis()) {
      int sensor = analogRead(SENSOR_PIN); 
      if(sensor < SENSOR_DISTANCE_THRESHOLD) {
        if(!inLatch) {
          // Serial.println("found latch");
          inLatch = true;
          count--;
          if(count < 0) {
            // stop and then wait for the next enter
            moving = false;
            // reset to moving forward
            // TODO: maybe we want to just make it keep
            // moving to the same direction from now on...
            forwardDirection = true;
          }
        } else {
          // Serial.println("Already in latch, ignoring");
        }
      } else {
        // Serial.print("Not in latch anymore ");
        // Serial.println(sensor, DEC);
        inLatch = false;
      }
    }
  } else {
    char readKey;
    if(Serial.available()) {
      readKey = Serial.read();
    } else {
      // check the keypad
      readKey = kpd.getKey();
    }
    if(readKey) {
      if(readKey == ' ') {
        // go until the next one
        count = 0;
      } else if(readKey == '-') {
        forwardDirection = !forwardDirection;
      } else {
        if(readKey >= '0' && readKey <= '9') {
          count = readKey - '0';
        } else {
          // not recognized, so make sure to continue
          count = -1;
        }
      }
      if(count >= 0) {
        moving = true;
        toWait = millis() + WAIT_TIME_TO_SENSE_IN_MS;
      }
    }
  }
}
