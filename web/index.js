(function() {

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

            alert('The device is restarting...\n\n' + res.msg);

            window.location.reload();

            return res;
        })
        .catch(function(err) {
            console.log('error: unexpected response from server:');
            console.log(err);
            return err;
        });
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
