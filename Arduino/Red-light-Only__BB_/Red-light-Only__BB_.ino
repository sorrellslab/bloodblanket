/*
  Synchronizes a temperature reading with LED output for the blood blanket setup
  This experiment detects a heat decrease to denote the start of a trial then
  detects a heat increase that triggers a light stimulus. 
  Times are in milliseconds
*/

#include <Adafruit_MAX31856.h>
Adafruit_MAX31856 maxthermo = Adafruit_MAX31856(10, 11, 12, 13);

//constants
const int synchPin =  3; // synchronization LED
const int redPin = 5; // red LED
const long Accommodation = 600000; // length of incubation period to avoid detecting a temperature stimulus
const long LEDduration = 5000; // LED duration
const long interval = 60000; // Intervals between LED lights
const int LEDTimes = 10; // LED stimulus count
const long TempReadInterval = 100; // time between temperature readings
const double thresholdTemp = 24.5; //threshold for triggering the start of a stimulus

void setup() {
  // Set up the output pins:
  pinMode(synchPin, OUTPUT);
  pinMode(redPin, OUTPUT);
  // Test light function:
  digitalWrite(redPin, LOW);
  digitalWrite(synchPin, HIGH);
  delay(5000);
  digitalWrite(synchPin, LOW);
  digitalWrite(redPin, LOW);
  // Serial monitor:
  Serial.begin(115200);
  // Set up the thermometer:
  maxthermo.begin();
  maxthermo.setThermocoupleType(MAX31856_TCTYPE_T);
  maxthermo.Config(); //added line after editing the library to remove delay aspect
}

// variables
long BeginTime = 0;  // start of the current LED stimulus
int RedTimes = 0; // current number of LED stimuli
long incubation = 0; // current duration of the acclimation period
int Dip = 0; // number of temperature dips detected

void loop() {
  // read the clock
  unsigned long currentTime = millis();
  // read the thermometer
  float temperatureC = maxthermo.readThermocoupleTemperature();
  if (Dip == 0){
    // don't activate the red light before detecting the dip in temperature starting a trial
    digitalWrite(redPin, LOW);
    Serial.print(millis()); Serial.print(' '); Serial.print(temperatureC); Serial.print(" degrees C "); Serial.print(" Program hasn't triggered - Red light OFF ");Serial.println(Dip);
    // detect a heat dip
    if (temperatureC < thresholdTemp){
      Dip += 1;
      Serial.print(millis()); Serial.print(' '); Serial.print(temperatureC); Serial.print(" degrees C "); Serial.println(" Dip detected - Red light OFF");
      BeginTime = currentTime;
    } 
  }
  else if (Dip == 1) {
    // if a temperature increase is detected during the incubation period, don't turn on the red light
    if (incubation < Accommodation){
      digitalWrite(redPin, LOW);
      Serial.print(millis()); Serial.print(' '); Serial.print(temperatureC); Serial.print(" degrees C "); Serial.print(" During incubation - Red light OFF  Incubation time:");Serial.println(incubation);
      incubation = currentTime - BeginTime;
    }
    else {
      // if within an LED stimulus, turn the red light on
      if (currentTime - BeginTime < LEDduration && RedTimes <= LEDTimes ){
        digitalWrite(redPin, HIGH);
        digitalWrite(synchPin, HIGH);
        Serial.print(millis()); Serial.print(' '); Serial.print(temperatureC); Serial.print(" degrees C "); Serial.print("Stimulus - Red light ON  Stimulus time:");Serial.println(currentTime - BeginTime);
      }
      // if stimulus has ended, turn LED off
      else if (currentTime - BeginTime >= LEDduration && currentTime - BeginTime < LEDduration + interval && RedTimes <= LEDTimes ) {
        digitalWrite(redPin, LOW);
        digitalWrite(synchPin, LOW);
        Serial.print(millis()); Serial.print(' '); Serial.print(temperatureC); Serial.print(" degrees C "); Serial.print("Stimulus interval - Red light OFF");Serial.println(currentTime - BeginTime);
      }
      // if LED stimulus has begun, increment stimulus count and stimulus start time
      else if (currentTime - BeginTime >= LEDduration + interval && RedTimes <= LEDTimes ){
        RedTimes += 1;
        BeginTime = currentTime;
      }
      else {
        Serial.print(millis()); Serial.print(' '); Serial.print(temperatureC); Serial.print(" degrees C "); Serial.println("Finish stimulus - Red light OFF");
      }

    }
  }
}
