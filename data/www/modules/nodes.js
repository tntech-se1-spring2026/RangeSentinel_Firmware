import * as query from "/modules/query.js";
import * as utils from "/modules/utils.js"

export async function update(){
    const response = await query.get_nodes();
    const node_statuses = await response.json();

    console.log(node_statuses);
    node_statuses.forEach(element => {
        console.log(element);
        document.querySelector('#node-cards').insertAdjacentHTML("beforeend", utils.create_card(element));
    });

    document.querySelector('#device-total').innerHTML = node_statuses.length;
};