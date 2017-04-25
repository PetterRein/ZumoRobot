#include <Pushbutton.h>
#include <PLab_ZumoMotors.h>
#include <ZumoMotors.h>
#include <QTRSensors.h>
#include <ZumoReflectanceSensorArray.h>
#include <NewPing.h>
#include <PLab_ZumoMotors.h>
#include <SoftwareSerial.h>
#include <PLabBTSerial.h>

#define NUM_SENSORS 6
unsigned int sensor_values[NUM_SENSORS];

ZumoReflectanceSensorArray reflectanceSensors; //(QTR_NO_EMITTER_PIN)

// Bluetooth
#define txPin 5  // Tx pin on Bluetooth unit
#define rxPin 3  // Rx pin on Bluetooth unit
#define green 2
#define blinkDelay 200


char BTName[] = "Brukernavn";
char ATCommand[] = "AT+NAMEPLab_";
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
int maxLength = 60; //Setter maks lengde på hva sensorne skal finne
int speeDNear = 250;
int cm = -1;
int turnSpeed = 50;
int limitNear = 8;

bool seek = true; // For å besteme om vi skal søke eller ikke
bool foundLeft = false; //Bool for å vite om vi har funnet noe på venstre fremme sensor
bool foundRight = false; //Bool for å vite om vi har funnet noe på høyre fremme sensor
bool foundRightSide = false; //Bool for å vite om vi har funnet noe på sensor på høyre side
bool foundRear = false; //Bool for å vite pm vi har funnet noe på sensor bak
bool foundLeftSide = false; //Bool for å vite om vi har funnet noe på sensor på venstre side
bool turnRight = false; //Bestemer om vi skal søke til høyre eller ikke
bool cali = false; //Bestemmer om vi skal kalibrere IR-sensorene
bool start1 = false; //Bool for bluetooth for å starte og stoppe roboten
bool slowWhenNear = false;

NewPing sonarL(A1, A1, maxLength); //Sensor fremme venstre
NewPing sonarR(A4, A4, maxLength); //Sensor fremme høyre
NewPing sonarRS(A0, A0, maxLength); //Sensor høyre side (RightSide)
NewPing sonarRear(A5, A5, maxLength); //Sensor bak
NewPing sonarLS(6, 6, maxLength); //Sensor venstre side (LeftSide)

void setup() {
  pinMode(green, OUTPUT);
  digitalWrite(green, HIGH);
  reflectanceSensors.init();
  Serial.begin(9600);
  // For løkken kalibrer IR sensorne
  motors.setSpeeds(0, 0);
  btSerial.begin(9600); // Open serial communication to Bluetooth unit
  button.waitForButton();
}

void blinkG() {
  digitalWrite(green, HIGH);
  delay(blinkDelay);
  digitalWrite(green, LOW);
  delay(blinkDelay);
  digitalWrite(green, HIGH);
  delay(blinkDelay);
  digitalWrite(green, LOW);
  
}

void blinkF() {
  digitalWrite(green, HIGH);
  delay(blinkDelay);
  digitalWrite(green, LOW);
  delay(blinkDelay);
  
}

void calibration(){
  for(int i = 0; i < 80; i++) {
    if ((i > 10 && i <= 30) || (i > 50 && i <= 70)) {
      motors.setSpeeds(-(speeD), speeD);
    }
    else {
      motors.setSpeeds(speeD, -(speeD));
    }
    reflectanceSensors.calibrate();
    delay(10);
  }
  cali = false;
}

// Main loop
void loop() {
  bluetooth();
  if(cali){
    calibration();
  }
  if(start1){
   AI(); 
  }
  else if(!start1){
    motors.setSpeeds(0,0);
	  seek = true;
  }
  /**AI();**/
}

// Bluetooth functionality

void bluetooth() {
  String array5 [10] = {};
  String string = "";
  int i = 0;
  while (btSerial.available()) { // If input available from Bluetooth unit
   char c = btSerial.read();    // Read character from from Bluetooth unit
   if(c != ' '){
    string += c;
    array5[i] = string;
   }
   else{
    string = "";
    i++;
   }
  }
  if(array5[0].length() > 0){
   Serial.println(array5[0]); 
  }
	if(array5[0] == "G"){
    Serial.println("Heivikom inn");
    if(!start1){
      start1 = true;
    }
    else if(start1){
      start1 = false;
      blinkF();
    }
 }
 else if(array5[0] == "F"){
  start1 = false;
 }
 else if(array5[0] == "speeD" || array5[0] == "speeDNear" || array5[0] == "searchSpeed" || array5[0] == "slowWhenNear" || array5[0] == "cali" || array5[0] == "turnSpeed" || array5[0] == "limitNear"){
  int nr = 0;
  int gange = 1;
  for(int i = 0; i < array5[1].length(); i++){
    if(i > 0){
      gange = 10;
    }
    if(array5[1][i] == '1'){
      nr = nr*gange;
      nr += 1;
    }
    else if(array5[1][i] == '2'){
      nr = nr*gange;
      nr += 2;
    }
    else if(array5[1][i] == '3'){
      nr = nr*gange;
      nr += 3;
    }
    else if(array5[1][i] == '4'){
      nr = nr*gange;
      nr += 4;
    }
    else if(array5[1][i] == '5'){
      nr = nr*gange;
      nr += 5;
    }
    else if(array5[1][i] == '6'){
      nr = nr*gange;
      nr += 6;
    }
    else if(array5[1][i] == '7'){
      nr = nr*gange;
      nr += 7;
    }
    else if(array5[1][i] == '8'){
      nr = nr*gange;
      nr += 8;
    }
    else if(array5[1][i] == '9'){
      nr = nr*gange;
      nr += 9;
    }
    else if(array5[1][i] == '0'){
      nr = nr*gange;
      nr += 0;
    }
  }
  if(array5[0] == "slowWhenNear"){
    if(nr == 1){
      slowWhenNear = true;
      blinkG();
    }
    else if(nr == 0){
      slowWhenNear = false;
      blinkF();
    }
  }
  else if(array5[0] == "searchSpeed"){
    searchSpeed = nr;
    blinkG();
  }
  else if(array5[0] == "speeD"){
    speeD = nr;
    blinkG();
  }
  else if(array5[0] == "cali"){
    if(nr == 1){
      cali = true;
      blinkG();
    }
    else if(nr == 0){
      cali = false;
      blinkF();
    }
  }
  else if(array5[0] == "speeDNear"){
    speeDNear = nr;
    blinkG();
  }
  else if(array5[0] == "turnSpeed"){
    turnSpeed = nr;
    blinkG();
  }
  else if(array5[0] == "limitNear"){
    limitNear = nr;
    blinkG();
  }
  
 }
 else if(array5[0] == "Vari"){
  btSerial.println("Test");
 }
	/** Kan kaste string til int for å sette variabler direkte (string.toInt();)**/
}

int ping1(NewPing sonar, bool *found) {
  float cm = sonar.convert_cm(sonar.ping());
  if(cm > 1 && cm < maxLength) {
    *found = true;
  }
  else{
    *found = false;
  }
  return cm;
}

void allPing() {
  ping1(sonarL, &foundLeft);
  cm = ping1(sonarR, &foundRight);
  ping1(sonarRS, &foundRightSide);
  ping1(sonarRear, &foundRear);
  ping1(sonarLS, &foundLeftSide);
}

// Checking for border by any of the IR sensors
bool detectBorder() {
 bool border = false;
  reflectanceSensors.read(sensor_values); // gives raw values 0-2000 (pulse times in um)
  Serial.println("Her ");
  Serial.println(sensor_values[0]);
  Serial.println(sensor_values[4]);
  
  if ((sensor_values[0] < 1000) && (sensor_values[4] < 1000)) {
    border = true;
    motors.setSpeeds(-(speeD), -(speeD));
    delay(500);
   }
  else if((sensor_values[0] < 1000)) {
    border = true;
    motors.setSpeeds(-(speeD-100),-(speeD));
    delay(50);
  }
  else if((sensor_values[4] < 1000)) {
    border = true;
    motors.setSpeeds(-(speeD), -(speeD-100));
    delay(50);
  }
  return border;
}

// Main drive logic
void AI() {
  detectBorder();
  if(seek) {
    if(turnRight) {
      motors.setSpeeds((searchSpeed), -(searchSpeed));
    }
    if(!turnRight) {
     motors.setSpeeds(-(searchSpeed), searchSpeed);
    }
  }
  allPing();
  if(foundLeft && !foundRight && !detectBorder()) {
    seek = false;
    motors.setSpeeds((speeD - turnSpeed), speeD);
    cm = ping1(sonarR, &foundRight);
    if(foundRight && !detectBorder()) {
      if(slowWhenNear){
        if(cm > limitNear){
          motors.setSpeeds(speeD, speeD);    
        }
        else if(cm < limitNear){
          motors.setSpeeds(speeDNear, speeDNear);
        }
      }
      if(!slowWhenNear){
        motors.setSpeeds(speeD, speeD);
      }
    }
  }
  if(!foundLeft && foundRight && !detectBorder()) {
	seek = false;
    motors.setSpeeds(speeD, (speeD - turnSpeed));
    ping1(sonarL, &foundLeft);
    if(foundRight && !detectBorder()) {
      if(slowWhenNear){
        if(cm > limitNear){
          motors.setSpeeds(speeD, speeD);    
        }
        else if(cm < limitNear){
          motors.setSpeeds(speeDNear, speeDNear);
        }
      }
      if(!slowWhenNear){
        motors.setSpeeds(speeD, speeD);
      }
    }
  }
  if(foundLeftSide && !foundRight && !foundLeft && !foundRightSide && !foundRear) {
	/**seek = false;**/
    turnRight = false;
    plab_Motors.turnLeft(speeD, 80);
  }

  if(foundRightSide && !foundLeft && !foundLeftSide && !foundRear && !foundRight) {
    /**seek = false;**/
	turnRight = true;
    plab_Motors.turnRight(speeD, 80);
  }
  if(foundRear && !foundLeftSide && !foundRight && !foundLeft && !foundRightSide) {
    /**seek = false;**/
	if(turnRight){
     plab_Motors.turnRight((speeD), 170);
    }
    else{
      plab_Motors.turnLeft((speeD), 170);
    }
  }
  if(foundLeft && foundRight && !detectBorder()) {
	  seek = false;
    cm = ping1(sonarL, &foundLeft);
    cm = ping1(sonarR, &foundRight);
    if(!foundLeft && foundRight && !detectBorder()) {
      motors.setSpeeds((speeD-turnSpeed), speeD);
      cm = ping1(sonarL, &foundLeft);
      if(foundLeft && !detectBorder()) {
        if(slowWhenNear){
          if(cm > limitNear){
            motors.setSpeeds(speeD, speeD);    
           }
        else if(cm < limitNear){
          motors.setSpeeds(speeDNear, speeDNear);
        }
      }
      if(!slowWhenNear){
        motors.setSpeeds(speeD, speeD);
      }
      }
    }
    if(foundLeft && !foundRight && !detectBorder()) {
      motors.setSpeeds(speeD, (speeD - turnSpeed));
      cm = ping1(sonarR, &foundRight);
      if(foundRight && !detectBorder()) {
        if(slowWhenNear){
          if(cm > limitNear){
            motors.setSpeeds(speeD, speeD);    
          }
        else if(cm < limitNear){
          motors.setSpeeds(speeDNear, speeDNear);
        }
      }
      if(!slowWhenNear){
        motors.setSpeeds(speeD, speeD);
      }
      }
    }
	  if(foundLeft && foundRight && !detectBorder()){
	    if(slowWhenNear){
        if(cm > limitNear){
          motors.setSpeeds(speeD, speeD);    
        }
        else if(cm < limitNear){
          motors.setSpeeds(speeDNear, speeDNear);
        }
      }
      if(!slowWhenNear){
        motors.setSpeeds(speeD, speeD);
      }
	  }
  }
  if(!foundLeft && !foundRight && !detectBorder()) {
    seek = true;
  }
}
