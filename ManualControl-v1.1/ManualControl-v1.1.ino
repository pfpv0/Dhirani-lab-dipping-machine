/* June 7 2023

*/

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
#define joyButton 38  //centre button of the joystick, Currently unused

//Limit switch button
#define limitPin 48

int tolerance = 30;
int xJoy0 = 0;
int yJoy0 = 0;
int xposition = 0;
//maximum x position
int xposmax = 200;

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

}

void loop() {
  //Reads for joystick movement
  int yJoyNow = analogRead(yJoyPin);
  int yJoyDiff = yJoyNow - yJoy0;

  int xJoyNow = analogRead(xJoyPin);
  int xJoyDiff = xJoyNow - xJoy0;

  yMove(tolerance, yJoyDiff);
  xMove(tolerance, xJoyDiff);

  int limitVal = digitalRead(limitPin);

  // this gives the x-position, 1 step is approximately 1 cm
  if(xJoyDiff < -1 * tolerance && limitVal == 1){
    xposition ++;
  }
  else if(xJoyDiff > tolerance && limitVal == 1){
    xposition --;
  }
  else if(limitVal == 0){
    xposition = 0;
  }

  
  Serial.print(0);
  Serial.print(", ");
  Serial.print(xposmax);
  Serial.print(", ");
  Serial.println(xposition);
}

void xMove(int tol, int xInput) {
  int speed = 100;

  //controls and changes direction
  if (xInput < 0) {
    digitalWrite(xdirpin, LOW);
  } else {
    digitalWrite(xdirpin, HIGH);
  }

//controls movement
  if (abs(xInput) > tol) {
    for (int i = 0; i < speed; i++) {
      //int killer = digitalRead(killerPin);
      int limitVal = digitalRead(limitPin);
    

      if (limitVal == 0 && xInput > 0) {
        break;
      }
      
      digitalWrite(xpulpin, HIGH);
      delayMicroseconds(1000);
      digitalWrite(xpulpin,LOW);
      delayMicroseconds(1000);
      
    }
  }
}

void yMove(int tol, int yInput) {
  //Tol is a given tolerance. It helps account for noise in the analog reading of the joystick
  if (yInput < -1 * tol) {
    //moves down
    digitalWrite(upPin, HIGH);
    digitalWrite(downPin, LOW);
  } else if (yInput > tol) {
    //moves up
    digitalWrite(upPin, LOW);
    digitalWrite(downPin, HIGH);
  } else {
    digitalWrite(upPin, LOW);
    digitalWrite(downPin, LOW);
  }
}
