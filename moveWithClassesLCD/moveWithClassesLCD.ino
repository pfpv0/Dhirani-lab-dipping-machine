#include <LiquidCrystal_I2C.h>
#include "Dipper_and_Comms.h"

// Sets pins for the stepper motor. xdirpin controls directions and xpulpin sends a pulse corresponding to a step
#define dirpin 30
#define pulpin 31

//Sets pins for the dipping (y movement). HIGH LOW is up(?) and LOW HIGH is down
#define upPin 40
#define downPin 41

//Limit switch button
#define limitPin 48

#define xJoyPin A2
#define yJoyPin A3
#define joyButton 38

Comms comms;
LiquidCrystal_I2C LCD(0x27, A4, A5);
Dipper dipper(dirpin, pulpin, upPin, downPin, limitPin, LCD);

int xJoy0, yJoy0;

void setup() {
  setUpPins();
  Serial.begin(9600);
  delay(100);
  Serial.println("Serial connected");

  xJoy0 = analogRead(xJoyPin);
  yJoy0 = analogRead(yJoyPin);

  LCD.init();
  LCD.backlight();
  LCD.print("hello");

  dipper.calibrateY();
  dipper.calibrateX();
  dipper.xMoveSteps(1200); //Move left to not put strain on the switch
}

void loop() {
  comms.stops = 0;
  LCD.clear();
  while (comms.beginJob == false) {
    int xJoyDiff = analogRead(xJoyPin) - xJoy0;
    int yJoyDiff = analogRead(yJoyPin) - yJoy0;

    dipper.joystick(xJoyDiff, yJoyDiff);

    comms.readSerialData();
    if (comms.newData == true) {
      comms.parseString(comms.receivedChars);
    }
  }
  dipper.calibrateY();

  dipper.doDipSequence(comms.layers, comms.positions, comms.times, comms.washCycles, comms.stops);
  comms.beginJob = false;
}

void setUpPins() {
  //Sets pins as outputs
  pinMode(dirpin, OUTPUT);
  pinMode(pulpin, OUTPUT);

  pinMode(upPin, OUTPUT);
  pinMode(downPin, OUTPUT);

  //pinMode(killerPin, INPUT);
  pinMode(xJoyPin, INPUT);
  pinMode(yJoyPin, INPUT);
  pinMode(joyButton, INPUT_PULLUP);


  pinMode(limitPin, INPUT_PULLUP);
}