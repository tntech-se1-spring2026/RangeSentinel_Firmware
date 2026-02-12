

export function create_card(node_status) {

let bg_color;
let bat_icon;

if (node_status.batt < 4) {
    bg_color = "#ff2e2e";
    bat_icon = "img/battery_empty.svg";
} else {
    bg_color = "#2cdf42";
    bat_icon = "img/battery_full.svg";
}

let trigger;
let trigger_css = "";

if (node_status.door == false) {
    trigger = "Alert";
    trigger_css = "color: red;"
} else {
    trigger = "Normal";
}

return `
<div style="width: 300px; background-color: ${bg_color};" class="card text-center shadow-sm m-3">
    <div class="card-body">
        <img style="width: 50px;" src="img/memory.svg" alt="device"></img>
        <h2>${node_status.name}</h2>
        <h2 class="${trigger_css}">${trigger}</h2>
        <img class="mt-2" style="width: 30px;" src="${bat_icon}" alt="battery level"></img>
    </div>
</div>
`;
}
