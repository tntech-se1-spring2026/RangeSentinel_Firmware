export function create_card(node_status) {
    let bg_color;
    let bat_icon;

    if (node_status.sensors[1].val < 3) {
        bg_color = "#ff2e2e";
        bat_icon = "img/battery-empty.svg";
    } else {
        bg_color = "#2cdf42";
        bat_icon = "img/battery-full.svg";
    }

    let trigger;
    let trigger_css = "";
    if (node_status.door) {
        trigger = "Alert"
        trigger_css = "color: red;"
    } else {
        trigger = "Normal"
    }
    return `
    <div class="col-md-4 justify-content-center d-flex">
            <!-- Implementation of user-context node card -->
            <div class="card w-auto text-center me-3">
                <div class="card-header">
                    <h5 class="float-start">${node_status.name}</h5>
                    <img class="w-25 h-25 img-fluid float-end ms-3" src="/img/edit.svg" alt="Edit Device Name" />
                    <p class="chosenFont id="${node_status.id}" style="display: none;">${node_status.id}</p> <!-- hidden element to store node ID for later use -->
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
                    <em class="last-seen ms-3 mt-2">2 days ago</em>
                </div>
            </div>

            <!-- Implementation of user-context node card (WITH ALERT)-->
            <div id="${node_status.id}" class="card w-auto text-center me-3 bg-warning">
                <div class="card-header">
                    <h5 class="float-start">${node_status.name}</h5>
                    <img class="w-25 h-25 img-fluid float-end ms-3" src="/img/edit.svg" alt="Edit Device Name" />
                    <p class="chosenFont" style="display: none;">${node_status.id}</p> <!-- hidden element to store node ID for later use -->
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
                    <em class="last-seen ms-3 mt-2">2 days ago</em>
                </div>
            </div>
    </div>
    `
}

export function createNotification(notification){
    return `
    <li><a class="dropdown-item chosenFont" href="#">${notification.alert}</a></li>
    `
}