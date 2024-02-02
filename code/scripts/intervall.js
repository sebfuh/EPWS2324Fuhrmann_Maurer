setInterval(() => {
    delete require.cache[require.resolve('./getFirebase.js')];
}, 10000);