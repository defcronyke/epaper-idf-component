(function() {

function reconnect(timeout, i) {
    if (timeout != 0 && !timeout) {
        timeout = 4000;
    }

    if (!i) {
        i = 1;
    }

    var timeoutSecs = timeout / 1000.0;

    console.log('Reconnect requested. Will attempt reconnect in ' + timeoutSecs + ' seconds.');

    window.setTimeout(function() {
        console.log('Attempting reconnect with ' + timeoutSecs + ' second timeout.\nAttempt #: ' + i);

        fetch(window.location.href)
        .then(function() {
            window.location.reload();
        })
        .catch(function(err) {
            console.log(err);
            window.setTimeout(function() {
                i++;
                reconnect(timeout, i);
            }, timeout);
        });
    }, timeout);
}

function restartButtonHandler() {
    if (confirm('Would you like to restart the device?')) {
        fetch('/api/util/restart', {
            method: 'POST',
        })
        .then(function(res) {
            if (res.status === 404) {
                return { msg: 'DEMO MODE: no-op' };
            }

            return res.json();
        })
        .then(function(res) {
            console.log('POST /api/util/restart response:');
            console.log(res);
            return res;
        })
        .catch(function(err) {
            console.log('error: unexpected response from server:');
            console.log(err);
            return err;
        });
        
        // attempt reconnect
        var reconnect_timeout = 4000;
        reconnect(reconnect_timeout);

        var utilStatus = document.getElementById('util-status');

        var utilStatusMsg = 'The device is restarting. This page will refresh when the device is ready. Please wait...';

        console.log(utilStatusMsg);

        utilStatus.innerText = 'The device is restarting. This page will refresh when the device is ready. Please wait...';
    }
}

function initButtons() {
    console.log('initializing buttons...');

    var restartButton = document.getElementById('restart');
    if (!!restartButton) {
        restartButton.addEventListener('click', function() {
            restartButtonHandler();
        });
    }

    console.log('buttons initialized');
}

function main() {
    console.log('js loaded');

    initButtons();
}

main();

})();
