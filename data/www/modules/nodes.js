import * as query from "/modules/query.js";
import * as utils from "/modules/utils.js";

export async function update(dev_view) { //Gets node data from json and calls create_card for each node
    const response = await query.get_nodes();
    const node_statuses = await response.json();

    const container = document.querySelector('#node-cards');

    let html = ""; //Initialize an empty string to hold the generated HTML

    node_statuses.forEach(element => {
        html += utils.create_card(element, dev_view);//Append the generated card HTML to the overall HTML string
    //This way, we only update the DOM once after processing all nodes, which is more efficient than updating it for each node
    });
    container.innerHTML = html; //Insert the generated HTML into the container, replacing any existing content
}

export async function addAlert(){
    const response = await query.get_node_alert();
    const alert = await response.json();
    let html = ""; //Initialize an empty string to hold the generated HTML

    let alertCount = 0;

    for (const element of alert) {
        let lowBattery = false;

        console.log(element);

        // See if low battery is a reason
        element.reasons.forEach(reason => {
            console.log(reason);
            if (reason == "Low Battery" && element.reasons.length == 1) {
                lowBattery = true;
                console.log("Low battery is detected");
            }
        });

        if (element.mac = "F0:24:F9:93:C9:AC" && lowBattery == true) {
            continue;
        }

        console.log(element);
        html += utils.createNotification(element); //Append the generated notification HTML to the overall HTML string
        const card = document.getElementById(element.id);
        if (card) {
            card.classList.remove("bg-light");
            card.classList.add("bg-warning");
        }

        alertCount += 1;
    }

    document.querySelector('#notification-dropdown').innerHTML = html;
    updateNotificationCount(alertCount);
    
} //This function follows the same process that update() does
export function updateNotificationCount(count) { // Changes the count of notifications
    const badge = document.getElementById("notif-count");

    if (count > 0) {
        badge.textContent = count;
        badge.style.display = "inline-block";
    } else {
        badge.style.display = "none";
    }
}
