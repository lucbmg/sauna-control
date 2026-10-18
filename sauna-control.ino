#include <SPI.h>
#include <Adafruit_MAX31865.h>

// --- Configuration Pins Capteur ---
#define MAX_CS   53
#define MAX_SDI  51
#define MAX_SDO  50
#define MAX_CLK  52

// --- Pins Relais (SSR) ---
#define RELAY1       30
#define RELAY2       31
#define LIGHT_RELAY  32

Adafruit_MAX31865 rtd = Adafruit_MAX31865(MAX_CS, MAX_SDI, MAX_SDO, MAX_CLK);

// --- LOGIQUE STANDARD ---
#define RELAY_ON  HIGH  // 5V (Pin) -> SSR ON
#define RELAY_OFF LOW   // 0V (Pin) -> SSR OFF
#define RNOMINAL  100.0
#define RREF      430.0

// --- Variables ---
float currentTemp = 0;
float setTemp = 60;
bool saunaOn = false;
bool lightOn = false;
bool toggleHeater = false;
unsigned long prevMillisHeater = 0;
unsigned long heaterInterval = 4000; 
unsigned long sessionDuration = 0;
unsigned long sessionStart = 0;
unsigned long lastDisplayUpdate = 0;

void setup() {
  Serial.begin(115200);
  Serial1.begin(9600); // Nextion sur 18/19
  
  pinMode(RELAY1, OUTPUT);
  pinMode(RELAY2, OUTPUT);
  pinMode(LIGHT_RELAY, OUTPUT);
  
  // Tout éteint au départ
  digitalWrite(RELAY1, RELAY_OFF);
  digitalWrite(RELAY2, RELAY_OFF);
  digitalWrite(LIGHT_RELAY, RELAY_OFF);
  
  rtd.begin(MAX31865_3WIRE);
  Serial.println("Systeme pret. Logique Standard (Pin 5V = ON)");
}

void loop() {
  currentTemp = rtd.temperature(RNOMINAL, RREF);

  // Lecture Nextion
  if(Serial1.available()) {
    String cmd = Serial1.readStringUntil('\n');
    cmd.trim();
    
    if(cmd == "START") {
      saunaOn = true;
      sessionStart = millis();
      if(sessionDuration == 0) sessionDuration = 30UL * 60000UL;
    }
    else if(cmd == "STOP") {
      saunaOn = false;
    }
    else if(cmd == "LTOGGLE") {
      lightOn = !lightOn;
      digitalWrite(LIGHT_RELAY, lightOn ? RELAY_ON : RELAY_OFF);
    }
    else if(cmd == "TUP") {
      if(setTemp < 95) setTemp++;
    }
    else if(cmd == "TDOWN") {
      if(setTemp > 30) setTemp--;
    }
    else if(cmd.startsWith("TIME")) {
      int mins = cmd.substring(4).toInt();
      sessionDuration = (unsigned long)mins * 60000UL;
      sessionStart = millis();
      saunaOn = true;
    }
  }

  // Fin de session : couper le chauffage quand la minuterie expire
  if (saunaOn && sessionDuration > 0 && (millis() - sessionStart >= sessionDuration)) {
    saunaOn = false;
  }

  // Thermostat
  if(saunaOn && currentTemp < setTemp) {
    if(millis() - prevMillisHeater >= heaterInterval) {
      prevMillisHeater = millis();
      toggleHeater = !toggleHeater;
    }
    digitalWrite(RELAY1, toggleHeater ? RELAY_ON : RELAY_OFF);
    digitalWrite(RELAY2, !toggleHeater ? RELAY_ON : RELAY_OFF);
  } else {
    digitalWrite(RELAY1, RELAY_OFF);
    digitalWrite(RELAY2, RELAY_OFF);
  }

  if (millis() - lastDisplayUpdate >= 500) {
    updateDisplay();
    lastDisplayUpdate = millis();
  }
}

void updateDisplay() {
  int minsLeft = 0;
  if(saunaOn && sessionDuration > 0) {
    unsigned long elapsed = millis() - sessionStart;
    if(elapsed < sessionDuration) {
      minsLeft = (int)((sessionDuration - elapsed) / 60000) + 1;
    }
  }
  Serial1.print("tTemp.txt=\""); Serial1.print(currentTemp, 1); Serial1.print("\""); endNextion();
  Serial1.print("tSet.txt=\""); Serial1.print(setTemp, 0); Serial1.print("\""); endNextion();
  Serial1.print("tTime.txt=\""); Serial1.print(minsLeft); Serial1.print("\""); endNextion();
  Serial1.print("tStatus.txt=\""); Serial1.print(saunaOn ? "ACTIF" : "OFF"); Serial1.print("\""); endNextion();
  Serial1.print("tLight.txt=\""); Serial1.print(lightOn ? "ON" : "OFF"); Serial1.print("\""); endNextion();
}

void endNextion() {
  Serial1.write(0xff); Serial1.write(0xff); Serial1.write(0xff);
}
