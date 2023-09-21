#ifndef DIPPER_AND_COMMS__H
#define DIPPER_AND_COMMS_H
#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

class Dipper {
private:
  int tolerance = 20;  //Tolerance for the joystick
  int posmax = 20000;  //Maximum displacement from 0 in units of steps

  //Pins for horizontal movement
  byte xdirpin;
  byte xpulpin;

  //pins for verticla movement
  byte uppin;
  byte downpin;

  //limit switch pin
  byte limitpin;

  int position;            //Position, in steps, of the stepper motor
  int opheight = 13000;    //Height in milliseconds that the linear actuator lowers
  int pulse_speed = 1000;  //Pulse width in ms


  //True if an LCD is passed in the contructor
  bool LCDon = false;
  //LCD display
  LiquidCrystal_I2C& LCD;

  //Sends a single pulse to the stepper
  void doStep();


public:
  //Contructor without the LED screen
  Dipper(byte xdp, byte xpp, byte up, byte dp, byte lp);

  //Contructor with the LED screen
  Dipper(byte xdp, byte xpp, byte up, byte dp, byte lp, LiquidCrystal_I2C lcd);

  //
  void xMoveSteps(int steps);

  void xMoveTo(int finalPlace);

  void yMoveTime(int time, bool up);

  void diamond(bool dir);

  void dip(int diptime);

  void rinse(int rinse_cycles);

  void doDipSequence(int layers, int position_array[], int time_array[], int wash_array[], int stops);

  void calibrateX();

  void calibrateY();

  void joystick(int xval, int yval);
};

class Comms {
private:
  const static byte numChars = 64;

public:
  int layers;
  int stops;
  int positions[20];
  int times[20];
  int washCycles[20];

  char receivedChars[numChars];
  bool newData = false;
  bool beginJob = false;

  void readSerialData();
  void parseString(char inputStr[]);
  void presentData(int index);
  void arrayReset();
};
#endif