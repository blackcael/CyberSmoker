function updateSmokerData() {
    fetch('/smoker_data')
        .then(response => response.json())
        .then(data => {
            document.getElementById('meat_temp').innerText = data.meat_temp;
            document.getElementById('ambient_temp').innerText = data.ambient_temp;
            document.getElementById('set_temp').innerText = data.set_temp;
            document.getElementById('pellet_level').innerText = data.pellet_level;
        });
}

function sendCommand(cmd) {
    fetch('/command', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ command: cmd })
    });
}

// Poll temperature every 2 seconds
setInterval(updateSmokerData, 2000);
updateSmokerData();
