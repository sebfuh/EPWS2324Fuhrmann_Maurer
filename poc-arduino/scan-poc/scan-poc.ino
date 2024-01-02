#include <Arduino.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <SoftwareSerial.h>

#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"


#define WIFI_SSID "Hier ist alles super"
#define WIFI_PASSWORD "Wlan-FBI2022"
// const char *WIFI_SSID = "moxd-lab-test-net";          
//const char *WIFI_PASS = "!Moxd3209#";

#define API_KEY "AIzaSyAFjj-U3Ylj5daf__Zzq3wllb4GiRF3kio"
#define DATABASE_URL "https://ep-poc-f1041-default-rtdb.firebaseio.com/" 

// Konfiguration des Barcode-Scanners
const int scannerTxPin = 17;  // TX-Pin des Barcode-Scanners
const int scannerRxPin = 16;  // RX-Pin des Barcode-Scanners
SoftwareSerial scannerSerial(scannerTxPin, scannerRxPin);

// LED
const int ledPinGreen = 22; 
const int ledPinRed = 33;


//Define Firebase Data object
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

  // Barcode-Scanner
  scannerSerial.begin(9600);
}


oid loop(){

  if (scannerSerial.available() > 0) {
    String barcodeData = scannerSerial.readStringUntil('\n');

    Serial.println(barcodeData); 

    Serial.println("Barcode: " + barcodeData);

    if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 15000 || sendDataPrevMillis == 0)){
      sendDataPrevMillis = millis();
      
    
      // Write on the database path test/int
      if (Firebase.RTDB.setString(&fbdo, "test/int", barcodeData)){
        Serial.println("PASSED");
        Serial.println("PATH: " + fbdo.dataPath());
        Serial.println("TYPE: " + fbdo.dataType());
        Serial.println("Barcode: " + barcodeData);
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
