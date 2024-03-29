const admin = require('firebase-admin');
const express = require('express');
const fs = require('fs');
const path = require('path');
const axios = require('axios');
const app = express();

var serviceAccount = require('../admin.json');
const { start } = require('repl');
const e = require('express');
const { url } = require('inspector');
admin.initializeApp({
    credential: admin.credential.cert(serviceAccount),
    databaseURL: "https://ep-poc-f1041-default-rtdb.firebaseio.com",
    authDomain: "ep-poc-f1041.firebaseapp.com",
});

const db = admin.database();
var userRef; 


//ID des Geräts eingeben 
function readLoginHtml(fileName, callback) {
    const filePath = path.join(__dirname, `../public/pages/login.html`);
    fs.readFile(filePath, 'utf8', (err, htmlContent) => {
        if (err) {
            console.error(`Fehler beim Lesen der HTML-Datei ${fileName}:`, err);
            callback(null);
        } else {
            callback(htmlContent);
        }
    });
}

app.get('/', (req, res) => {
    readLoginHtml('login.html', (htmlContent) => {
        if (htmlContent) {
            res.status(200).send(htmlContent);
        } else {
            res.status(500).send('Interner Serverfehler');
        }
    });
});

//Start - Benutzerwahl 
function readStartHtml(fileName, callback) {
    const filePath = path.join(__dirname, `../public/pages/start.html`);
    fs.readFile(filePath, 'utf8', (err, htmlContent) => {
        if (err) {
            console.error(`Fehler beim Lesen der HTML-Datei ${fileName}:`, err);
            callback(null);
        } else {
            callback(htmlContent);
        }
    });
}

app.get('/start', (req, res) => {
    readStartHtml('start.html', (htmlContent) => {
        if (htmlContent) {
            res.status(200).send(htmlContent);
        } else {
            res.status(500).send('Interner Serverfehler');
        }
    });
}); 


function selectProfile(selectedProfile, enteredId) {

        var profilePaths = {
            profile1: `/${enteredId}/inventarUserOne`,
            profile2: `/${enteredId}/inventarUserTwo`
        };
        userRef = db.ref(profilePaths[selectedProfile]);
   
}

const OPEN_FOOD_FACTS_API_URL = 'https://world.openfoodfacts.org/api/v0/product/';
app.use(express.static(path.join(__dirname, '../public'))); 
const getBarcodes = async (req, res) => {
    try {
        if (!userRef) {
            return res.status(400).send('Profil wurde nicht ausgewählt');
        }

        // Daten aus Firebase abrufen
        const snapshot = await userRef.once('value');
        const barcodesObject = snapshot.val();

        if (!barcodesObject) {
            // Wenn keine Barcodes vorhanden sind, zeige eine leere Liste an
            const htmlList = '<p>Keine Produkte vorhanden.</p>';
            const htmlFilePath = path.join(__dirname, '../index.html');

            fs.readFile(htmlFilePath, 'utf8', (err, htmlTemplate) => {
                if (err) {
                    console.error('Fehler beim Lesen der HTML-Datei:', err);
                    return res.status(500).send('Interner Serverfehler');
                }

                const finalHtml = htmlTemplate.replace('<!-- Inhalte werden hier dynamisch eingefügt -->', htmlList);

                res.status(200).send(finalHtml);
            });
            return;
        }

        const barcodes = Object.keys(barcodesObject);

        // Informationen für jeden Barcode von der Open Food Facts API abrufen
        const productInfoPromises = barcodes.map(async (barcode) => {
            try {
                const response = await axios.get(`${OPEN_FOOD_FACTS_API_URL}${barcode}.json`);
                const product = response.data.product;
                const barcodeValue = barcodesObject[barcode];

                if (barcodeValue > 0) {
                    if (product && product.product_name) {
                        return { barcode, product, barcodeValue };
                    } else {
                        return { barcode, product: { product_name: barcode }, barcodeValue };
                    }
                } else {
                    return null;
                }
            } catch (error) {
                return { barcode, product: { product_name: 'Nicht gefunden' }, barcodeValue: null };
            }
        });

        const productInfos = await Promise.all(productInfoPromises);

        const htmlList = '<table border="1">' +
            '<caption>Inventar</caption>' +
            '<thead>' +
            '<tr>' +
            '<th>Anzahl</th>' +
            '<th>Produktname</th>' +
            '</tr>' +
            '</thead>' +
            '<tbody>' +
            productInfos.map(info => `<tr><td>${info.barcodeValue}</td><td>${info.product.product_name}</td></tr>`).join('') +
            '</tbody>' +
            '</table>';

        const htmlFilePath = path.join(__dirname, '../index.html');

        // HTML-Datei lesen und mit dynamisch erstellter Liste ersetzen
        fs.readFile(htmlFilePath, 'utf8', (err, htmlTemplate) => {
            if (err) {
                console.error('Fehler beim Lesen der HTML-Datei:', err);
                return res.status(500).send('Interner Serverfehler');
            }

            const finalHtml = htmlTemplate.replace('<!-- Inhalte werden hier dynamisch eingefügt -->', htmlList);

            res.status(200).send(finalHtml);
        });
    } catch (error) {
        console.error('Fehler beim Abrufen der Barcodes:', error);
        res.status(500).send('Interner Serverfehler');
    }
};

app.get('/:id/:profile', (req, res) => {

    const selectedProfile = req.params.profile;
    const enteredId = req.params.id;

    selectProfile(selectedProfile, enteredId); 
    getBarcodes(req, res);
    
});




const PORT = 3000;
app.listen(PORT, () => {
    console.log(`Server läuft auf http://localhost:${PORT}`);
});
