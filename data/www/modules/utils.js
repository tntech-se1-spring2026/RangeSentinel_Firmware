export function create_card(node_status, view_toggle) {
    let battValue = "";
    let sensorIcon = "";
    let bat_icon = "/img/battery_frame_full.svg";

    let cardColour = node_status.alert ? "bg-warning" : "bg-light";
    let openStatus = "";
    let textColour = "";
    let lastSeenText = "";

    node_status.sensors.forEach(sensor => {
        if (sensor.type == "batt") {
            if (sensor.val < 20) {
                bat_icon = "/img/battery_empty.svg";
            }
            battValue = sensor.value + "%";
        }
        if (sensor.type == "door") {
            sensorIcon = sensor.val == false ? "/img/door_open.svg" : "/img/door_front.svg";
            openStatus = sensor.val == false ? "Open" : "Closed";
            textColour = sensor.val == false ? "text-danger" : "text-dark";
        } 
    });

    if (node_status.alert) {
        cardColour = "bg-warning";
    }

    if(node_status.name == "Viewing Node"){
        sensorIcon = "/img/viewing_node.svg";
    } else {
        lastSeenText = (node_status.lastSeen / 1000) + " seconds ago";
    }

    return `
    <div class="col-6 col-md-4 justify-content-center d-flex">
        <!-- Implementation of user-context node card (WITH ALERT)-->
        <div id="${node_status.id}" class="card w-100 text-center ${cardColour}">
            <div class="card-header">
                <h5 class="float-start card-title">${node_status.name}</h5>
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
    if(view_toggle == true){ //Keep == true to keep it from going dev upon loading page
        //Dev card
        return `
        <div class="col-6 col-md-4 justify-content-center d-flex">
            <!-- Implementation of DEV-context node card (WITH ALERT)-->
            <div id="${node_status.id}" class="card w-100 text-center ${cardColour}">
                <div class="card-header">
                    <h1>DEV</h1>
                    <h5 class="float-start card-title">${node_status.name}</h5>
                    <button class="btn btn-link edit-name" type="button"><img class="w-50 h-50 img-fluid float-end ms-3" src="/img/edit.svg" alt="Edit Device Name" /></button>
                    <button class="alert-clear btn btn-sm float-end" style="background-color: #4a4a4a; border: none; color: white;">Clear</button>
                </div>
                <div class="card-body">
                    <h5 class="float-start card-title">ID: ${node_status.id}<br>Mac: ${node_status.mac}</h5>
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
                    <em class="last-seen ms-3" style="display: none;">2 days ago</em>
                </div>
            </div>
        </div>`
    }
    else{
        //Non Dev card
        return `
        <div class="col-6 col-md-4 justify-content-center d-flex">
            <!-- Implementation of user-context node card (WITH ALERT)-->
            <div id="${node_status.id}" class="card w-100 text-center ${cardColour}">
                <div class="card-header">
                    <h1>NOT DEV</h1>
                    <h5 class="float-start card-title">${node_status.name}</h5>
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
                    <em class="last-seen ms-3" style="display: none;">2 days ago</em>
                </div>
            </div>
        </div>`
    }
}

export function createNotification(notif){
    let full_notif = notif.reasons.join(", ")
    return `
    <li><a class="dropdown-item chosenFont" data-node-id="${notif.id}" href="#${notif.id}">${notif.name}: ${full_notif}</a></li>
    `
}