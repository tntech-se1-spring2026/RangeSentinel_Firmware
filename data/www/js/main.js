import * as nodes from "/modules/nodes.js";

document.addEventListener("DOMContentLoaded", updateAll);
document.querySelector("#notif-button").addEventListener("click", testNotification);

function updateAll() {
  nodes.update();
}

function testNotification() {
  nodes.test_notification();
}