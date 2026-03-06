export function create_card(nodeInfo, dev_view) {
    let battValue = "";
    let sensorIcon = "";
    let bat_icon = "/img/battery_frame_full.svg";

    let cardColour = nodeInfo.alert ? "bg-warning" : "bg-light";
    let openStatus = "";
    let textColour = "";
    let lastSeenText = "";

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
        openStatus = "Viewing Node";
        textColour = "text-dark";
    } else {
        lastSeenText = (nodeInfo.lastSeen / 1000) + " seconds ago";
    }
    if(dev_view == true){
        //Dev card
        return `
        <div class="col-6 col-md-4 justify-content-center d-flex">
            <!-- Implementation of DEV-context node card (WITH ALERT)-->
            <div id="${nodeInfo.id}" class="card w-100 text-center ${cardColour}${nodeInfo.id}">
                <div class="card-header">
                    <h1>DEV</h1>
                    <h5 class="float-start card-title">${nodeInfo.name}</h5>
                    <button class="btn btn-link edit-name" type="button"><img class="w-50 h-50 img-fluid float-end ms-3" src="/img/edit.svg" alt="Edit Device Name" /></button>
                    <button class="alert-clear btn btn-sm float-end" style="background-color: #4a4a4a; border: none; color: white;">Clear</button>
                </div>
                <div class="card-body">
                    <h5 class="float-start card-title">ID: ${nodeInfo.id}<br>Mac: ${nodeInfo.mac}</h5>
                </div>
                <div class="card-body">
                    <img width="40em" src="${sensorIcon}" alt="Sensor Icon" />
                    <hr class="mb-4" />
                    <h3 class="${textColour}">${openStatus}</h3>
                </div>
                <div class="card-footer text-muted d-flex align-items-center">
                    <span>
                        <img style="color: black" src="${bat_icon}" alt="Battery Level:" />
                        <em class="ms-1">${battValue}</em>
                    </span>
                    <em class="last-seen ms-3" style="display: none;">${lastSeenText}</em>
                </div>
            </div>
        </div>`
    }
    else{
        return `
        <div class="col-6 col-md-4 justify-content-center d-flex">
            <!-- Implementation of user-context node card (WITH ALERT)-->
            <div id="${nodeInfo.id}" class="card w-100 text-center ${cardColour} ${nodeInfo.id}">
                <div class="card-header">
                    <h5 class="float-start card-title">${nodeInfo.name}</h5>
                    <button class="btn btn-link edit-name" type="button"><img class="w-50 h-50 img-fluid float-end ms-3" src="/img/edit.svg" alt="Edit Device Name" /></button>
                    <button class="alert-clear btn btn-sm float-end" style="background-color: #4a4a4a; border: none; color: white;">Clear</button>
                </div>
                <div class="card-body">
                    <img width="40em" src="${sensorIcon}" alt="Sensor Icon" />
                    <hr class="mb-4" />
                    <h3 class="${textColour}">${openStatus}</h3>
                </div>
                <div class="card-footer text-muted d-flex align-items-center">
                    <span>
                        <img style="color: black" src="${bat_icon}" alt="Battery Level:" />
                        <em class="ms-1">${battValue}</em>
                    </span>
                    <em class="last-seen ms-3">${lastSeenText}</em>
                </div>
            </div>
        </div>`
    }
}

export function createNotification(notif){
    return `
    <li><a class="dropdown-itemviewing data-node-id="${notif.id}" href="#${notif.id}">${notif.name}: ${notif.reasons} </a></li>
    `
}