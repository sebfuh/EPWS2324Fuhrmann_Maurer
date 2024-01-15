#include <Arduino.h>
#include <Firebase_ESP_Client.h>
#include <SoftwareSerial.h>
#include "addons/RTDBHelper.h"
#include "addons/TokenHelper.h"

#define WIFI_SSID "Hier ist alles super"
#define WIFI_PASSWORD "Wlan-FBI2022"
#define API_KEY "AIzaSyAFjj-U3Ylj5daf__Zzq3wllb4GiRF3kio"
#define DATABASE_URL "https://ep-poc-f1041-default-rtdb.firebaseio.com/"

const int scannerTxPin = 17;
const int scannerRxPin = 16;
SoftwareSerial scannerSerial(scannerTxPin, scannerRxPin);

const int ledPinGreen = 22;
const int ledPinRed = 33;
const int switchP = 14;

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
unsigned long sendDataPrevMillis = 0;
bool signupOK = false;

void setup() {
  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("ok");
    signupOK = true;
  } else {
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  config.token_status_callback = tokenStatusCallback;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  pinMode(ledPinGreen, OUTPUT);
  pinMode(ledPinRed, OUTPUT);
  pinMode(switchP, INPUT_PULLUP);
  scannerSerial.begin(9600);
}

void loop() {
  int switchState = digitalRead(switchP);

  if (switchState == HIGH) {
    if (scannerSerial.available() > 0) {
      String barcodeData = scannerSerial.readStringUntil('\r');
      if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 1500 || sendDataPrevMillis == 0)) {
        sendDataPrevMillis = millis();
        String databasePath = "inventar/" + barcodeData;
        updateBarcodeCount(barcodeData);
      }
    }
  }

  if (switchState == LOW) {
    if (scannerSerial.available() > 0) {
      String barcodeDataDelete = scannerSerial.readStringUntil('\r');
      Serial.println("Barcode-to-delete: " + barcodeDataDelete);
      decreaseBarcodeCount(barcodeDataDelete);
    }
  }
}

void decreaseBarcodeCount(String barcode) {
  String path = "inventar/" + barcode;

  if (Firebase.RTDB.getString(&fbdo, path.c_str())) {
    String currentValue = fbdo.stringData();

    if (currentValue == "null") {
      if (Firebase.RTDB.setInt(&fbdo, path.c_str(), 0)) {
        Serial.println("Barcode nicht vorhanden, Anzahl auf 0 gesetzt: " + barcode);
        digitalWrite(ledPinGreen, HIGH);
        delay(2000);
        digitalWrite(ledPinGreen, LOW);
      } else {
        Serial.println("Fehler beim Aktualisieren des Barcodes");
        Serial.println(fbdo.errorReason());
        digitalWrite(ledPinRed, HIGH);
        delay(2000);
        digitalWrite(ledPinRed, LOW);
      }
    } else {
      int count = currentValue.toInt();
      count = max(count - 1, 0);

      if (Firebase.RTDB.setInt(&fbdo, path.c_str(), count)) {
        Serial.println("Anzahl des Barcodes um 1 verringert: " + barcode);
        Serial.println("Neue Anzahl: " + String(count));
        digitalWrite(ledPinGreen, HIGH);
        delay(2000);
        digitalWrite(ledPinGreen, LOW);
      } else {
        Serial.println("Fehler beim Aktualisieren der Anzahl des Barcodes");
        Serial.println(fbdo.errorReason());
        digitalWrite(ledPinRed, HIGH);
        delay(2000);
        digitalWrite(ledPinRed, LOW);
      }
    }
  } else {
    Serial.println("Fehler beim Lesen des Barcodes");
    Serial.println(fbdo.errorReason());
    digitalWrite(ledPinRed, HIGH);
    delay(2000);
    digitalWrite(ledPinRed, LOW);
  }
}

void updateBarcodeCount(String barcode) {
  String path = "inventar/" + barcode;

  if (Firebase.RTDB.getString(&fbdo, path.c_str())) {
    String currentValue = fbdo.stringData();

    if (currentValue == "null") {
      if (Firebase.RTDB.setInt(&fbdo, path.c_str(), 1)) {
        Serial.println("Barcode hinzugefügt: " + barcode);
        Serial.println("Anzahl: 1");
        digitalWrite(ledPinGreen, HIGH);
        delay(2000);
        digitalWrite(ledPinGreen, LOW);
      } else {
        Serial.println("Fehler beim Hinzufügen des Barcodes");
        Serial.println(fbdo.errorReason());
        digitalWrite(ledPinRed, HIGH);
        delay(2000);
        digitalWrite(ledPinRed, LOW);
      }
    } else {
      int count = currentValue.toInt();
      count++;

      if (Firebase.RTDB.setInt(&fbdo, path.c_str(), count)) {
        Serial.println("Anzahl des Barcodes aktualisiert: " + barcode);
        Serial.println("Neue Anzahl: " + String(count));
        digitalWrite(ledPinGreen, HIGH);
        delay(2000);
        digitalWrite(ledPinGreen, LOW);
      } else {
        Serial.println("Fehler beim Aktualisieren der Anzahl des Barcodes");
        Serial.println(fbdo.errorReason());
        digitalWrite(ledPinRed, HIGH);
        delay(2000);
        digitalWrite(ledPinRed, LOW);
      }
    }
  } else {
    if (Firebase.RTDB.setString(&fbdo, path.c_str(), "1")) {
      Serial.println("Barcode hinzugefügt: " + barcode);
      Serial.println("Anzahl: 1");
      digitalWrite(ledPinGreen, HIGH);
      delay(2000);
      digitalWrite(ledPinGreen, LOW);
    } else {
      Serial.println("Fehler beim Hinzufügen des Barcodes");
      Serial.println(fbdo.errorReason());
      digitalWrite(ledPinRed, HIGH);
      delay(2000);
      digitalWrite(ledPinRed, LOW);
    }
  }
}
