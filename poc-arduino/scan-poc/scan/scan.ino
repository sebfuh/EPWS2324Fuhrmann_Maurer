#include <Arduino.h>
#include <Firebase_ESP_Client.h>
#include <SoftwareSerial.h>
#include "addons/RTDBHelper.h"
#include "addons/TokenHelper.h"
#include <esp_system.h>
#include <ArduinoJson.h>

//#define WIFI_SSID "HUAWEI P20"
//#define WIFI_PASSWORD "08150000"
#define WIFI_SSID "Hier ist alles super"
#define WIFI_PASSWORD "Wlan-FBI2022"
//#define WIFI_SSID "moxd-lab-test-net"
//#define WIFI_PASSWORD "!Moxd3209#"

#define API_KEY "AIzaSyAFjj-U3Ylj5daf__Zzq3wllb4GiRF3kio"
#define DATABASE_URL "https://ep-poc-f1041-default-rtdb.firebaseio.com/" 

const int scannerTxPin = 17;
const int scannerRxPin = 16;
SoftwareSerial scannerSerial(scannerTxPin, scannerRxPin);

const int ledPinGreen = 22;
const int ledPinRed = 33;
const int switchS = 14;
const int switchU = 15; 

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
unsigned long sendDataPrevMillis = 0;
bool signupOK = false;
String uniquePath;
String uuid;

void setup() {
  Serial.begin(115200);

  // Initialisiere LEDs
  pinMode(ledPinGreen, OUTPUT);
  pinMode(ledPinRed, OUTPUT);

  digitalWrite(ledPinRed, HIGH); 
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
  digitalWrite(ledPinRed, LOW);  

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



  pinMode(switchS, INPUT_PULLUP);
  pinMode(switchU, INPUT_PULLUP);
  scannerSerial.begin(9600);
}

void loop() {
  int switchState = digitalRead(switchS);
  int switchUser = digitalRead(switchU);
  String uuid = generateUUID();

  if (switchState == HIGH) {
    if (scannerSerial.available() > 0) {
      String barcodeData = scannerSerial.readStringUntil('\r');
      if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 1500 || sendDataPrevMillis == 0)) {
        sendDataPrevMillis = millis();
        String databasePath;
        if (switchUser == HIGH) {
          databasePath = uuid + "/inventarUserOne/" + barcodeData;
        } else {
          databasePath = uuid + "/inventarUserTwo/" + barcodeData;
        }
        updateBarcodeCount(barcodeData, databasePath);
      }
    }
  }

  if (switchState == LOW) {
    if (scannerSerial.available() > 0) {
      String barcodeDataDelete = scannerSerial.readStringUntil('\r');
      Serial.println("Barcode-to-delete: " + barcodeDataDelete);
      String databasePath;
      if (switchUser == HIGH) {
        databasePath = uuid + "/inventarUserOne/" + barcodeDataDelete;
      } else {
        databasePath = uuid + "/inventarUserTwo/" + barcodeDataDelete;
      }
      decreaseBarcodeCount(barcodeDataDelete, databasePath);
    }
  }
}




//Functions -> scan Barcodes 
void decreaseBarcodeCount(String barcode, String path) {
 // String path = "inventar/" + barcode;

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

void updateBarcodeCount(String barcode, String path) {
  //String path = "inventar/" + barcode;

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
        Serial.println(String(path));
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
      Serial.println(String(path));
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

String generateUUID() {
  uint8_t mac[6];
  esp_read_mac(mac, ESP_MAC_WIFI_STA);

  char uuid[36];
  sprintf(uuid, "%02X%02X%02X%02X%02X%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

  return String(uuid);
}
