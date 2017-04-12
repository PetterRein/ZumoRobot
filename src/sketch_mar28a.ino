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
 
ZumoReflectanceSensorArray reflectanceSensors; //(QTR_NO_EMITTER_PIN);


//ECHO Hvit ledning
//TRIG Blå ledning
//5V Rød ledning
//Ground Svart ledning



PLab_ZumoMotors PLab_motors;
Pushbutton button(ZUMO_BUTTON);
ZumoMotors motors;
PLab_ZumoMotors plab_Motors;

int speeD = 350; //Farten som roboten bruker når den har funnet noe
int searchSpeed = 250; //Farten som roboten bruker når den søker
int maxLength = 30; //Setter maks lengde på hva sensorne skal finne
bool seek = true; //Bool for å besteme om vi skal søke eller ikke
bool foundLeft = false; //Bool for å vite om vi har funnet noe på venstre fremme sensor
bool foundRight = false; //Bool for å vite om vi har funnet noe på høyre fremme sensor
bool foundRightSide = false; //Bool for å vite om vi har funnet noe på sensor på høyre side
bool foundLeftSide = false; //Bool for å vite om vi har funnet noe på sensor på venstre side
bool foundRear = false; //bool for å vite pm vi har funnet noe på sensor bak
bool ir1 = true; //Bool for å vite om vi er over kanten eller ikke
bool turnRight = false; //Bestemer om vi skal søke til høyre eller ikke

long cmR, cmL; //Brukes for å lagre avstaden sensorene oppdager
long durationR; //Brukes for å lagre tiden det tar før sensorene får måling
long durationL; //Brukes for å lagre tiden det tar før sensorene får måling


NewPing sonarLS(A1, A1, 35); //Setter pin
NewPing sonarRS(A4, A4, 35);
NewPing sonarRear(A5, A5, 35);
NewPing sonarR(6, 6, 35);
NewPing sonarL(5, 5, 35);

void setup() {
  // For løkken kalibrer IR sensorne
  reflectanceSensors.init();
  Serial.begin(9600);
  button.waitForButton();
  int i;
  for(i = 0; i < 80; i++)
  {
    if ((i > 10 && i <= 30) || (i > 50 && i <= 70))
      motors.setSpeeds(-(speeD), speeD);
    else
      motors.setSpeeds(speeD, -(speeD));
      
    reflectanceSensors.calibrate();

    // Since our counter runs to 80, the total delay will be
    // 80*20 = 1600 ms.
    delay(2);
  }
  motors.setSpeeds(0,0);
  button.waitForButton();
}

void loop() {
  // put your main code here, to run repeatedly:
  pingR(sonarR, &foundRight);
  pingR(sonarL, &foundLeft);
  AI();
}

void pingR(NewPing sonar, bool *found){
  unsigned int time = sonar.ping();
  float cmLeftS = sonar.convert_cm(time);
  if(cmLeftS > 1 && cmLeftS < 30){
    *found = true;
  }
  else{
    *found = false;
  }
}

void allPing(){
  pingR(sonarR, &foundRight);
  pingR(sonarL, &foundLeft);
  pingR(sonarRS, &foundRightSide);
  pingR(sonarLS, &foundLeftSide);
  pingR(sonarRear, &foundRear);
}


bool ir(){
  bool kant = false;
   reflectanceSensors.read(sensor_values); // gives raw values 0-2000 (pulse times in um)
   //Check if border has been detected by any of the two sensors at each side
   //Serial.print(sensor_values[0]);
   //Serial.print(sensor_values[4]);
    if ((sensor_values[0] < 1000) && (sensor_values[4] < 1000)){
      kant = true;
      motors.setSpeeds(-(speeD), -(speeD));
      delay(500);
   }
   else if((sensor_values[0] < 1000)){
    kant = true;
    motors.setSpeeds((-(speeD)-80),-(speeD));
    delay(20);
   }
   else if((sensor_values[4] < 1000)){
    kant = true;
    motors.setSpeeds(-(speeD),-(speeD)-(80));
    delay(20);
   }
   return kant;
}

void AI(){
  if(ir1){
   ir();
 }
  if(seek){
    pingR(sonarRS, &foundRightSide);
    pingR(sonarLS, &foundLeftSide);
    pingR(sonarRear, &foundRear);
    if(turnRight){
      motors.setSpeeds((searchSpeed), -(searchSpeed));
    }
    if(!turnRight){
     motors.setSpeeds(-(searchSpeed), searchSpeed); 
    }
    pingR(sonarL, &foundLeft);
    if(foundLeft){
      seek = false;
    }
  }
  
  if(foundLeft && !foundRight){
    motors.setSpeeds(-(searchSpeed), searchSpeed);
    pingR(sonarR, &foundRight);
    if(foundRight && !ir()){
      motors.setSpeeds(speeD, speeD);
    }
  }

  if(!foundLeft && foundRight){
    motors.setSpeeds(searchSpeed, -(searchSpeed));
    pingR(sonarL, &foundLeft);
    if(foundRight && !ir()){
      motors.setSpeeds(speeD, speeD);
    }
  }
  allPing();
  if(foundLeftSide && !foundRight && !foundLeft && !foundRightSide && !foundRear){
    turnRight = false;
    plab_Motors.turnLeft(speeD, 90);
  }

  if(foundRightSide && !foundLeft && !foundLeftSide && !foundRear && !foundRight){
    turnRight = true;
    plab_Motors.turnRight(speeD, 90);
  }

  if(foundRear && !foundLeftSide && !foundRight && !foundLeft && !foundRightSide){
    turnRight = false;
    plab_Motors.turnLeft((speeD - 20), 180);
  }
  
  if(foundLeft && foundRight){
    pingR(sonarL, &foundLeft);
    pingR(sonarR, &foundRight);
    if(!foundLeft && foundRight){
      motors.setSpeeds((speeD-50), speeD);
      pingR(sonarL, &foundLeft);
      if(foundLeft && !ir()){
        motors.setSpeeds(speeD, speeD);
      }
    }
    if(foundLeft && !foundRight){
      motors.setSpeeds(speeD, (speeD - 50));
      pingR(sonarR, &foundRight);
      if(foundRight && !ir()){
        motors.setSpeeds(speeD, speeD);
      }
    }
  }
  if(!foundLeft && !foundRight){
    seek = true;
  }

  /**
  pingLeftSide();
  if(pingLeftSide){
    turn 90 left
  pingRightSide();
  if(pingRightSide){
    turn 90 right
  pingbehinde();
  if(pingLbehinde){
    turn 180

  If speed is descrsing when it should be incrinsing
   turn motors speed down a bit

  
   **/

}



