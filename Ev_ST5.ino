#include <WiFi.h>
#include <FirebaseESP32.h>

/* ---------- WIFI ---------- */
#define WIFI_SSID "Bahadur galaxy"
#define WIFI_PASSWORD "wifi4656"

/* ---------- FIREBASE ---------- */
#define FIREBASE_HOST "batterylocker-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "WQVa5d1RocBxYb6aqxvVfQWsriQ6xOYnEBu0L9S8"

/* ---------- LED ---------- */
#define WIFI_LED 2        // Green LED (WiFi)
#define STATUS_LED 4      // Red LED (Charging)

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

/* ---------- RELAYS ---------- */
int doorRelay[6]    = {27,26,25,14,13,16};
int chargerRelay[6] = {17,18,19,21,22,23};

/* ---------- VARIABLES ---------- */
unsigned long chargeStart[6];
bool charging[6];

bool doorPulseActive[6];
unsigned long doorPulseStart[6];

/* WiFi LED blink control */
unsigned long wifiBlinkTimer = 0;
bool wifiLedState = false;

/* WiFi retry timer */
unsigned long wifiRetryTimer = 0;

#define CHARGE_TIME 12600000
#define DOOR_PULSE_TIME 500

/* ---------- PRINT ---------- */
void printSlotStatus(int i, String msg){
  Serial.print("Slot ");
  Serial.print(i+1);
  Serial.print(" : ");
  Serial.println(msg);
}

/* ---------- WIFI CHECK ---------- */
void checkWiFi(){

  if(WiFi.status() == WL_CONNECTED){
    digitalWrite(WIFI_LED, HIGH);
    return;
  }

  // Blink LED while disconnected
  if(millis() - wifiBlinkTimer > 500){
    wifiBlinkTimer = millis();
    wifiLedState = !wifiLedState;
    digitalWrite(WIFI_LED, wifiLedState);
  }

  // Retry connection every 5 sec
  if(millis() - wifiRetryTimer > 5000){
    wifiRetryTimer = millis();
    Serial.println("Reconnecting WiFi...");
    WiFi.disconnect();
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  }
}

/* ---------- SETUP ---------- */
void setup(){

  Serial.begin(115200);

  pinMode(WIFI_LED, OUTPUT);
  pinMode(STATUS_LED, OUTPUT);

  for(int i=0;i<6;i++){
    pinMode(doorRelay[i], OUTPUT);
    pinMode(chargerRelay[i], OUTPUT);

    digitalWrite(doorRelay[i], LOW);
    digitalWrite(chargerRelay[i], LOW);

    charging[i] = false;
    chargeStart[i] = 0;
    doorPulseActive[i] = false;
    doorPulseStart[i] = 0;
  }

  Serial.print("Connecting WiFi");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  // Blink LED while connecting
  while(WiFi.status()!=WL_CONNECTED){

    if(millis() - wifiBlinkTimer > 500){
      wifiBlinkTimer = millis();
      wifiLedState = !wifiLedState;
      digitalWrite(WIFI_LED, wifiLedState);
    }

    delay(100);
    Serial.print(".");
  }

  digitalWrite(WIFI_LED, HIGH);
  Serial.println("\nWiFi Connected");

  config.host = FIREBASE_HOST;
  config.signer.tokens.legacy_token = FIREBASE_AUTH;

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  restoreChargingState();

  Serial.println("System Ready");
}

/* ---------- LOOP ---------- */
void loop(){
  checkWiFi();
  checkCommands();
  checkChargingTimeout();
  handleDoorPulse();
  updateChargingLED();
}

/* ---------- RESTORE CHARGING ---------- */
void restoreChargingState(){
  for(int i=0;i<6;i++){
    String path="/locker/slot"+String(i+1)+"/charging";

    if(Firebase.getInt(fbdo,path)){
      if(fbdo.intData()==1){
        charging[i]=true;
        chargeStart[i]=millis();
        digitalWrite(chargerRelay[i],HIGH);
        printSlotStatus(i,"Charging RESTORED");
      }
    }
  }
}

/* ---------- COMMAND CHECK ---------- */
void checkCommands(){

  for(int i=0;i<6;i++){

    String base="/locker/slot"+String(i+1);

    if(Firebase.getInt(fbdo,base+"/startCharge")){
      if(fbdo.intData()==1){
        startCharging(i);
        Firebase.setInt(fbdo,base+"/startCharge",0);
      }
    }

    if(Firebase.getInt(fbdo,base+"/stopCharge")){
      if(fbdo.intData()==1){
        stopCharging(i);
        Firebase.setInt(fbdo,base+"/stopCharge",0);
      }
    }

    if(Firebase.getInt(fbdo,base+"/openDoor")){
      if(fbdo.intData()==1){
        unlockDoor(i);
        Firebase.setInt(fbdo,base+"/openDoor",0);
      }
    }
  }
}

/* ---------- START CHARGING ---------- */
void startCharging(int i){
  charging[i]=true;
  chargeStart[i]=millis();
  digitalWrite(chargerRelay[i],HIGH);
  Firebase.setInt(fbdo,"/locker/slot"+String(i+1)+"/charging",1);
  printSlotStatus(i,"Charging STARTED");
}

/* ---------- STOP CHARGING ---------- */
void stopCharging(int i){
  charging[i]=false;
  digitalWrite(chargerRelay[i],LOW);
  Firebase.setInt(fbdo,"/locker/slot"+String(i+1)+"/charging",0);
  printSlotStatus(i,"Charging STOPPED");
}

/* ---------- TIMEOUT CHECK ---------- */
void checkChargingTimeout(){
  for(int i=0;i<6;i++){
    if(charging[i]){
      if(millis()-chargeStart[i]>CHARGE_TIME){
        stopCharging(i);
        unlockDoor(i);
        printSlotStatus(i,"Charging TIMEOUT");
      }
    }
  }
}

/* ---------- DOOR UNLOCK ---------- */
void unlockDoor(int i){
  digitalWrite(doorRelay[i],HIGH);
  doorPulseActive[i]=true;
  doorPulseStart[i]=millis();
  printSlotStatus(i,"Door UNLOCK pulse");
}

/* ---------- DOOR PULSE ---------- */
void handleDoorPulse(){
  for(int i=0;i<6;i++){
    if(doorPulseActive[i]){
      if(millis()-doorPulseStart[i]>DOOR_PULSE_TIME){
        digitalWrite(doorRelay[i],LOW);
        doorPulseActive[i]=false;
      }
    }
  }
}

/* ---------- CHARGING LED ---------- */
void updateChargingLED(){
  bool anyCharging=false;
  for(int i=0;i<6;i++){
    if(charging[i]) anyCharging=true;
  }
  digitalWrite(STATUS_LED,anyCharging);
}
