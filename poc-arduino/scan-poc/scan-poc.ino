#include <SoftwareSerial.h>

// Konfiguration des Barcode-Scanners
const int scannerTxPin = 17;  // TX-Pin des Barcode-Scanners
const int scannerRxPin = 16;  // RX-Pin des Barcode-Scanners
SoftwareSerial scannerSerial(scannerTxPin, scannerRxPin);

void setup() {
  // Initialisiere die serielle Kommunikation mit dem PC
  Serial.begin(115200);

  // Initialisiere die serielle Kommunikation mit dem Barcode-Scanner
  scannerSerial.begin(9600);
}

void loop() {
  // Überprüfe, ob Daten vom Barcode-Scanner verfügbar sind
  if (scannerSerial.available() > 0) {
    // Lese die empfangenen Daten vom Barcode-Scanner
    String barcodeData = scannerSerial.readStringUntil('\n');
    
    // Gib den Barcode im Serial Monitor aus
    Serial.println("Barcode: " + barcodeData);
  }
}
