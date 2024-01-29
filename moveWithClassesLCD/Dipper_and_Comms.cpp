#include "Dipper_and_Comms.h"

DipperLCD::DipperLCD(byte xdp, byte xpp, byte up, byte dp, byte lp) {
  _xdirpin = xdp;
  _xpulpin = xpp;
  _uppin = up;
  _downpin = dp;
  _limitpin = lp;

  //Sets pins as outputs
  pinMode(_xdirpin, OUTPUT);
  pinMode(_xpulpin, OUTPUT);

  pinMode(_uppin, OUTPUT);
  pinMode(_downpin, OUTPUT);

  pinMode(_limitpin, INPUT_PULLUP);

  position = 0;
}

void DipperLCD::doLCDsetup(){
  LCD.init();
  LCD.clear();
  LCD.backlight();
  LCD.print("hello");
}

void DipperLCD::doStep() {
  digitalWrite(_xpulpin, HIGH);
  delayMicroseconds(pulse_speed);
  digitalWrite(_xpulpin, LOW);
  delayMicroseconds(pulse_speed);
}
void DipperLCD::xMoveSteps(int steps) {

  //Sets the direction of movement
  int dir;
  int speeddelay = 1000;

  if (steps < 0) {
    digitalWrite(_xdirpin, HIGH);
    dir = -1;
  } else {
    digitalWrite(_xdirpin, LOW);
    dir = +1;
  }

  for (int i = 0; i < abs(steps); i++) {
    int limitVal = digitalRead(_limitpin);

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
void DipperLCD::xMoveTo(int finalPlace) {
  xMoveSteps(finalPlace - position);
}
void DipperLCD::yMoveTime(int time, bool up) {
  if (up == true) {
    digitalWrite(_uppin, HIGH);
    digitalWrite(_downpin, LOW);
    delay(time);
  } else {
    digitalWrite(_uppin, LOW);
    digitalWrite(_downpin, HIGH);
    delay(time);
  }

  digitalWrite(_uppin, LOW);
  digitalWrite(_downpin, LOW);
}
void DipperLCD::diamond(bool dir) {
  int rinse_steps = 200;
  int rinse_delay = 50;

  digitalWrite(_uppin, LOW);
  digitalWrite(_downpin, HIGH);
  digitalWrite(_xdirpin, dir);
  for (int i = 0; i < rinse_steps; i++) {
    doStep();
  }

  delay(rinse_delay);

  digitalWrite(_xdirpin, !dir);
  for (int i = 0; i < rinse_steps; i++) {
    doStep();
  }

  delay(rinse_delay);

  digitalWrite(_uppin, HIGH);
  digitalWrite(_downpin, LOW);
  digitalWrite(_xdirpin, !dir);

  for (int i = 0; i < rinse_steps; i++) {
    doStep();
  }

  delay(rinse_delay);

  digitalWrite(_xdirpin, dir);

  for (int i = 0; i < rinse_steps; i++) {
    doStep();
  }

  digitalWrite(_uppin, LOW);
  digitalWrite(_downpin, LOW);
}
void DipperLCD::dip(int diptime) {
  yMoveTime(opheight, false);

  unsigned long current_time = millis();
  int period = diptime * 1000;

  for (int i = 0; i < diptime; i++) {
    delay(1000);
  }

  yMoveTime(opheight + 2500, true);
}
void DipperLCD::rinse(int rinse_cycles) {
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
void DipperLCD::doDipSequence(int layers, int position_array[], int time_array[], int wash_array[], int stops) {
  Serial.println("Starting procedure");
  for (int i = 0; i < layers; i++) {
    for (int j = 0; j < stops; j++) {
      Serial.print("Layer number: ");
      Serial.print(i + 1);
      Serial.print(" Stop: ");
      Serial.println(j + 1);

      /*
      _LCD.clear();
      _LCD.setCursor(0, 0);
      _LCD.print("Layer: ");
      _LCD.print(i + 1);

      _LCD.setCursor(0, 1);
      _LCD.print("Stop: ");
      _LCD.print(j + 1);
      */

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
void DipperLCD::calibrateX() {
    Serial.println("Calibrating horizontal...");

  int limitval = digitalRead(_limitpin);
  digitalWrite(_xdirpin, HIGH);

  while (limitval == 1) {
    limitval = digitalRead(_limitpin);

    doStep();
  }

  position = 0;
}
void DipperLCD::calibrateY() {
  Serial.println("Calibrating vertical...");
  yMoveTime(19000, true);
}
void DipperLCD::joystick(int xval, int yval) {
  int dir;
  static int prev_position = position;

  if (abs(xval) > tolerance) {
    if (xval < 0) {
      digitalWrite(_xdirpin, LOW);
      dir = 1;
    } else {
      digitalWrite(_xdirpin, HIGH);
      dir = -1;
    }

    int limitval = digitalRead(_limitpin);

    if (limitval == 1 || (limitval == 0 && xval < 0)) {
      doStep();
      position += dir;
    }
  } else {
    if (prev_position != position) {
      Serial.print("Position: ");
      Serial.print(0.025 * position);
      Serial.println(" mm");

      prev_position = position;
    }
  }

  if (yval < -1 * tolerance) {
    //moves down
    digitalWrite(_uppin, HIGH);
    digitalWrite(_downpin, LOW);
  } else if (yval > tolerance) {
    //moves up
    digitalWrite(_uppin, LOW);
    digitalWrite(_downpin, HIGH);
  } else {
    digitalWrite(_uppin, LOW);
    digitalWrite(_downpin, LOW);
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