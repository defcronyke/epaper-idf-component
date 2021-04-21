(function() {

function restartButtonHandler() {
    alert('restarting device...');
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
