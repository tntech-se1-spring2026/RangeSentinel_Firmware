import * as nodes from "/modules/nodes.js";

document.addEventListener("DOMContentLoaded", updateAll); //Loads all nodes when page is loaded
document.querySelector("#notif-button").addEventListener("click", testNotification); //Tests notification retrieval when button is clicked
document.addEventListener("click", function(event) { //Event delegation for alert clear buttons
    if (event.target.classList.contains("alert-clear")) {
        const card = event.target.closest('.card');
        card.classList.remove('bg-warning');
        card.classList.add('bg-light');
        const nodeId = card.id;
        const notification = document.querySelector(
            `.dropdown-item[data-node-id="${nodeId}"]`
        );

        if (notification) {
            console.log(`Clearing notification for node ${nodeId}`);
            notification.remove();
        }
    }
});

function updateAll() {
  nodes.update();
}

function testNotification() {
  nodes.test_notification();
}
