// Sets pins for the stepper motor. xdirpin controls directions and xpulpin sends a pulse corresponding to a step
#define xdirpin 30
#define xpulpin 31

//Sets pins for the dipping (y movement). HIGH LOW is up(?) and LOW HIGH is down
#define upPin 40
#define downPin 41

//Emergency switch pin. Currently unused
//#define killerPin 22

//Joystick pins
#define xJoyPin A2    //contorls horizontal movement (stepper)
#define yJoyPin A3    // controls vertical movement (linear actuator)
#define joyButton 38  //centre button of the joystick

//Limit switch button
#define limitPin 48

int tolerance = 30;
int xJoy0;
int yJoy0;
int xposition = 0;
//maximum x position
int xposmax = 200;


//Variables for running the dipping program

//number of cycles you want the program to run
int nlayers = 100;

//number of dips per cycle
int ndips = 4;



void setup() {
  // Begins communication with the serial monitor
  Serial.begin(9600);

  //Sets pins as outputs
  pinMode(xdirpin, OUTPUT);
  pinMode(xpulpin, OUTPUT);

  pinMode(upPin, OUTPUT);
  pinMode(downPin, OUTPUT);

  //pinMode(killerPin, INPUT);
  pinMode(xJoyPin, INPUT);
  pinMode(yJoyPin, INPUT);
  pinMode(joyButton, INPUT_PULLUP);
  pinMode(limitPin, INPUT_PULLUP);

  //Calibrates the joystick
  xJoy0 = analogRead(xJoyPin);
  yJoy0 = analogRead(yJoyPin);

  //calibrates y-direction
  ymove(20000, true);

  //calibrates the x-direction
  calibrateX();

  xMoveSteps(20, 1000);
  ymove(11000, false);
}

void loop() {
  int buttonStatus = digitalRead(joyButton);

  int positions[4] = { 20, 60, 100, 60 };
  int times[4] = { 15, 0, 15, 0 };
  bool doRinse[4] = { false, true, false, true };

  if (buttonStatus == 0) {
    //begins dipping cycles
    for (int i = 0; i < nlayers; i++) {
      for (int j = 0; j < 4; j++) {

        if (doRinse[j] == true) {
          rinse(20, 7000, 7000);
        } else {
          dip(times[j], 7000, 7000);
        }

        xMoveSteps(positions[(j + 1) % 4] - positions[j % 4], 1000);
      }
    }
  }
}

void calibrateX() {
  int limitVal = digitalRead(limitPin);
  digitalWrite(xdirpin, HIGH);

  while (limitVal == 1) {
    limitVal = digitalRead(limitPin);

    digitalWrite(xpulpin, HIGH);
    delayMicroseconds(1500);
    digitalWrite(xpulpin, LOW);
    delayMicroseconds(1500);
  }

  xposition = 0;
}

//dir=1 means up, dir=-1 means down
void ymove(int time, bool up) {
  if (up == true) {
    digitalWrite(upPin, HIGH);
    digitalWrite(downPin, LOW);
    delay(time);
  } else {
    digitalWrite(upPin, LOW);
    digitalWrite(downPin, HIGH);
    delay(time);
  }
  digitalWrite(upPin, LOW);
  digitalWrite(downPin, LOW);
}

//diptime given in milliseconds
void dip(int diptime, int downtime, int uptime) {
  //dips y-direction
  ymove(downtime, false);

  //waits
  for (int i = 0; i < diptime; i++) {
    delay(1000);
  }
  //brings y-direction up
  ymove(uptime, true);
}


void rinse(int cycles, int downtime, int uptime) {
  //dips y-direction
  ymove(downtime, false);

  int xrinse = 200;
  int xdelay = 50;
  int rinsespeed = 900;

  //rinse cycles
  for (int i = 0; i < cycles; i++) {
    //set xdirection right
    digitalWrite(xdirpin, LOW);
    //set ymovement up
    digitalWrite(upPin, LOW);
    digitalWrite(downPin, HIGH);

    for (int i = 0; i < xrinse; i++) {
      // moves x
      digitalWrite(xpulpin, HIGH);
      delayMicroseconds(rinsespeed);
      digitalWrite(xpulpin, LOW);
      delayMicroseconds(rinsespeed);
    }

    delay(xdelay);

    //set xdirection left
    digitalWrite(xdirpin, HIGH);

    for (int i = 0; i < xrinse; i++) {
      // moves x
      digitalWrite(xpulpin, HIGH);
      delayMicroseconds(rinsespeed);
      digitalWrite(xpulpin, LOW);
      delayMicroseconds(rinsespeed);
    }

    delay(xdelay);

    //set xdirection right
    digitalWrite(xdirpin, LOW);
    //set ymovement down
    digitalWrite(upPin, HIGH);
    digitalWrite(downPin, LOW);

    for (int i = 0; i < xrinse; i++) {
      // moves x
      digitalWrite(xpulpin, HIGH);
      delayMicroseconds(rinsespeed);
      digitalWrite(xpulpin, LOW);
      delayMicroseconds(rinsespeed);
    }

    delay(xdelay);

    //set xdirection left
    digitalWrite(xdirpin, HIGH);

    for (int i = 0; i < xrinse; i++) {
      // moves x
      digitalWrite(xpulpin, HIGH);
      delayMicroseconds(rinsespeed);
      digitalWrite(xpulpin, LOW);
      delayMicroseconds(rinsespeed);
    }

    delay(xdelay);
  }
  ymove(1000, false);

  //brings y-direction up
  ymove(uptime, true);
}



//give positive halfsteps for right, or negative halfsteps for left
//speeddelay= 1000 is standard
void xMoveSteps(int halfsteps, int speeddelay) {
  //controls and changes direction
  if (halfsteps < 0) {
    digitalWrite(xdirpin, HIGH);
  } else {
    digitalWrite(xdirpin, LOW);
  }

  //moves
  for (int i = 0; i < abs(halfsteps) * 100; i++) {
    int limitVal = digitalRead(limitPin);


    if (limitVal == 0 && halfsteps < 0) {
      break;
    }

    digitalWrite(xpulpin, HIGH);
    delayMicroseconds(speeddelay);
    digitalWrite(xpulpin, LOW);
    delayMicroseconds(speeddelay);
  }
}
