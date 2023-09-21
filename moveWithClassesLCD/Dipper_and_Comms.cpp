#include "Dipper_and_Comms.h"
#include "Arduino.h"
#include "LiquidCrystal_I2C.h"

Dipper::Dipper(byte xdp, byte xpp, byte up, byte dp, byte lp) {
  xdirpin = xdp;
  xpulpin = xpp;
  uppin = up;
  downpin = dp;
  limitpin = lp;

  //Sets pins as outputs
  pinMode(xdirpin, OUTPUT);
  pinMode(xpulpin, OUTPUT);

  pinMode(uppin, OUTPUT);
  pinMode(downpin, OUTPUT);

  pinMode(limitpin, INPUT_PULLUP);

  position = 0;
}

Dipper::Dipper(byte xdp, byte xpp, byte up, byte dp, byte lp, LiquidCrystal_I2C lcd) {
  xdirpin = xdp;
  xpulpin = xpp;
  uppin = up;
  downpin = dp;
  limitpin = lp;

  LCD = lcd;
  LCD.init();
  LCD.backlight();
  LCD.print("hello");
  
  //Sets pins as outputs
  pinMode(xdirpin, OUTPUT);
  pinMode(xpulpin, OUTPUT);

  pinMode(uppin, OUTPUT);
  pinMode(downpin, OUTPUT);

  pinMode(limitpin, INPUT_PULLUP);

  position = 0;
  LCDon = true;
}

void Dipper::doStep() {
  digitalWrite(31, HIGH);
  delayMicroseconds(pulse_speed);
  digitalWrite(31, LOW);
  delayMicroseconds(pulse_speed);
}
void Dipper::xMoveSteps(int steps) {

  //Sets the direction of movement
  int dir;
  int speeddelay = 1000;

  if (steps < 0) {
    digitalWrite(xdirpin, HIGH);
    dir = -1;
  } else {
    digitalWrite(xdirpin, LOW);
    dir = +1;
  }

  for (int i = 0; i < abs(steps); i++) {
    int limitVal = digitalRead(limitpin);

    if (limitVal == 0 && steps < 0) {
      break;
    }
    if (position > posmax) {
      break;
    }

    doStep();

    position += dir;
  }
}
void Dipper::xMoveTo(int finalPlace) {
  xMoveSteps(finalPlace - position);
}
void Dipper::yMoveTime(int time, bool up) {
  if (up == true) {
    digitalWrite(uppin, HIGH);
    digitalWrite(downpin, LOW);
    delay(time);
  } else {
    digitalWrite(uppin, LOW);
    digitalWrite(downpin, HIGH);
    delay(time);
  }

  digitalWrite(uppin, LOW);
  digitalWrite(downpin, LOW);
}
void Dipper::diamond(bool dir) {
  int rinse_steps = 200;
  int rinse_delay = 50;

  digitalWrite(uppin, LOW);
  digitalWrite(downpin, HIGH);
  digitalWrite(xdirpin, dir);
  for (int i = 0; i < rinse_steps; i++) {
    doStep();
  }

  delay(rinse_delay);

  digitalWrite(xdirpin, !dir);
  for (int i = 0; i < rinse_steps; i++) {
    doStep();
  }

  delay(rinse_delay);

  digitalWrite(uppin, HIGH);
  digitalWrite(downpin, LOW);
  digitalWrite(xdirpin, !dir);

  for (int i = 0; i < rinse_steps; i++) {
    doStep();
  }

  delay(rinse_delay);

  digitalWrite(xdirpin, dir);

  for (int i = 0; i < rinse_steps; i++) {
    doStep();
  }

  digitalWrite(uppin, LOW);
  digitalWrite(downpin, LOW);
}
void Dipper::dip(int diptime) {
  yMoveTime(opheight, false);

  unsigned long current_time = millis();
  int period = diptime * 1000;

  for (int i = 0; i < diptime; i++) {
    delay(1000);
  }

  yMoveTime(opheight + 2500, true);
}
void Dipper::rinse(int rinse_cycles) {
  yMoveTime(opheight - 200, false);
  for (int i = 0; i < rinse_cycles; i++) {
    if (i % 2 == 0) {
      yMoveTime(250, true);
    } else {
      yMoveTime(250, false);
    }
  }
  yMoveTime(opheight + 2700, true);
}
void Dipper::doDipSequence(int layers, int position_array[], int time_array[], int wash_array[], int stops) {
  for (int i = 0; i < layers; i++) {
    for (int j = 0; j < stops; j++) {
      Serial.print("Layer: ");
      Serial.print(i + 1);
      Serial.print(" Stop: ");
      Serial.println(j + 1);

      LCD.clear();

      LCD.setCursor(0, 0);
      LCD.print("Layer: ");
      LCD.print(i + 1);

      LCD.setCursor(0, 1);
      LCD.print("Stop: ");
      LCD.print(j + 1);

      xMoveTo(position_array[j]);
      if (wash_array[j] == 0) {
        dip(time_array[j]);
      } else {
        Serial.println(wash_array[j]);
        rinse(wash_array[j]);
      }
    }
  }
  Serial.println("Country roads, take me home!");
  xMoveTo(position_array[0]);
}
void Dipper::calibrateX() {
  int limitval = digitalRead(limitpin);
  digitalWrite(xdirpin, HIGH);

  while (limitval == 1) {
    limitval = digitalRead(limitpin);

    doStep();
  }

  position = 0;
}
void Dipper::calibrateY() {
  yMoveTime(19000, true);
}
void Dipper::joystick(int xval, int yval) {
  int dir;
  static int prev_position = position;

  if (abs(xval) > tolerance) {
    if (xval < 0) {
      digitalWrite(xdirpin, LOW);
      dir = 1;
    } else {
      digitalWrite(xdirpin, HIGH);
      dir = -1;
    }

    int limitval = digitalRead(limitpin);

    if (limitval == 1 || (limitval == 0 && xval < 0)) {
      doStep();
      position += dir;
    }
  } else {
    if (prev_position != position) {
      Serial.print("Position: ");
      Serial.print(0.025 * position);
      Serial.println(" mm");

      if (LCDon == true) {
        LCD.clear();
        LCD.setCursor(0, 0);
        LCD.print("Position: ");
        LCD.print(0.025 * position);
        LCD.print(" mm");
      }
      prev_position = position;
    }
  }

  if (yval < -1 * tolerance) {
    //moves down
    digitalWrite(uppin, HIGH);
    digitalWrite(downpin, LOW);
  } else if (yval > tolerance) {
    //moves up
    digitalWrite(uppin, LOW);
    digitalWrite(downpin, HIGH);
  } else {
    digitalWrite(uppin, LOW);
    digitalWrite(downpin, LOW);
  }
}

//Comms library

void Comms::readSerialData() {
  static bool receiving = false;
  static int index = 0;
  char startmarker = '<';
  char endmarker = '>';
  char rc;

  while (Serial.available() > 0 && newData == false) {
    rc = Serial.read();

    if (rc == startmarker) {
      receiving = true;
    } else if (receiving == true) {
      if (rc != endmarker) {
        receivedChars[index] = rc;
        index++;
        if (index >= numChars) {
          index = numChars - 1;
        }
      } else {
        receivedChars[index] = '\0';
        receiving = false;
        newData = true;
        index = 0;
      }
    }
  }
}
void Comms::parseString(char inputStr[]) {
  Serial.print("Parsing string: ");
  Serial.println(inputStr);

  char delimiters[] = "(), ";
  char* pch;
  pch = strtok(inputStr, delimiters);

  switch (*pch) {
    case 'P':
      pch = strtok(NULL, delimiters);
      positions[stops] = atoi(pch);

      pch = strtok(NULL, delimiters);
      times[stops] = atoi(pch);

      pch = strtok(NULL, delimiters);
      washCycles[stops] = atoi(pch);

      Serial.print("Added point (Position: ");
      Serial.print(positions[stops]);
      Serial.print(" Time: ");
      Serial.print(times[stops]);
      Serial.print(" Wash Cycles: ");
      Serial.print(washCycles[stops]);
      Serial.println(")");

      newData = false;

      stops++;
      break;

    case 'L':
      pch = strtok(NULL, delimiters);
      layers = atoi(pch);

      Serial.print("Layers: ");
      Serial.println(layers);

      newData = false;
      break;

    case 'R':
      Serial.println("Reset the arrays");
      arrayReset();
      stops = 0;
      newData = false;
      break;

    case 'S':
      presentData(stops);
      beginJob = true;
      break;

    case 'E':
      Serial.end();
      break;

    default:
      Serial.println("Input not recognized");
      newData = false;
  }
}
void Comms::presentData(int index) {
  //Print the numbers of layers
  Serial.print("Layers: ");
  Serial.println(layers);

  //Positions of the vials
  Serial.print("Positions: ");
  for (int i = 0; i < index; i++) {
    Serial.print(positions[i]);
    Serial.print(", ");
  }
  Serial.print("\n");

  //Time spend in each vial
  Serial.print("Times: ");
  for (int i = 0; i < index; i++) {
    Serial.print(times[i]);
    Serial.print(", ");
  }
  Serial.print("\n");

  //Wash cycles in each vial
  Serial.print("Wash cycles: ");
  for (int i = 0; i < index; i++) {
    Serial.print(washCycles[i]);
    Serial.print(", ");
  }
  Serial.println("\n");

  newData = false;
}
void Comms::arrayReset() {
  layers = 0;
  for (int i = 0; i < 20; i++) {
    positions[i] = 0;
    times[i] = 0;
    washCycles[0] = 0;
  }
}