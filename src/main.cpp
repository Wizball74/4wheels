#include <AccelStepper.h>
#include <Arduino.h>

//----------------------------------|  STEPPER  |----------------------------------
#define STEP_MOTOR_01_1 14
#define STEP_MOTOR_01_2 15
#define STEP_MOTOR_01_3 16
#define STEP_MOTOR_01_4 17

#define STEP_MOTOR_02_1 10
#define STEP_MOTOR_02_2 11
#define STEP_MOTOR_02_3 12
#define STEP_MOTOR_02_4 13

#define STEP_MOTOR_03_1 6
#define STEP_MOTOR_03_2 7
#define STEP_MOTOR_03_3 8
#define STEP_MOTOR_03_4 9

#define STEP_MOTOR_04_1 2
#define STEP_MOTOR_04_2 3
#define STEP_MOTOR_04_3 4
#define STEP_MOTOR_04_4 5

//----------------------------------|  WIPPSCHALTER  |----------------------------------
#define SWITCH_N 30
#define SWITCH_W 26
#define SWITCH_S 24
#define SWITCH_E 22

#define SWITCH_N2 31
#define SWITCH_W2 37
#define SWITCH_S2 35
#define SWITCH_E2 33

//----------------------------------|  BUTTONS  |----------------------------------
#define SWITCH_C1 28
#define SWITCH_C2 39


// Pins IN1-IN3-IN2-IN4
/*AccelStepper mainStepperN(AccelStepper::FULL4WIRE, STEP_MOTOR_01_1, STEP_MOTOR_01_3, STEP_MOTOR_01_2, STEP_MOTOR_01_4); // Defaults to AccelStepper::FULL4WIRE (4 pin) on 2, 3, 4, 5
AccelStepper mainStepperE(AccelStepper::FULL4WIRE, STEP_MOTOR_02_1, STEP_MOTOR_02_3, STEP_MOTOR_02_2, STEP_MOTOR_02_4); // Defaults to AccelStepper::FULL4WIRE (4 pins) on 2, 3, 4, 5
AccelStepper mainStepperS(AccelStepper::FULL4WIRE, STEP_MOTOR_03_1, STEP_MOTOR_03_3, STEP_MOTOR_03_2, STEP_MOTOR_03_4); // Defaults to AccelStepper::FULL4WIRE (4 pins) on 2, 3, 4, 5
AccelStepper mainStepperW(AccelStepper::FULL4WIRE, STEP_MOTOR_04_1, STEP_MOTOR_04_3, STEP_MOTOR_04_2, STEP_MOTOR_04_4); // Defaults to AccelStepper::FULL4WIRE (4 pins) on 2, 3, 4, 5

AccelStepper *mainStepper[4] = {mainStepperN, mainStepperE, mainStepperS, mainStepperW};*/
//----------------------------------|  JOYSTICK  |----------------------------------

AccelStepper mainStepper[4] = {
  {AccelStepper::FULL4WIRE, STEP_MOTOR_01_1, STEP_MOTOR_01_3, STEP_MOTOR_01_2, STEP_MOTOR_01_4},
  {AccelStepper::FULL4WIRE, STEP_MOTOR_02_1, STEP_MOTOR_02_3, STEP_MOTOR_02_2, STEP_MOTOR_02_4},
  {AccelStepper::FULL4WIRE, STEP_MOTOR_03_1, STEP_MOTOR_03_3, STEP_MOTOR_03_2, STEP_MOTOR_03_4},
  {AccelStepper::FULL4WIRE, STEP_MOTOR_04_1, STEP_MOTOR_04_3, STEP_MOTOR_04_2, STEP_MOTOR_04_4}
};

const int PIN_JOYSTICK_X  = A0;
const int PIN_JOYSTICK_Y  = A1;
const int PIN_JOYSTICK_SW = 21;
const int JOYSTICK_TOLERANCE = 10;

const int SELECTED_DIRECTION_N = 0;
const int SELECTED_DIRECTION_NE = 45;
const int SELECTED_DIRECTION_E = 90;
const int SELECTED_DIRECTION_SE = 135;
const int SELECTED_DIRECTION_S = 180;
const int SELECTED_DIRECTION_SW = 225;
const int SELECTED_DIRECTION_W = 270;
const int SELECTED_DIRECTION_NW = 315;
const int SELECTED_DIRECTION_NONE = -1;

const int DEFAULT_SPEED = 600;
int joystickOldX = 600;
int joystickOldY = 600;
int joystickOldSW = false;
int selectedDirection = -1;
int selectedDirectionOld = -1;
bool wheelRunning[4] = {false, false, false, false};
bool wheelClockwise[4] = {true, true, true, true};
bool wheelClockwiseOld[4] = {true, true, true, true};
bool wheelStop[4] = {false, false, false, false};
bool wheelStopOld[4] = {false, false, false, false};
bool switchC1Old = false;
bool switchC2Old = false;
int lastSwitchChanged1 = -1;
int lastSwitchChanged2 = -1;

int getDirection(int x, int y) {
  if (x < 200) {
    if (y < 200) {
      return(SELECTED_DIRECTION_NW);
    } else if (y > 800) {
      return(SELECTED_DIRECTION_SW);
    } else {
      return(SELECTED_DIRECTION_W);
    }
  } else if (x > 800) {
    if (y < 200) {
      return(SELECTED_DIRECTION_NE);
    } else if (y > 800) {
      return(SELECTED_DIRECTION_SE);
    } else {
      return(SELECTED_DIRECTION_E);
    }
  } else {
    if (y < 200) {
      return(SELECTED_DIRECTION_N);
    } else if (y > 800) {
      return(SELECTED_DIRECTION_S);
    }
  }

  return(SELECTED_DIRECTION_NONE);
}

void setWheelSpeed(int nr, int speed) {
  /*char buffer[50];
  sprintf(buffer, "%d : %d (%d)", nr, speed, wheelClockwise[nr]);
  Serial.println (buffer);
  return;*/

  if (speed <= 0) {
    mainStepper[nr].stop();
    wheelRunning[nr] = false;
    wheelStopOld[nr] = wheelStop[nr];
  } else {
    mainStepper[nr].setSpeed(wheelClockwise[nr] ? speed : -speed);
    wheelRunning[nr] = true;
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(PIN_JOYSTICK_SW, INPUT_PULLUP);
  pinMode(SWITCH_N, INPUT_PULLUP);
  pinMode(SWITCH_E, INPUT_PULLUP);
  pinMode(SWITCH_S, INPUT_PULLUP);
  pinMode(SWITCH_W, INPUT_PULLUP);
  pinMode(SWITCH_C1, INPUT_PULLUP);
  pinMode(SWITCH_N2, INPUT_PULLUP);
  pinMode(SWITCH_E2, INPUT_PULLUP);
  pinMode(SWITCH_S2, INPUT_PULLUP);
  pinMode(SWITCH_W2, INPUT_PULLUP);
  pinMode(SWITCH_C2, INPUT_PULLUP);

  for(int i = 0; i < 4; i++) {
    mainStepper[i].setMaxSpeed(DEFAULT_SPEED);
  }
}

void loop() {
  //read joystick
  int i;
  int x = analogRead(PIN_JOYSTICK_X);   // 0–1023
  int y = analogRead(PIN_JOYSTICK_Y);   // 0–1023
  bool sw = !digitalRead(PIN_JOYSTICK_SW); // gedrückt = true

  //read switches and buttons
  wheelClockwise[0] = !digitalRead(SWITCH_N);
  wheelClockwise[1] = !digitalRead(SWITCH_E);
  wheelClockwise[2] = !digitalRead(SWITCH_S);
  wheelClockwise[3] = !digitalRead(SWITCH_W);
  bool switchC1 =      !digitalRead(SWITCH_C1);

  wheelStop[0] =      !digitalRead(SWITCH_N2);
  wheelStop[1] =      !digitalRead(SWITCH_E2);
  wheelStop[2] =      !digitalRead(SWITCH_S2);
  wheelStop[3] =      !digitalRead(SWITCH_W2);
  bool switchC2 =     !digitalRead(SWITCH_C2);

  /*char buffer[50];
  sprintf(buffer, "N: %d | E: %d | S: %d | W: %d", !digitalRead(SWITCH_N2), !digitalRead(SWITCH_E2), !digitalRead(SWITCH_S2), !digitalRead(SWITCH_W2));
  Serial.println (buffer);
  delay(2000);
  return;*/

  bool changeOfDirection = false;
  for (i = 0; i < 4; i++) {
    if (wheelClockwise[i] != wheelClockwiseOld[i]) {
      wheelClockwiseOld[i] = wheelClockwise[i];
      lastSwitchChanged1 = i;
      changeOfDirection = true;
    }

    if (wheelStop[i] != wheelStopOld[i]) {
      wheelStopOld[i] = wheelStop[i];
      lastSwitchChanged2 = i;
      changeOfDirection = true;
    }
  }

  if (joystickOldSW != sw) {
    if (sw) {
      //Joystick clicked -->
    }
    joystickOldSW = sw;
  }

  if (switchC1Old != switchC1) {
    if (lastSwitchChanged1 >= 0) {
      wheelClockwise[lastSwitchChanged1] = !wheelClockwise[lastSwitchChanged1];
    }
    switchC1Old = switchC1;
    changeOfDirection = true;
  }

  if ((max(joystickOldX, x) - min(joystickOldX, x) > JOYSTICK_TOLERANCE)
   || (max(joystickOldY, y) - min(joystickOldY, y) > JOYSTICK_TOLERANCE)
   || changeOfDirection) {
    //Joystick or wheels changed direction
    selectedDirection = getDirection(x, y);
    if (selectedDirection != selectedDirectionOld || changeOfDirection) {
      //Neue Richtung!
      switch(selectedDirection) {

        case SELECTED_DIRECTION_N:
          setWheelSpeed(0, DEFAULT_SPEED);      //N
          setWheelSpeed(1, DEFAULT_SPEED / 3);  //E
          setWheelSpeed(2, 0);                  //S
          setWheelSpeed(3, DEFAULT_SPEED / 3);  //W
          break;

        case SELECTED_DIRECTION_S:
          setWheelSpeed(0, 0);                  //N
          setWheelSpeed(1, DEFAULT_SPEED / 3);  //E
          setWheelSpeed(2, DEFAULT_SPEED);      //S
          setWheelSpeed(3, DEFAULT_SPEED / 3);  //W
          break;

        case SELECTED_DIRECTION_W:
          setWheelSpeed(0, DEFAULT_SPEED / 3);  //N
          setWheelSpeed(1, 0);                  //E
          setWheelSpeed(2, DEFAULT_SPEED / 3);  //S
          setWheelSpeed(3, DEFAULT_SPEED);  //W
          break;

        case SELECTED_DIRECTION_E:
          setWheelSpeed(0, DEFAULT_SPEED / 3);  //N
          setWheelSpeed(1, DEFAULT_SPEED);      //E
          setWheelSpeed(2, DEFAULT_SPEED / 3);  //S
          setWheelSpeed(3, 0);                  //W
          break;

        case SELECTED_DIRECTION_NE:
          setWheelSpeed(0, DEFAULT_SPEED);      //N
          setWheelSpeed(1, DEFAULT_SPEED);      //E
          setWheelSpeed(2, 0);                  //S
          setWheelSpeed(3, 0);                  //W
          break;

        case SELECTED_DIRECTION_SE:
          setWheelSpeed(0, 0);                  //N
          setWheelSpeed(1, DEFAULT_SPEED);      //E
          setWheelSpeed(2, DEFAULT_SPEED);      //S
          setWheelSpeed(3, 0);                  //W
          break;

        case SELECTED_DIRECTION_SW:
          setWheelSpeed(0, 0);                  //N
          setWheelSpeed(1, 0);                  //E
          setWheelSpeed(2, DEFAULT_SPEED);      //S
          setWheelSpeed(3, DEFAULT_SPEED);      //W
          break;

        case SELECTED_DIRECTION_NW:
          setWheelSpeed(0, DEFAULT_SPEED);      //N
          setWheelSpeed(1, 0);                  //E
          setWheelSpeed(2, 0);                  //S
          setWheelSpeed(3, DEFAULT_SPEED);      //W
          break;

        case SELECTED_DIRECTION_NONE:
        default:
          //STOP!
          setWheelSpeed(0, 0); //N
          setWheelSpeed(1, 0); //E
          setWheelSpeed(2, 0); //S
          setWheelSpeed(3, 0); //W
          break;

      }

      selectedDirectionOld = selectedDirection;
    }

    joystickOldX = x;
    joystickOldY = y;
  }

  for (i = 0; i < 4; i++) {
    if (wheelRunning[i]) {
      mainStepper[i].runSpeed();
    }
  }
}
