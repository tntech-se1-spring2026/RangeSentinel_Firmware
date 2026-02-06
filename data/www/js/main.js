import * as query from "/modules/query.js";


document.querySelector('#test-button').addEventListener("click", () => {
  query.ping_backend();
  query.get_nodes();
  query.get_node_by_id("node002");
  query.get_node_by_id("node003");
});
