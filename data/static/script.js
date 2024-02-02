let $currentSeqDisplay = null;
let $currentSpeedDisplay = null;
let $selectSeq = null;
let $selectSpeed = null;
let $progBar = null;
let $btnApply = null;
let $footer = null;
let $btnSetDefault = null;

let SequenceList = null;

const Speed = {
    500: 'Fast',
    1000: 'Normal',
    2000: 'Slow'
};

window.addEventListener('load', (_) => {
    $currentSeqDisplay = document.getElementById('currentSeq');
    $currentSpeedDisplay = document.getElementById("currentSpeed");

    $progBar = document.getElementById('refreshTimerBar');

    $selectSeq = document.getElementById('selectSequence');
    $selectSpeed = document.getElementById('selectSpeed');

    $btnApply = document.getElementById("btnApply");
    $btnSetDefault = document.getElementById("btnSetDefault");

    $footer = document.getElementsByTagName("footer")[0];

    SequenceList = [];
    try {
        SequenceList = JSON.parse(document.getElementById("sequencelist").textContent);
    } catch (_) { };

    SequenceList.forEach((e, i) => {
        var opt = document.createElement('option');
        opt.value = i;
        opt.innerHTML = e;
        $selectSeq.appendChild(opt);
    });
    getStatus();
});

let progbarValue = 0;
async function startNewRefreshTimer() {
    await new Promise(r => setTimeout(r, 500));
    progbarValue = 0;
    $progBar.value = progbarValue;
    let progBarInterval = setInterval(() => {
        progbarValue += 5;
        $progBar.value = progbarValue;
        if (progbarValue >= 100) {
            getStatus();
            clearInterval(progBarInterval);
        }
    }, 100);
}

async function getStatus() {
    try {
        const resp = await fetch("/status", {
            method: "GET"
        });
        const textResp = await resp.text();
        handleStatusJson(textResp);
    } catch (err) {
        console.log(err);
        $currentSeqDisplay.value = 'Lost connection...';
    } finally {
        startNewRefreshTimer();
    }
}

async function setSeq() {
    disableButtons();

    let payload = JSON.stringify({
        "command": "setSequence",
        'sequenceID': parseInt($selectSeq.value),
        'speed': parseInt($selectSpeed.value)
    });

    try {
        const resp = await fetch("/rpc", {
            method: "POST",
            body: payload,
            headers: {
                "Content-Type": "application/json",
            }
        });
        const textResp = await resp.text();
        if (!resp.ok) {
            addNotif(resp.textResp);
            return;
        }
        handleStatusJson(textResp);
    } catch (err) {
        addNotif(err);
    } finally {
        enableButtons(500);
    }
}

function addNotif(message) {
    let html = `<div class="card opaque">
        <span class="closebtn" onclick="this.parentElement.remove();">✖&nbsp;</span>
        ${message}
    </div>`;
    $footer.innerHTML += html;
}

function disableButtons() {
    $btnApply.disabled = true;
    $btnSetDefault.disabled = true;
}

function enableButtons(delay) {
    setTimeout(() => {
        $btnApply.disabled = false;
        $btnSetDefault.disabled = false;
    }, delay);
}

async function setDefault() {
    disableButtons();

    let payload = JSON.stringify({
        "command": "setDefault",
        'sequenceID': parseInt($selectSeq.value),
        'speed': parseInt($selectSpeed.value)
    });

    try {
        const resp = await fetch("/rpc", {
            method: "POST",
            body: payload,
            headers: {
                "Content-Type": "application/json",
            }
        });

        if (!resp.ok) {
            addNotif(await resp.text());
            return;
        }

        const jsonResp = await resp.json();
        const message = `Default Sequence set to: ${SequenceList[jsonResp['sequenceID']]} | ${Speed[jsonResp['speed']]}`;
        addNotif(message);
    } catch (err) {
        addNotif(err)
    } finally {
        enableButtons(500);
    };
}


function handleStatusJson(plaintext) {
    try {
        let jsonResponse = JSON.parse(plaintext);
        $currentSeqDisplay.textContent = SequenceList[jsonResponse['sequenceID']] + " | " + Speed[jsonResponse['speed']];
    } catch (err) {
        addNotif(err);
    } finally {
    }
}