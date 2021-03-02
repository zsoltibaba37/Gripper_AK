/*
   Gripper with 28HS51 and DRV8825

   Full Step 200step/rev

   0.4 - A5 Lock Input. If LOW Close Gripper Else Open
   0.3 - Two Limitswitch - Home - End
   0.2 - Homing working

*/
float version = 0.4;

#include "AccelStepper.h"

const int stepPin = 3;
const int dirPin = 2;
#define motorInterfaceType 1

AccelStepper stepper = AccelStepper(motorInterfaceType, stepPin, dirPin);

#define home_switch 5
#define end_switch 6
#define LockPin A5
bool LockState = false;

long TravelX;
int move_finished = 1;
long initial_homing = -1;
long initial_ending = -1;

// Speeds
int homingMaxSpeed = 600;
int homingAccel = 200;
int homingSpeed = 500;

int endingMaxSpeed = 800;
int endingAccel = 600;
int endingSpeed = 800;

long maxPosition = 0;

unsigned int MaxTraSpeed = 1200;
unsigned int TraAccSpeed = 800;
unsigned int travelSpeed = 1100;

// Positions
//long maxPosition = 5000;

//
bool goHomeState = false;

void writeMess();
void goHome();
void serialComm();

// ---------- SETUP ----------
// ---------- SETUP ----------
void setup() {
  Serial.begin(115200);
  Serial.println("#####################################");
  Serial.println("#        DRV8825 Gripper Test       #");
  Serial.print  ("               ");
  Serial.println(String(version));
  Serial.println("#        With serial control        #");
  Serial.println("#####################################");
  Serial.println("");
  pinMode(home_switch, INPUT_PULLUP);
  pinMode(end_switch, INPUT_PULLUP);
  pinMode(LockPin, INPUT_PULLUP);
  delay(5);

  // ---------- Homing ----------
  stepper.setMaxSpeed(homingMaxSpeed);
  stepper.setSpeed(homingSpeed);
  stepper.setAcceleration(homingAccel);
  Serial.println("The Gripper is Homing . .");

  while (digitalRead(home_switch)) {
    stepper.moveTo(initial_homing);
    initial_homing--;
    stepper.run();
    delay(1);
  }

  stepper.setCurrentPosition(0);
  stepper.setMaxSpeed(homingMaxSpeed);
  stepper.setSpeed(homingSpeed);
  stepper.setAcceleration(homingAccel);
  initial_homing = 1;

  while (!digitalRead(home_switch)) {
    stepper.moveTo(initial_homing);
    stepper.run();
    initial_homing++;
    delay(1);
  }

  stepper.setCurrentPosition(0);
  //int curPos = stepper.currentPosition();
  Serial.println("Gripper Homing Completed !!!");
  Serial.println("");
  // ---------- END Homing ----------
  delay(500);
  // ---------- Find End ----------
  stepper.setMaxSpeed(endingMaxSpeed);
  stepper.setSpeed(endingSpeed);
  stepper.setAcceleration(endingAccel);
  Serial.println("The Gripper search End Position . .");

  while (digitalRead(end_switch)) {
    stepper.moveTo(initial_ending);
    initial_ending++;
    stepper.run();
    delay(1);
  }

  maxPosition = stepper.currentPosition();
  //stepper.setCurrentPosition(0);
  stepper.setMaxSpeed(endingMaxSpeed);
  stepper.setSpeed(endingSpeed);
  stepper.setAcceleration(endingAccel);
  initial_ending = 1;

  while (!digitalRead(end_switch)) {
    stepper.moveTo(initial_ending);
    stepper.run();
    initial_ending--;
    delay(1);
  }

  Serial.println("Gripper Find End Position !!!");
  Serial.println("");
  // ---------- END Find End ----------

  stepper.setMaxSpeed(MaxTraSpeed);
  stepper.setAcceleration(TraAccSpeed);
  stepper.setSpeed(travelSpeed);

  // ---------- GO HOME ----------
  delay(600);
  goHome();
  Serial.println("Gripper is at Home Position!");
  Serial.println("");
  writeMess();

}

// -------------------- LOOP --------------------
// -------------------- LOOP --------------------
void loop() {
  openClose();

  //serialComm();
  
  delay(1);
}
// ----------------- END LOOP -------------------


void openClose() {
  while (LockState == false) {
    stepper.setSpeed(travelSpeed);
    stepper.moveTo(0);
    stepper.runSpeedToPosition();
    if (!digitalRead(LockPin)) {
      LockState = true;
    }
  }  
  while (LockState == true) {
    stepper.setSpeed(travelSpeed);
    stepper.moveTo(maxPosition);
    stepper.runSpeedToPosition();
    if ((stepper.distanceToGo() == 0) && digitalRead(LockPin)) {
      LockState = false;
    }
  }
}


void writeMess() {
  Serial.print("Enter Travel distance ");
  Serial.print("[0 - ");
  Serial.print(maxPosition);
  Serial.println("]:");
}

void goHome() {
  while ((stepper.distanceToGo() != 0)) {
    stepper.setMaxSpeed(MaxTraSpeed);
    stepper.setAcceleration(TraAccSpeed);
    stepper.setSpeed(travelSpeed);

    stepper.moveTo(0);
    stepper.runSpeedToPosition();
  }
}

void serialComm() {
  while (Serial.available() > 0)  {

    move_finished = 0;

    TravelX = Serial.parseInt();
    if (TravelX < 0 || TravelX > maxPosition) {
      Serial.println("");
      Serial.print("Incorrect Value! ");
      Serial.print("[ 0 - ");
      Serial.print(maxPosition);
      Serial.println(" ]");
      Serial.println("");
    }
    else {
      Serial.print("Moving Gripper into position: ");
      Serial.println(TravelX);
      stepper.moveTo(TravelX);

    }
    if (TravelX >= 0 && TravelX <= maxPosition) {


      while ((stepper.distanceToGo() != 0)) {
        //stepper.setMaxSpeed(MaxTraSpeed);
        stepper.setAcceleration(TraAccSpeed);
        stepper.setSpeed(travelSpeed);
        //stepper.run();
        stepper.runSpeedToPosition();

      }
    }

    if ((move_finished == 0) && (stepper.distanceToGo() == 0)) {
      Serial.println("Gripper is in Position!");
      Serial.println("");
      writeMess();
      //Serial.println("Enter Travel distance (Positive for CW / Negative for CCW and Zero for back to Home): ");
      move_finished = 1;
    }

  }
  
  // go home when press home_switch
//  if (!digitalRead(home_switch) && goHomeState == false) {
//    Serial.println("Now Gripper is Go to Home Position...");
//    goHomeState = true;
//  }
//
//  while (goHomeState == true) {
//    stepper.setMaxSpeed(MaxTraSpeed);
//    stepper.setAcceleration(TraAccSpeed);
//    stepper.setSpeed(travelSpeed);
//    stepper.moveTo(0);
//    stepper.runSpeedToPosition();
//    if ((stepper.distanceToGo() == 0)) {
//      Serial.println("");
//      Serial.println("Gripper is at Home Position!");
//      Serial.println("");
//      writeMess();
//      goHomeState = false;
//    }
//  }
}
