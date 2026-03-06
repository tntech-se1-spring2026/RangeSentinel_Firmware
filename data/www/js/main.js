import * as nodes from "/modules/nodes.js";
import * as query from "/modules/query.js";

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
document.addEventListener("click", function (event) { //Event delegation for edit name buttons
    const editBtn = event.target.closest(".edit-name");

    if (!editBtn) return;

    const card = editBtn.closest(".card");
    const title = card.querySelector(".card-title");
    const currentName = title.textContent;
    title.innerHTML = `
        <input type="text"
               class="form-control form-control-sm name-input"
               value="${currentName}">`;
    const input = title.querySelector(".name-input");
    input.focus();
});
document.addEventListener("keydown", function (event) { //Event delegation for name input fields
    if (!event.target.classList.contains("name-input")) return;

    if (event.key === "Enter") { //When enter is pressed, update the card title and log the change
        const input = event.target;
        const newName = input.value.trim();
        const card = input.closest(".card");
        const nodeId = card.id;
        const title = card.querySelector(".card-title");
        title.textContent = newName;
        console.log(`Renamed node ${nodeId} to ${newName}`);
    }
});

document.querySelector('#wifi-button').addEventListener('click', async () => {
    let newPassword;
    newPassword = document.querySelector('#wifi-password').value;
    newPassword = newPassword.trim();
    
    if (newPassword.length < 8) {
        document.querySelector('#wifi-help').innerHTML = "Password must be at least 8 characters";
    } else {
        let response = await query.set_wifi_password(newPassword);
        if (!response.ok) {
            document.querySelector('#wifi-help').innerHTML = await response.text();
        }
        document.querySelector('#wifi-help').innerHTML = "";
    }
});

function updateAll() {
  nodes.update();
}

function testNotification() {
  nodes.test_notification();
}
