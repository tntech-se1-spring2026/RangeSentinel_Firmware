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
    <div class="col-md-4">
        <div style="width: 100%; background-color: ${bg_color};" class="card text-center shadow-sm m-3">
            <div class="card-body">
                <img style="width: 50px;" src="img/memory.svg" alt="device"></img>
                <h2>${node_status.name}</h2>
                <h2 style="${trigger_css}">${trigger}</h2>
                <img class="mt-2" style="width: 30px;" src="${bat_icon}" alt="device"></img>
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