#include <Pushbutton.h>
#include <PLab_ZumoMotors.h>
#include <ZumoMotors.h>
#include <QTRSensors.h>
#include <ZumoReflectanceSensorArray.h>
#include <NewPing.h>
#include <PLab_ZumoMotors.h>
#include <SoftwareSerial.h>
#include <PLabBTSerial.h>


#define QTR_THRESHOLD  1500
#define NUM_SENSORS 6
unsigned int sensor_values[NUM_SENSORS];

ZumoReflectanceSensorArray reflectanceSensors; //(QTR_NO_EMITTER_PIN)

// Bluetooth
#define txPin 4  // Tx pin on Bluetooth unit
#define rxPin 3  // Rx pin on Bluetooth unit
#define redPin 8
#define greenPin 9

char BTName[] = "Brukernavn";
char ATCommand[] = "AT+NAMEPLab_";
char c = 'x';
PLabBTSerial btSerial(txPin, rxPin);


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

int speeD = 385; //Farten som roboten bruker når den har funnet noe
int searchSpeed = 300; //Farten som roboten bruker når den søker
int maxLength = 30; //Setter maks lengde på hva sensorne skal finne

bool seek = true; // For å besteme om vi skal søke eller ikke
bool foundLeft = false; //Bool for å vite om vi har funnet noe på venstre fremme sensor
bool foundRight = false; //Bool for å vite om vi har funnet noe på høyre fremme sensor
bool foundRightSide = false; //Bool for å vite om vi har funnet noe på sensor på høyre side
bool foundRear = false; //Bool for å vite pm vi har funnet noe på sensor bak
bool foundLeftSide = false; //Bool for å vite om vi har funnet noe på sensor på venstre side
bool turnRight = false; //Bestemer om vi skal søke til høyre eller ikke
bool cali = false; //Bestemmer om vi skal kalibrere IR-sensorene

NewPing sonarL(5, 5, 35); //Sensor fremme venstre
NewPing sonarR(6, 6, 35); //Sensor fremme høyre
NewPing sonarRS(A4, A4, 35); //Sensor høyre side (RightSide)
NewPing sonarRear(A5, A5, 35); //Sensor bak
NewPing sonarLS(A1, A1, 35); //Sensor venstre side (LeftSide)

void setup() {
  reflectanceSensors.init();
  Serial.begin(9600);
  button.waitForButton();

  // For løkken kalibrer IR sensorne
  if(cali){
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
  }
  motors.setSpeeds(0, 0);
  btSerial.begin(9600); // Open serial communication to Bluetooth unit
  button.waitForButton();
}

// Main loop
void loop() {
  bluetooth();
  ping(sonarL, &foundLeft);
  ping(sonarR, &foundRight);
  AI();
}

// Bluetooth functionality

void bluetooth() {
  while (btSerial.available()) { // If input available from Bluetooth unit
    c = btSerial.read();    // Read character from from Bluetooth unit
    Serial.write(c);             // Write that character to Serial Monitor
  };
  while (Serial.available()) { // If input available from Serial Monitor
    char c = Serial.read();    // Read character from from Serial Monitor
    btSerial.write(c);
  };
}

void ping(NewPing sonar, bool *found) {
  float cm = sonar.convert_cm(sonar.ping());
  if(cm > 1 && cm < 30) {
    *found = true;
  }
  else{
    *found = false;
  }
}

void allPing() {
  ping(sonarL, &foundLeft);
  ping(sonarR, &foundRight);
  ping(sonarRS, &foundRightSide);
  ping(sonarRear, &foundRear);
  ping(sonarLS, &foundLeftSide);
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
    motors.setSpeeds((-(speeD)-100),-(speeD));
    delay(25);
  }
  else if((sensor_values[4] < 1000)) {
    bool border = true;
    motors.setSpeeds(-(speeD), -(speeD)-(100));
    delay(25);
  }
  return border;
}

// Main drive logic
void AI() {
  detectBorder();
  if(seek) {
    ping(sonarRS, &foundRightSide);
    ping(sonarLS, &foundLeftSide);
    ping(sonarRear, &foundRear);
    if(turnRight) {
      motors.setSpeeds((searchSpeed), -(searchSpeed));
    }
    if(!turnRight) {
     motors.setSpeeds(-(searchSpeed), searchSpeed);
    }
    ping(sonarL, &foundLeft);
    if(foundLeft) {
      seek = false;
    }
  }
  allPing();
  if(foundLeft && !foundRight && !detectBorder()) {
    motors.setSpeeds((speeD - 50), speeD);
    ping(sonarR, &foundRight);
    if(foundRight && !detectBorder()) {
      motors.setSpeeds(speeD, speeD);
    }
  }
  allPing();
  if(!foundLeft && foundRight && !detectBorder()) {
    motors.setSpeeds(speeD, (speeD - 50));
    ping(sonarL, &foundLeft);
    if(foundRight && !detectBorder()) {
      motors.setSpeeds(speeD, speeD);
    }
  }
  allPing();
  if(foundLeftSide && !foundRight && !foundLeft && !foundRightSide && !foundRear) {
    turnRight = false;
    plab_Motors.turnLeft(speeD, 80);
  }

  if(foundRightSide && !foundLeft && !foundLeftSide && !foundRear && !foundRight) {
    turnRight = true;
    plab_Motors.turnRight(speeD, 80);
  }
  if(foundRear && !foundLeftSide && !foundRight && !foundLeft && !foundRightSide) {
    if(turnRight){
     plab_Motors.turnRight((speeD), 170);
    }
    else{
      plab_Motors.turnLeft((speeD), 170);
    }
  }
  allPing();
  if(foundLeft && foundRight && !detectBorder()) {
    ping(sonarL, &foundLeft);
    ping(sonarR, &foundRight);
    if(!foundLeft && foundRight) {
      motors.setSpeeds((speeD-50), speeD);
      ping(sonarL, &foundLeft);
      if(foundLeft && !detectBorder()) {
        motors.setSpeeds(speeD, speeD);
      }
    }
    if(foundLeft && !foundRight) {
      motors.setSpeeds(speeD, (speeD - 50));
      ping(sonarR, &foundRight);
      if(foundRight && !detectBorder()) {
        motors.setSpeeds(speeD, speeD);
      }
    }
  }
  allPing();
  if(!foundLeft && !foundRight && !detectBorder()) {
    seek = true;
  }
}
