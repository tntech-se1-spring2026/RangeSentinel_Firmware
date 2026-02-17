import * as query from "/modules/query.js";
import * as utils from "/modules/utils.js";

export async function update(){
    const response = await query.get_nodes();
    const node_statuses = await response.json();

    console.log(node_statuses);
    node_statuses.forEach(element => {
        console.log(element);
        document.querySelector('#node-cards').insertAdjacentHTML("beforeend", utils.create_card(element));
    });
    document.querySelector('#device-total').innerHTML = node_statuses.length;
}

export async function test_notification(){
    const response = await query.get_node_notification(); //Requests notification data from backend, currently a JSON
    const notification = await response.json(); 

    console.log(notification);
    notification.forEach(element => { //For each element, create a notification card and add it to the dropdown menu
        console.log(element);
        document.querySelector('#notification-dropdown').insertAdjacentHTML("beforeend", utils.createNotification(element));
    });
}