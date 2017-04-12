#include <Pushbutton.h>
#include <PLab_ZumoMotors.h>
#include <ZumoMotors.h>
#include <QTRSensors.h>
#include <ZumoReflectanceSensorArray.h>
#include <NewPing.h>
#include <PLab_ZumoMotors.h>

#define QTR_THRESHOLD  1500
#define NUM_SENSORS 6
unsigned int sensor_values[NUM_SENSORS];

ZumoReflectanceSensorArray reflectanceSensors; //(QTR_NO_EMITTER_PIN)

/**
  ECHO Hvit ledning
  TRIG Blå ledning
  5V Rød ledning
  Ground Svart ledning
**/

PLab_ZumoMotors PLab_motors;
Pushbutton button(ZUMO_BUTTON);
ZumoMotors motors;
PLab_ZumoMotors plab_Motors;

int speeD = 350; //Farten som roboten bruker når den har funnet noe
int searchSpeed = 250; //Farten som roboten bruker når den søker
int maxLength = 30; //Setter maks lengde på hva sensorne skal finne

bool seek = true; // For å besteme om vi skal søke eller ikke
bool foundLeft = false; //Bool for å vite om vi har funnet noe på venstre fremme sensor
bool foundRight = false; //Bool for å vite om vi har funnet noe på høyre fremme sensor
bool foundRightSide = false; //Bool for å vite om vi har funnet noe på sensor på høyre side
bool foundRear = false; //Bool for å vite pm vi har funnet noe på sensor bak
bool foundLeftSide = false; //Bool for å vite om vi har funnet noe på sensor på venstre side
bool turnRight = false; //Bestemer om vi skal søke til høyre eller ikke

NewPing sonarL(5, 5, 35);
NewPing sonarR(6, 6, 35);
NewPing sonarRS(A4, A4, 35);
NewPing sonarRear(A5, A5, 35);
NewPing sonarLS(A1, A1, 35);

void setup() {
  reflectanceSensors.init();
  Serial.begin(9600);
  button.waitForButton();

  // For løkken kalibrer IR sensorne
  for(int i = 0; i < 80; i++) {
    if ((i > 10 && i <= 30) || (i > 50 && i <= 70)) {
      motors.setSpeeds(-(speeD), speeD);
    }
    else {
      motors.setSpeeds(speeD, -(speeD));
    }
    reflectanceSensors.calibrate();
    delay(2);
  }
  motors.setSpeeds(0, 0);
  button.waitForButton();
}

// Main loop
void loop() {
  pingR(sonarL, &foundLeft);
  pingR(sonarR, &foundRight);
  AI();
}

void pingR(NewPing sonar, bool *found) {
  float cmLeftS = sonar.convert_cm(sonar.ping());
  if(cmLeftS > 1 && cmLeftS < 30) {
    *found = true;
  }
  else{
    *found = false;
  }
}

void allPing() {
  pingR(sonarL, &foundLeft);
  pingR(sonarR, &foundRight);
  pingR(sonarRS, &foundRightSide);
  pingR(sonarRear, &foundRear);
  pingR(sonarLS, &foundLeftSide);
}

// Checking for border by any of the IR sensors
bool detectBorder() {
  bool border = false;
  reflectanceSensors.read(sensor_values); // gives raw values 0-2000 (pulse times in um)
  if ((sensor_values[0] < 1000) && (sensor_values[4] < 1000)) {
    bool border = true;
    motors.setSpeeds(-(speeD), -(speeD));
    delay(500);
   }
  else if((sensor_values[0] < 1000)) {
    bool border = true;
    motors.setSpeeds((-(speeD)-80),-(speeD));
    delay(20);
  }
  else if((sensor_values[4] < 1000)) {
    bool border = true;
    motors.setSpeeds(-(speeD), -(speeD)-(80));
    delay(20);
  }
  return border;
}

// Main drive logic
void AI() {

  detectBorder();

  if(seek) {
    pingR(sonarRS, &foundRightSide);
    pingR(sonarLS, &foundLeftSide);
    pingR(sonarRear, &foundRear);
    if(turnRight) {
      motors.setSpeeds((searchSpeed), -(searchSpeed));
    }
    if(!turnRight) {
     motors.setSpeeds(-(searchSpeed), searchSpeed);
    }
    pingR(sonarL, &foundLeft);
    if(foundLeft) {
      seek = false;
    }
  }

  if(foundLeft && !foundRight) {
    motors.setSpeeds(-(searchSpeed), searchSpeed);
    pingR(sonarR, &foundRight);
    if(foundRight && !detectBorder()) {
      motors.setSpeeds(speeD, speeD);
    }
  }

  if(!foundLeft && foundRight) {
    motors.setSpeeds(searchSpeed, -(searchSpeed));
    pingR(sonarL, &foundLeft);
    if(foundRight && !detectBorder()) {
      motors.setSpeeds(speeD, speeD);
    }
  }

  allPing();

  if(foundLeftSide && !foundRight && !foundLeft && !foundRightSide && !foundRear) {
    turnRight = false;
    plab_Motors.turnLeft(speeD, 90);
  }

  if(foundRightSide && !foundLeft && !foundLeftSide && !foundRear && !foundRight) {
    turnRight = true;
    plab_Motors.turnRight(speeD, 90);
  }

  if(foundRear && !foundLeftSide && !foundRight && !foundLeft && !foundRightSide) {
    turnRight = false;
    plab_Motors.turnLeft((speeD - 20), 180);
  }

  if(foundLeft && foundRight) {
    pingR(sonarL, &foundLeft);
    pingR(sonarR, &foundRight);
    if(!foundLeft && foundRight) {
      motors.setSpeeds((speeD-50), speeD);
      pingR(sonarL, &foundLeft);
      if(foundLeft && !detectBorder()) {
        motors.setSpeeds(speeD, speeD);
      }
    }
    if(foundLeft && !foundRight) {
      motors.setSpeeds(speeD, (speeD - 50));
      pingR(sonarR, &foundRight);
      if(foundRight && !detectBorder()) {
        motors.setSpeeds(speeD, speeD);
      }
    }
  }

  if(!foundLeft && !foundRight) {
    seek = true;
  }

}
