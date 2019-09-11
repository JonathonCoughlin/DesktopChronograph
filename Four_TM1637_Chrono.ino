#include <TM1637Display.h>

const int clk1 = 2;
const int data1 = 3;
const int clk2 = 4;
const int data2 = 5;
const int clk3 = 6;
const int data3 = 7;
const int clk4 = 8;
const int data4 = 9;

const int potSpeedPin = A5;

TM1637Display displayWorldClock(clk1, data1);// define dispaly 1 object
TM1637Display displayStopwatch(clk2, data2);// define dispaly 1 object
TM1637Display displayAirspeed(clk3, data3);// define dispaly 1 object
TM1637Display displayDistance(clk4, data4);// define dispaly 1 object

int valueWorldClock,
    digit1WorldClock,
    digit2WorldClock,
    digit3WorldClock,
    digit4WorldClock; 

int valueStopwatch,
    digit1Stopwatch,
    digit2Stopwatch,
    digit3Stopwatch,
    digit4Stopwatch;

int lastStopwatchValue;
int valueAirspeed;
int valueDistance;

const int resetButtonPin = A0;
const int pauseButtonPin = A1;

unsigned long arduinoTime;
unsigned long startTimeStopwatch;
unsigned long watchTime;
unsigned long pauseTimeStopwatch;

bool paused;

unsigned long nextPotSurveyTime;

String debugString = "debu";

int potBuffer = 10;
int potFloor, potCeiling;

void setup(){
  pinMode(resetButtonPin,INPUT_PULLUP);
  pinMode(pauseButtonPin,INPUT_PULLUP);
  pinMode(potSpeedPin, INPUT);
  
  displayWorldClock.setBrightness(0x0a);
  displayStopwatch.setBrightness(0x0a);
  displayAirspeed.setBrightness(0x0a);
  displayDistance.setBrightness(0x0a);
//  Serial.begin(9600);
  arduinoTime = millis();
  startTimeStopwatch = arduinoTime;
  pauseTimeStopwatch = arduinoTime;
  paused = true;
  
//  Serial.println(debugString);

  displayWorldClock.showNumberDecEx(0, 0b01000000, true, 4, 0); 
  displayStopwatch.showNumberDecEx(0, 0b01000000, true, 4, 0);
  displayDistance.showNumberDecEx(0, 0b00000000, false, 4, 0);
  displayAirspeed.showNumberDecEx(0, 0b00000000, false, 4, 0);

  potFloor = 0 + potBuffer;
  potCeiling = 1023 - (potBuffer * 2);
}

int potValue;
int lastPotValue;
unsigned long potSurveyDelay = 100;

int airspeedRaw;
int minAirspeed = 100;
int maxAirspeed = 650;
int lastAirspeed;
float millisPerHour = 3600000; 
float watchTimeHours;
float trueDistance;
int distanceForDisplay;
int lastDistanceForDisplay;
bool pauseButtonDebounceCleared = true;
unsigned long debounceCheckTime;
unsigned long debounceCheckDelay = 30L;
unsigned long pauseDeltaTime;

int potHist0;
int potHist1;
int potHist2;
int potHist3;
int potHist4;
int potAverage;

int airspeedDigitHundreds, airspeedDigitTens;

void loop() 
{
  arduinoTime = millis();

  if (arduinoTime > nextPotSurveyTime)
  {
    potValue = analogRead(potSpeedPin);

    potHist4 = potHist3;
    potHist3 = potHist2;
    potHist2 = potHist1;
    potHist1 = potHist0;
    potHist0 = potValue;
    potAverage = (potHist4 + potHist3 + potHist2 + potHist1 + potHist0) / 5;

    airspeedRaw = constrain(map(potAverage, potFloor, potCeiling, maxAirspeed, minAirspeed), minAirspeed, maxAirspeed);
    airspeedDigitHundreds = airspeedRaw / 100;
    airspeedDigitTens = (airspeedRaw % 100) / 10;
    valueAirspeed = airspeedDigitHundreds * 100 + airspeedDigitTens * 10;
    
    nextPotSurveyTime = arduinoTime + potSurveyDelay;
  }
  
  if (digitalRead(resetButtonPin) == LOW)
  {
    startTimeStopwatch = arduinoTime;
    if (paused) 
    {
      pauseTimeStopwatch = startTimeStopwatch;
      watchTime = 0;
    }
  }

  if (pauseButtonDebounceCleared)
  {
    if (digitalRead(pauseButtonPin) == LOW) 
    {
//      Serial.println("Pause Button Pressed");
      pauseButtonDebounceCleared = false;
      debounceCheckTime = arduinoTime + debounceCheckDelay;
      if (paused)
      {
        paused = false;
        pauseDeltaTime = arduinoTime - pauseTimeStopwatch;
        startTimeStopwatch += pauseDeltaTime;
      }
      else 
      {
        pauseTimeStopwatch = arduinoTime;
        paused = true;
      }
    }
  }
  else if (arduinoTime > debounceCheckTime && digitalRead(pauseButtonPin) == HIGH) 
  {
    pauseButtonDebounceCleared = true;
  }

  if (!paused)
  {
    watchTime = arduinoTime - startTimeStopwatch;  
  }

  int watchSeconds = (int)(watchTime / 1000L);
  int watchMinutes = watchSeconds / 60;

  digit1Stopwatch = watchMinutes / 10;
  digit2Stopwatch = watchMinutes % 10;
  int clockSeconds = watchSeconds % 60;
  digit3Stopwatch = clockSeconds / 10;
  digit4Stopwatch = clockSeconds % 10;
//  debugString = String(digit1Stopwatch) +
//                String(digit2Stopwatch) +
//                String(digit3Stopwatch) +
//                String(digit4Stopwatch);

  valueStopwatch = digit1Stopwatch * 1000 + 
          digit2Stopwatch * 100 + 
          digit3Stopwatch * 10 + 
          digit4Stopwatch;
          
  if (lastStopwatchValue != valueStopwatch) 
  {
    displayStopwatch.showNumberDecEx(valueStopwatch, 0b01000000, true, 4, 0);
    lastStopwatchValue = valueStopwatch;
//    Serial.println(debugString);
    displayWorldClock.showNumberDecEx(valueStopwatch, 0b01000000, true, 4, 0);   
  }

  watchTimeHours = (float)watchTime / millisPerHour;
  trueDistance = (float)valueAirspeed * watchTimeHours;
  distanceForDisplay = (int)trueDistance;
  if (distanceForDisplay != lastDistanceForDisplay) 
  {
    displayDistance.showNumberDecEx(distanceForDisplay, 0b00000000, false, 4, 0);        
  }
  lastDistanceForDisplay = distanceForDisplay;

  if (valueAirspeed != lastAirspeed) 
  {
    displayAirspeed.showNumberDecEx(valueAirspeed, 0b00000000, false, 4, 0);
  }
  lastAirspeed = valueAirspeed;
}
