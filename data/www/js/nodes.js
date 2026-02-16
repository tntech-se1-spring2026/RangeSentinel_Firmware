import * as query from "/modules/query.js";
//Manual card creator for testing purposes
/*
let count = 1; //Card number
const cardRow = document.getElementById("cardRow"); // Get the card row container
document.querySelector('#test-button').addEventListener("click",async () => { //Listen for button click to add card
    cardRow.insertAdjacentHTML(
        "beforeend",
        card_creator(`Node ${count}`, "Name", 5000)
    )
    count++;
});

async function fetch_data(){
    const nodes_data_promise = await query.get_nodes();
    const node_data = await nodes_data_promise.json();
    console.log(node_data);
    let node_network = [];
    for (const iter in node_data["nodes"]){
        console.log(node_data["nodes"][iter]);
        let id_name =node_data["nodes"][iter]["id"];
        let condition = check_time(node_data["nodes"][iter]["lastSeen"]);
        let alert_status = node_data["nodes"][iter]["alert_status"]
        let single_node = [id_name,condition,alert_status];
        node_network.push(single_node);
    }
    return node_network; //Returns an array of array of the nodes and its id and conditon
}
*/
async function alert_check(){
    let alerted_nodes = [];
    let refresh_page = false;
    for (let i = 0; i <= node_network.length;i++){
        if(node_network[i][2] == true){
            alerted_nodes.push(node_network[i]);
            refresh_page = true;
        }
    }
}
//Types of notifications
/*

*/
//Refresh Data
const time = 1000000;
async function refresh_cards(){
    const cardRow = document.getElementById("cardRow");
    cardRow.innerHTML ="";
    const nodes_data_promise = await query.get_nodes();
    const node_data = await nodes_data_promise.json();
    for (const iter in node_data["nodes"]){
        console.log(node_data["nodes"][iter]);
        let id_name =node_data["nodes"][iter]["id"];
        let condition = check_time(node_data["nodes"][iter]["lastSeen"]);
        let alert_status = node_data["nodes"][iter]["alert_status"];
        card_creator(id_name,alert_status,condition);
    }
    
}
//Refresh the data every 10 minutes
setTimeout(()=>{
    refresh_cards();
},1000000) //10 min

function card_creator(id, name, lastSeen){ //Template for card creation
    const current_time = Date.now();
    const time_difference = current_time - lastSeen
    if (time_difference >= 1000000){  //If not seen for more than 10 mins, mark as offline
        let online_status = "Offline";
        let text_style = "text-danger";
    }
    else{
        let online_status = "Online";   //Otherwise, mark as online
        let text_style = "text-success";
    }
    return    `
        <div class="col-12 col-md-4">
            <div class="card text-center shadow-sm">
                <div class="card-body">
                    <h5 class="card-title">${name}</h5>
                    <h3 class="card-title">${id}</h3>
                    <p class="card-value ${text_style}">${online_status}</p>
                    <p class="card-text text-muted">${lastSeen}</p>
                </div>
            </div>
        </div>`;
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