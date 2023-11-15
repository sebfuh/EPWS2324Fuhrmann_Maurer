// Import the functions you need from the SDKs you need
import { initializeApp } from "firebase/app";
import { getDatabase, ref, set } from "firebase/database";
// TODO: Add SDKs for Firebase products that you want to use
// https://firebase.google.com/docs/web/setup#available-libraries

// Your web app's Firebase configuration
// For Firebase JS SDK v7.20.0 and later, measurementId is optional
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
const app = initializeApp(firebaseConfig);
function writeUserData(userId, name, email) {
    const db = getDatabase();
    const reference = ref(db, 'users/' + userId);

    set(reference, {
        username: name,
        email: email,
    });
}

writeUserData("123","name", "email");