#include <Arduino.h>
#include <WiFi.h>
//#include <Firebase_ESP_Client.h>
#include <SoftwareSerial.h> //ESPSoftwareSerial
#include <FirebaseESP32.h> //Firebase ESP32 Client


#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"


//#define WIFI_SSID "Hier ist alles super"
//#define WIFI_PASSWORD "Wlan-FBI2022"
#define WIFI_SSID "moxd-lab-test-net"          
#define WIFI_PASSWORD "!Moxd3209#"

#define API_KEY "AIzaSyAFjj-U3Ylj5daf__Zzq3wllb4GiRF3kio"
#define DATABASE_URL "https://ep-poc-f1041-default-rtdb.firebaseio.com/" 

// Konfiguration des Barcode-Scanners
const int scannerTxPin = 17;  // TX-Pin des Barcode-Scanners
const int scannerRxPin = 16;  // RX-Pin des Barcode-Scanners
SoftwareSerial scannerSerial(scannerTxPin, scannerRxPin);

// LED
const int ledPinGreen = 22; 
const int ledPinRed = 33;

// SWITCH
const int switchP = 14;


//Firebase
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
unsigned long sendDataPrevMillis = 0;
bool signupOK = false;


void setup(){
  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
    while (WiFi.status() != WL_CONNECTED){
      Serial.print(".");
      delay(300);
    }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

// FIREBASE
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  /* Sign up */
    if (Firebase.signUp(&config, &auth, "", "")){
      Serial.println("ok");
      signupOK = true;
    }
      else{
        Serial.printf("%s\n", config.signer.signupError.message.c_str());
      }

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; 
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  // LED
  pinMode(ledPinGreen, OUTPUT);
  pinMode(ledPinRed, OUTPUT);

  //SWITCH
  pinMode(switchP, INPUT_PULLUP);

  // Barcode-Scanner
  scannerSerial.begin(9600);
}


void loop(){

 int switchState = digitalRead(switchP);

 if (switchState == HIGH){

  if (scannerSerial.available() > 0) {
    String barcodeData = scannerSerial.readStringUntil('\r');

    if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 15000 || sendDataPrevMillis == 0)){
      sendDataPrevMillis = millis();
      
      // Write on the database path test/barcodes
      if (Firebase.RTDB.push(&fbdo, "test/barcodes/", barcodeData)){
        Serial.println("PASSED");
        Serial.println("Barcode: " + barcodeData);
        Serial.println("PATH: " + fbdo.dataPath());
        Serial.println("TYPE: " + fbdo.dataType());
        digitalWrite(ledPinGreen, HIGH);
        delay(2000);
        digitalWrite(ledPinGreen, LOW);
      }
        else {
          Serial.println("FAILED");
          Serial.println("REASON: " + fbdo.errorReason());
          digitalWrite(ledPinRed, HIGH);
          delay(2000);
          digitalWrite(ledPinRed, LOW);
        }
        
    }
 
 }
 }
 if (switchState == LOW){
  if (scannerSerial.available() > 0) {
          String barcodeDataDelete = scannerSerial.readStringUntil('\r');
          Serial.println("Barcode-to-delete: " + barcodeDataDelete);
          deleteBarcodeFromFirebase(barcodeDataDelete);
        }
}

}


void deleteBarcodeFromFirebase(String barcode) {
  String path = "test/barcodes/" + barcode;

  if (Firebase.RTDB.set(&fbdo, path, "null")) {
    Serial.println("Barcode value set null successfully"); 
    digitalWrite(ledPinGreen, HIGH);
    delay(2000);
    digitalWrite(ledPinGreen, LOW);
  } else {
    Serial.println("Barcode value set null NOT successfully");
    Serial.println(fbdo.errorReason());
    digitalWrite(ledPinRed, HIGH);
    delay(2000);
    digitalWrite(ledPinRed, LOW);
    }
}


