import * as query from "/modules/query.js";
//Manual card creator for testing purposes
let count = 1; //Card number
const cardRow = document.getElementById("cardRow"); // Get the card row container
document.querySelector('#test-button').addEventListener("click",async () => { //Listen for button click to add card
    cardRow.insertAdjacentHTML(
        "beforeend",
        card_creator(`Node ${count}`, "Name", 5000)
    )
    count++;
});
function card_creator(id, name, lastSeen){ //Template for card creation
    if (lastSeen >= 10000){ //If not seen for more than 10 mins, mark as offline
        return    `
            <div class="col-12 col-md-4">
                <div class="card text-center shadow-sm">
                    <div class="card-body">
                        <h5 class="card-title">${name}</h5>
                        <h3 class="card-title">${id}</h3>
                        <p class="card-value text-danger">Offline</p>
                        <p class="card-text text-muted">${lastSeen}</p>
                    </div>
                </div>
            </div>`;
    }
    else{ //Otherwise, mark as online
        return    `
            <div class="col-12 col-md-4">
                <div class="card text-center shadow-sm">
                    <div class="card-body">
                        <h5 class="card-title">${name}</h5>
                        <h3 class="card-title">${id}</h3>
                        <p class="card-value text-success">Online</p>
                        <p class="card-text text-muted">${lastSeen}</p>
                    </div>
                </div>
            </div>`;
    }
}
//Automatically load cards on page load from data in the backend
document.addEventListener("DOMContentLoaded", nodeCall);
function nodeCall(){
    const container = document.getElementById("cardRow");

    fetch("http://localhost:3000/web/nodes-load") //Grabs data from backend
        .then(response => {
            if (!response.ok) { //Checks if response is valid
                throw new Error("Network error");
            }
            return response.json();
        })
        .then(data => {
            data.forEach(node => { //Parses data and creates cards for each node
                container.insertAdjacentHTML(
                    "beforeend",
                    card_creator(node.id, node.name, node.ls)
                );
            });
        })
        .catch(error => {
            console.error("Error:", error);
        });
};