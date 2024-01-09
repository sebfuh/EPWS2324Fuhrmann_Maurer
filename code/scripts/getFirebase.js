const admin = require('firebase-admin');
const express = require('express');
const fs = require('fs');
const path = require('path');
const axios = require('axios');  // Füge Axios für HTTP-Anfragen hinzu
const app = express();

var serviceAccount = require('../admin.json');
admin.initializeApp({
    credential: admin.credential.cert(serviceAccount),
    databaseURL: "https://ep-poc-f1041-default-rtdb.firebaseio.com",
    authDomain: "ep-poc-f1041.firebaseapp.com",
});

// Referenz auf deine Firebase Realtime Database
const db = admin.database();
const userRef = db.ref('inventar');
const OPEN_FOOD_FACTS_API_URL = 'https://world.openfoodfacts.org/api/v0/product/';


const getBarcodes = async (req, res) => {
    try {
        // Daten aus Firebase abrufen
        const snapshot = await userRef.once('value');
        const barcodes = Object.keys(snapshot.val());

        // Informationen für jeden Barcode von der Open Food Facts API abrufen
        const productInfoPromises = barcodes.map(async (barcode) => {
            const response = await axios.get(`${OPEN_FOOD_FACTS_API_URL}${barcode}.json`);
            const product = response.data.product;
            const barcodeValue = snapshot.val()[barcode];
            // Nur Elemente mit barcodeValue > 0 berücksichtigen
            if (barcodeValue > 0) {
                return { barcode, product, barcodeValue };
            } else {
                return null;
            }
        });

        // Warten, bis alle API-Anfragen abgeschlossen sind und filtere gleichzeitig null-Elemente heraus
const productInfos = (await Promise.all(productInfoPromises)).filter(info => info !== null);


        // HTML-Liste dynamisch erstellen
        const htmlList = '<ul>' +
        productInfos.map(info => `<li>Anzahl: ${info.barcodeValue}, ${info.barcode}: ${info.product.product_name}</li>`).join('') +
            '</ul>';

        // Pfad zur HTML-Datei
        const htmlFilePath = path.join(__dirname, '../index.html');

        // HTML-Datei lesen und mit dynamisch erstellter Liste ersetzen
        fs.readFile(htmlFilePath, 'utf8', (err, htmlTemplate) => {
            if (err) {
                console.error('Fehler beim Lesen der HTML-Datei:', err);
                return res.status(500).send('Interner Serverfehler');
            }

            // Platzhalter ersetzen
            const finalHtml = htmlTemplate.replace('<!-- Inhalte werden hier dynamisch eingefügt -->', htmlList);

            // HTML als Antwort senden
            res.status(200).send(finalHtml);
        });
    } catch (error) {
        console.error('Fehler beim Abrufen der Barcodes:', error);
        res.status(500).send('Interner Serverfehler');
    }
};

// Beispielroute in Express
app.get('/barcodes', getBarcodes);

const PORT = 3000; // Wähle einen geeigneten Port
app.listen(PORT, () => {
    console.log(`Server läuft auf http://localhost:${PORT}`);
});
