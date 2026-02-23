export function create_card(node_status) {
    let bg_color;
    let bat_icon;

    if (node_status.sensors[1].val < 3) {
        bg_color = "#ff2e2e";
        bat_icon = "img/battery_empty.svg";
    } else {
        bg_color = "#2cdf42";
        bat_icon = "img/battery_frame_full.svg";
    }

    if (node_status.alert) {
        return `
        <div class="col-6 col-md-4 d-flex">
            <!-- Implementation of user-context node card (WITH ALERT)-->
            <div id="${node_status.id}" class="card w-100 text-center bg-warning">
                <div class="card-header">
                    <h5 class="float-start">${node_status.name}</h5>
                    <img class="w-25 h-25 img-fluid float-end ms-3" src="/img/edit.svg" alt="Edit Device Name" />
                    <button class="alert-clear btn btn-sm float-end" style="background-color: #4a4a4a; border: none; color: white;">Clear</button>
                </div>
                <div class="card-body">
                    <img width="40em" src="/img/door_open.svg" alt="Sensor Type: Gate" />
                    <hr class="mb-4" />
                    <h3 class="text-danger">Open</h3>
                </div>
                <div class="card-footer text-muted d-flex align-items-center">
                    <span>
                        <img style="color: black" src="${bat_icon}" alt="Battery Level:" />
                        <em class="ms-1">97%</em>
                    </span>
                    <em class="last-seen ms-3">2 days ago</em>
                </div>
            </div>
        </div>`
    } else {
        return `
        <div class="col-6 col-md-4 justify-content-center d-flex">
            <!-- Implementation of user-context node card -->
            <div id="${node_status.id}" class="card w-100 text-center">
                <div class="card-header">
                    <h5 class="float-start">${node_status.name}</h5>
                    <img class="w-25 h-25 img-fluid float-end ms-3" src="/img/edit.svg" alt="Edit Device Name" />
                    <button class="alert-clear btn btn-sm float-end" style="background-color: #4a4a4a; border: none; color: white;">Clear</button>
                </div>
                <div class="card-body">
                    <img width="40em" src="/img/door_front.svg" alt="Sensor Type: Gate" />
                    <hr class="mb-4" />
                    <h3>Closed</h3>
                </div>
                <div class="card-footer text-muted d-flex align-items-center">
                    <span>
                        <img style="color: black" src="${bat_icon}" alt="Battery Level:" />
                        <em class="ms-1">97%</em>
                    </span>
                    <em class="last-seen ms-3">2 days ago</em>
                </div>
            </div>
        </div>`
    }
    
}

export function createNotification(notif){
    return `
    <li><a class="dropdown-item chosenFont" data-node-id="${notif.id}" href="#${notif.id}">${notif.alert}</a></li>
    `
}