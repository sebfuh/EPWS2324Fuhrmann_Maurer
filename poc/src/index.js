/* var express = require('express');
var app = express();

app.get('/',function(req, res) {
    res.sendFile(__dirname + '../public/index.html');
    res.send()
});

 */

import { initializeApp } from "firebase/app";
import { getDatabase, ref, set } from "firebase/database";

const firebaseConfig = {
  apiKey: "AIzaSyAFjj-U3Ylj5daf__Zzq3wllb4GiRF3kio",
  authDomain: "ep-poc-f1041.firebaseapp.com",
  databaseURL: "https://ep-poc-f1041-default-rtdb.firebaseio.com",
  projectId: "ep-poc-f1041",
  storageBucket: "ep-poc-f1041.appspot.com",
  messagingSenderId: "354595581987",
  appId: "1:354595581987:web:5b782cb4af904d64cc4196",
  measurementId: "G-GK5MPP14ZY"
};

// Initialize Firebase
const appDb = initializeApp(firebaseConfig);


function generateUserId() {  
  const timestamp = new Date().getTime();
  const randomChars = Math.random().toString(36).substring(7);
  return `${timestamp}-${randomChars}`;
}

function writeUserData(item1, item2) {
    const db = getDatabase();
    const userId = "1700113958540-1femwh";       //generateUserId();
    const reference = ref(db, 'inhalte/' + userId);

    set(reference, {
        item1: item1,
        item2: item2,
    });
    console.log(`User ID: ${userId}, item1: ${item1}, item2: ${item2}`);
}

writeUserData("Apfel", "Birne");

