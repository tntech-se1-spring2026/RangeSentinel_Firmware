export function create_card(nodeInfo, dev_view) {
    let battValue = "";
    let sensorIcon = "";
    let bat_icon = "/img/battery_frame_full.svg";

    let cardColour = nodeInfo.alert ? "bg-warning" : "bg-light";
    let openStatus = "";
    let textColour = "";
    let lastSeenText = "";
    let removeHTML = "";
    let devHTML = "";

    nodeInfo.sensors.forEach(sensor => {
        if (sensor.type == "batt" || sensor.type == "battery") {
            if (sensor.val < 20) {
                bat_icon = "/img/battery_empty.svg";
            }
            battValue = sensor.value + "%";
        }
        if (sensor.type == "door") {
            sensorIcon = sensor.val == "Open" ? "/img/door_open.svg" : "/img/door_front.svg";
            openStatus = sensor.val == "Open" ? "Open" : "Closed";
            textColour = sensor.val == "Open" ? "text-danger" : "text-dark";
        } 
    });

    if(nodeInfo.type == "viewing"){
        sensorIcon = "/img/viewing_node.svg";
        openStatus = "Viewer";
        textColour = "text-dark";
    } else {
    }

        if (dev_view) {
            devHTML = `
                <div class="card-footer">
                    <h5>Node ID: ${nodeInfo.id}</h5>
                    <h5>MAC: ${nodeInfo.mac}</h5>
                </div>
                `
        }

        // Calculate Last Seen Text
        if (nodeInfo.status = "Online") {
            lastSeenText = Math.round((nodeInfo.timeAgo / 1000)) + " seconds ago";
        } else {
            lastSeenText = nodeInfo.status;
        }

        // Add remove button
        removeHTML = `
            <button class="btn btn-link delete-node" type="button"><img class="" height="auto" src="img/delete.svg" alt="Delete" aria-label="Delete" /></button>
        `;
        
        // return final HTML
        return `
        <div class="col-6 col-md-4 justify-content-center d-flex">
            <!-- Implementation of user-context node card (WITH ALERT)-->
            <div id="${nodeInfo.id}" class="card w-100 text-center ${cardColour} ${nodeInfo.id}">
                <div class="card-header d-flex align-items-center justify-content-between flex-column flex-xl-row">
                    <span class="d-flex align-items-center justify-content-start">
                        <h5 class="card-title" style="margin-bottom: 0px;">${nodeInfo.name}</h5>
                        <button class="btn btn-link edit-name" type="button"><img class="" src="/img/edit.svg" alt="Edit Device Name" /></button>
                    </span>
                    <span class="d-flex align-items-center justify-content-end">
                        ${removeHTML}
                        <button class="alert-clear btn btn-sm" style="background-color: #4a4a4a; border: none; color: white;">Clear</button>
                    </span>
                </div>
                <div class="card-body">
                    <img width="40em" src="${sensorIcon}" alt="Sensor Icon" />
                    <hr class="mb-4" />
                    <h3 class="${textColour}">${openStatus}</h3>
                </div>
                <div class="card-footer text-muted d-flex align-items-center justify-content-between">
                    <span class="d-flex align-items-center">
                        <img style="color: black" src="${bat_icon}" alt="Battery Level:" />
                        <em class="ms-1">${battValue}</em>
                    </span>
                    <em class="last-seen ms-3">${lastSeenText}</em>
                </div>
                ${devHTML}
            </div>
        </div>`
}

export function createNotification(notif){
    return `
    <li><a class="dropdown-itemviewing data-node-id="${notif.id}" href="#${notif.id}">${notif.name}: ${notif.reasons} </a></li>
    `
}