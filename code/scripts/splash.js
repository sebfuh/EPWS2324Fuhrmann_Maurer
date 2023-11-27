document.addEventListener("DOMContentLoaded", function() {
    setTimeout(function() {
        document.getElementById("splashScreen").style.display = "none";
        window.location.href = "../index.html"; // Weiterleitung zur index.html
    }, 2000); // 2 Sekunden SplashScreen
});