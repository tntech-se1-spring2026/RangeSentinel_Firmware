import * as nodes from "/modules/nodes.js";

document.addEventListener("DOMContentLoaded", updateAll);
document.querySelector("#notif-button").addEventListener("click", testNotification);
document.addEventListener("click", function(event) {
    if (event.target.classList.contains("alert-clear")) {
        const card = event.target.closest('.card');
        card.classList.remove('bg-warning');
        card.classList.add('bg-light');
    }
});

function updateAll() {
  nodes.update();
}

function testNotification() {
  nodes.test_notification();
}
