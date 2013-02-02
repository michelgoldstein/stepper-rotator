#include <Keypad.h>
#include <AFMotor.h>

#define SENSOR_PIN 8

const byte ROWS = 4; // Four rows
const byte COLS = 4; // Three columns
// Define the Keymap
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
// Connect keypad ROW0, ROW1, ROW2 and ROW3 to these Arduino pins.
byte rowPins[ROWS] = { 22, 24, 26, 28 };
// Connect keypad COL0, COL1 and COL2 to these Arduino pins.
byte colPins[COLS] = { 30, 32, 34, 36 }; 

// Create the Keypad
Keypad kpd = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

AF_Stepper motor(48, 1);

boolean moving = true;
long toWait = 0;
int count;
boolean inLatch = false;

void setup() {
  Serial.begin(9600);           // set up Serial library at 9600 bps
  Serial.println("Stepper test!");

  motor.setSpeed(50);  // 50 rpm   
}

/** 
 * Figures out if there is a number pressed on the keypad set in digital
 * pins 22-36 (even only) relative to pins 1-8 on the keypad. Will return -1
 * if none is pressed.
 */
int numberPressed() {
  
}

void loop() {
  if(moving) {
    motor.step(1, FORWARD, SINGLE);
    // read the sensor only after the wait
    if(toWait < millis()) {
      int sensor = analogRead(SENSOR_PIN); 
      if(sensor < 20) {
        if(!inLatch) {
          Serial.println("found latch");
          inLatch = true;
          count--;
          if(count < 0) {
            // stop and then wait for the next enter
            moving = false;
          }
        } else {
          Serial.println("Already in latch, ignoring");
        }
      } else {
        Serial.print("Not in latch anymore ");
        Serial.println(sensor, DEC);
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
        toWait = millis() + 2000;
      }
    }
  }
}
