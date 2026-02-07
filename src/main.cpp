#include <Arduino.h>
#include <AccelStepper.h>
#include <TM1637TinyDisplay.h>


//----------------------------------| STEPPER PINS |----------------------------------
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

//----------------------------------| DIRECTION SWITCHES |----------------------------
#define SWITCH_N 30
#define SWITCH_E 22
#define SWITCH_S 24
#define SWITCH_W 26

#define SWITCH_N2 31
#define SWITCH_W2 37
#define SWITCH_S2 35
#define SWITCH_E2 33

//----------------------------------|  BUTTONS  |----------------------------------
#define SWITCH_C1 28
#define SWITCH_C2 39

//----------------------------------| JOYSTICK |--------------------------------------
#define PIN_JOYSTICK_X A0
#define PIN_JOYSTICK_Y A1
#define PIN_JOYSTICK_SW 21

#define JOY_CENTER   512
#define JOY_DEADZONE 80

//----------------------------------| LEDS |------------------------------------------
#define LED_1 32
#define LED_2 34
#define LED_3 36
#define LED_4 38

//----------------------------------| 4 DIGIT DISPLAY |------------------------------------------
#define DIGIT_CLK 41
#define DIGIT_DIO 43

TM1637TinyDisplay digit(DIGIT_CLK, DIGIT_DIO);

//----------------------------------| SPEED SETUP |-----------------------------------
#define MAX_SPEED   600
#define SPEED_RATIO 10
#define STEPS_PER_REV 4096
#define DETENT_STEPS  (STEPS_PER_REV / 16) // 22.5 degrees per detent
#define SNAP_MAX_SPEED 300
#define SNAP_ACCEL     800

//----------------------------------| STEPPERS |--------------------------------------
AccelStepper stepper[4] = {
  {AccelStepper::FULL4WIRE, STEP_MOTOR_01_1, STEP_MOTOR_01_3, STEP_MOTOR_01_2, STEP_MOTOR_01_4}, // N
  {AccelStepper::FULL4WIRE, STEP_MOTOR_02_1, STEP_MOTOR_02_3, STEP_MOTOR_02_2, STEP_MOTOR_02_4}, // E
  {AccelStepper::FULL4WIRE, STEP_MOTOR_03_1, STEP_MOTOR_03_3, STEP_MOTOR_03_2, STEP_MOTOR_03_4}, // S
  {AccelStepper::FULL4WIRE, STEP_MOTOR_04_1, STEP_MOTOR_04_3, STEP_MOTOR_04_2, STEP_MOTOR_04_4}  // W
};

//----------------------------------| STATE |-----------------------------------------
int joyX = 0;
int joyY = 0;

bool wheelClockwise[4] = {true, true, true, true};
int  wheelSpeed[4]     = {0, 0, 0, 0};
bool snapActive[4]     = {false, false, false, false};
bool movementActive    = false;
bool wasMoving         = false;
bool lightStatus[4]    = {true, true, true, true};

constexpr uint8_t ledPins[4] = {LED_1, LED_2, LED_3, LED_4};

//----------------------------------| DIRECTION ENUM |--------------------------------
enum Direction {
  DIR_NONE,
  DIR_N, DIR_NE, DIR_E, DIR_SE,
  DIR_S, DIR_SW, DIR_W, DIR_NW
};

//----------------------------------| INPUT |-----------------------------------------
void readInputs() {
  joyX = analogRead(PIN_JOYSTICK_X) - JOY_CENTER;
  joyY = JOY_CENTER - analogRead(PIN_JOYSTICK_Y); // Y-Achse invertiert

  wheelClockwise[0] = !digitalRead(SWITCH_N);
  wheelClockwise[1] = !digitalRead(SWITCH_E);
  wheelClockwise[2] = !digitalRead(SWITCH_S);
  wheelClockwise[3] = !digitalRead(SWITCH_W);

  lightStatus[0] = !digitalRead(SWITCH_N2);
  lightStatus[1] = !digitalRead(SWITCH_E2);
  lightStatus[2] = !digitalRead(SWITCH_S2);
  lightStatus[3] = !digitalRead(SWITCH_W2);

  for (int i = 0; i < 4; i++) {
    digitalWrite(ledPins[i], lightStatus[i] ? HIGH : LOW);
  }
}

//----------------------------------| JOYSTICK → DIRECTION |---------------------------
Direction getDirection(int x, int y) {

  if (abs(x) < JOY_DEADZONE && abs(y) < JOY_DEADZONE) {
    return DIR_NONE;
  }

  if (y > JOY_DEADZONE) {
    if (x > JOY_DEADZONE)  return DIR_NE;
    if (x < -JOY_DEADZONE) return DIR_NW;
    return DIR_N;
  }

  if (y < -JOY_DEADZONE) {
    if (x > JOY_DEADZONE)  return DIR_SE;
    if (x < -JOY_DEADZONE) return DIR_SW;
    return DIR_S;
  }

  if (x > JOY_DEADZONE)  return DIR_E;
  if (x < -JOY_DEADZONE) return DIR_W;

  return DIR_NONE;
}

//----------------------------------| SPEED COMPUTATION |------------------------------
int computeBaseSpeed(int x, int y) {
  int magnitude = max(abs(x), abs(y));
  magnitude = constrain(magnitude, JOY_DEADZONE, JOY_CENTER);

  return static_cast<int>(map(magnitude, JOY_DEADZONE, JOY_CENTER, 0, MAX_SPEED));
}

long snapToDetent(long pos) {
  long remainder = pos % DETENT_STEPS;
  long target = pos - remainder;

  if (abs(remainder) >= (DETENT_STEPS / 2)) {
    target += (remainder > 0) ? DETENT_STEPS : -DETENT_STEPS;
  }

  return target;
}

//----------------------------------| LOGIC |-----------------------------------------
void calculateWheelSpeeds() {

  for (int & i : wheelSpeed) {
    i = 0;
  }

  Direction dir = getDirection(joyX, joyY);
  int baseSpeed = computeBaseSpeed(joyX, joyY);
  int sideSpeed = baseSpeed / SPEED_RATIO;
  movementActive = (dir != DIR_NONE) && (baseSpeed > 0);

  switch (dir) {

        case DIR_N:
      // Joystick nach oben → physisch Nord
      wheelSpeed[0] = baseSpeed;
      wheelSpeed[1] = sideSpeed;
      wheelSpeed[3] = sideSpeed;
      break;

    case DIR_S:
      // Joystick nach unten → physisch Süd
      wheelSpeed[2] = baseSpeed;
      wheelSpeed[1] = sideSpeed;
      wheelSpeed[3] = sideSpeed;
      break;

    case DIR_E:
      wheelSpeed[1] = baseSpeed;
      wheelSpeed[0] = sideSpeed;
      wheelSpeed[2] = sideSpeed;
      break;

    case DIR_W:
      wheelSpeed[3] = baseSpeed;
      wheelSpeed[0] = sideSpeed;
      wheelSpeed[2] = sideSpeed;
      break;

    case DIR_NE:
      wheelSpeed[0] = baseSpeed;
      wheelSpeed[1] = baseSpeed;
      break;

    case DIR_SE:
      wheelSpeed[1] = baseSpeed;
      wheelSpeed[2] = baseSpeed;
      break;

    case DIR_SW:
      wheelSpeed[2] = baseSpeed;
      wheelSpeed[3] = baseSpeed;
      break;

    case DIR_NW:
      wheelSpeed[3] = baseSpeed;
      wheelSpeed[0] = baseSpeed;
      break;

    case DIR_NONE:
    default:
      break;
  }
}

//----------------------------------| OUTPUT |----------------------------------------
void applyStepperSpeeds() {
  if (movementActive) {
    wasMoving = true;
    for (int i = 0; i < 4; i++) {
      snapActive[i] = false;
      stepper[i].setMaxSpeed(MAX_SPEED);
      int signedSpeed = wheelClockwise[i] ? wheelSpeed[i] : -wheelSpeed[i];
      stepper[i].setSpeed(signedSpeed);
      stepper[i].runSpeed();
    }
    return;
  }

  if (wasMoving) {
    for (int i = 0; i < 4; i++) {
      stepper[i].setMaxSpeed(SNAP_MAX_SPEED);
      stepper[i].moveTo(snapToDetent(stepper[i].currentPosition()));
      snapActive[i] = true;
    }
  }

  for (int i = 0; i < 4; i++) {
    if (!snapActive[i]) {
      continue;
    }

    if (stepper[i].distanceToGo() != 0) {
      stepper[i].run();
    } else {
      snapActive[i] = false;
    }
  }

  wasMoving = false;
}

void setDigit() {
  //setting the display
}

//----------------------------------| SETUP |-----------------------------------------
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

  pinMode(LED_1, OUTPUT);
  pinMode(LED_2, OUTPUT);
  pinMode(LED_3, OUTPUT);
  pinMode(LED_4, OUTPUT);

  digit.begin();
  digit.flipDisplay(true);
  digit.showString("HELLO");
  digit.setBrightness(8);

  for (unsigned char ledPin : ledPins) {
    digitalWrite(ledPin, LOW);
  }

  for (auto & i : stepper) {
    i.setMaxSpeed(MAX_SPEED);
    i.setAcceleration(SNAP_ACCEL);
    i.setSpeed(0);
  }
  delay(500);
  digit.clear();
}

//----------------------------------| LOOP |------------------------------------------
void loop() {
  setDigit();
  readInputs();
  calculateWheelSpeeds();
  applyStepperSpeeds();
}
