

export function create_card(node_status) {

let bg_color;
let bat_icon;

if (node_status.batt == "0") {
    bg_color = "#ff2e2e";
    bat_icon = "img/battery_empty.svg";
} else {
    bg_color = "#2cdf42";
    bat_icon = "img/battery_full.svg";
}

return `
<div style="width: 300px; background-color: ${bg_color};" class="card text-center shadow-sm m-3">
    <div class="card-body">
        <img style="width: 50px;" src="img/memory.svg" alt="device"></img>
        <h2>${node_status.name}</h2>
        <img style="width: 30px;" src="${bat_icon}" alt="battery level"></img>
    </div>
</div>
`;
}
