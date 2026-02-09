import * as query from "/modules/query.js";
//template 

const test_template = `
    <p> Hello World </p>
`
function card_creator(id, statas){
    return    `
    <div class="col-12 col-md-4">
        <div class="card text-center shadow-sm">
            <div class="card-body">
                <h5 class="card-title">${id}</h5>
                <p class="card-value text-success">${statas}</p>
                <p class="card-text text-muted"></p>
            </div>
        </div>
    </div>`;

}
let count = 1;
const cardRow = document.getElementById("cardRow");
document.querySelector('#test-button').addEventListener("click",async () => {
    cardRow.insertAdjacentHTML(
        "beforeend",
        card_creator(`Node ${count}`,"Online")
    )
    count++;
});
/*document.querySelector('#test-button').addEventListener("click",async () =>
{
    const nodes_data_promise = await query.get_nodes();
    const node_data = await nodes_data_promise.json();
    for (const iter in node_data["nodes"]){
        console.log(node_data["nodes"][iter]);
        let id = node_data["nodes"][iter]["id"];
        let statas = check_time(parseInt(node_data["nodes"][iter]["last_seen"]));
        let card = card_creator(id,statas);
        document.querySelector('#nodes-container').innerHTML += card;
    }   
});*/





/*
For each (node in node_list){
    id_name = node.id
    statas =node.status
}
*/
//Fetch data

const nodes_data_promise = await query.get_nodes();

const node_data = await nodes_data_promise.json();
console.log(node_data);
for (const iter in node_data["nodes"]){
    console.log(node_data["nodes"][iter]);
    let id_name =node_data["nodes"][iter]["id"];
    let statas = node_data["nodes"][iter]["status"];
    let time_frame = check_time(node_data["nodes"][iter]["last_seen"]);
}

//Interpret data
function check_time(last_seen){//last_seen integer
    const present_time = Date.now();
    const remain_time = present_time - last_seen;
    if (remain_time >= 1800000){ // Greater than 30 minutes
        return "Offline";
    }
    else{
        return "Online";
    }
};
//Creating node cards

//