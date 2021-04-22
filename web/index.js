(function() {

function reconnect(timeout) {
    if (timeout != 0 && !timeout) {
        timeout = 4000;
    }

    window.setTimeout(function() {
        fetch(window.location.href)
        .then(function() {
            window.location.reload();
        })
        .catch(function(err) {
            console.log(err);
            window.setTimeout(function() {
                reconnect(timeout);
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
