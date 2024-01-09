const admin = require('firebase-admin');
const express = require('express');
const fs = require('fs');
const path = require('path');
const app = express();

var serviceAccount = require('../admin.json');
admin.initializeApp({
    credential: admin.credential.cert(serviceAccount),
    databaseURL: "https://ep-poc-f1041-default-rtdb.firebaseio.com",
    authDomain: "ep-poc-f1041.firebaseapp.com",
});

// Referenz auf deine Datenbank
const db = admin.database();
const userRef = db.ref('test/barcodes');

const getBarcodes = (req, res) => {
    userRef.once('value', function (snap) {
        const data = snap.val();
        console.log('Inhalte der Firebase Realtime Database:', data);

        // HTML-Liste dynamisch erstellen
        const htmlList = '<ul>' +
            Object.keys(data).map(key => `<li>${key}: ${data[key]}</li>`).join('') +
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
    });
};

// Beispielroute in Express
app.get('/barcodes', getBarcodes);

const PORT = 3000; // Wähle einen geeigneten Port
app.listen(PORT, () => {
    console.log(`Server läuft auf http://localhost:${PORT}`);
});
