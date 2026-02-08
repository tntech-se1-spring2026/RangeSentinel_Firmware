///////////////////////////////////////
//            query.js               //
///////////////////////////////////////
// This file contains logic that handles
// HTTP queries.

const HOST_ADDRESS = "range-sentinel.com";
const HOST_PORT = "3000";

// Generic function to query GET
async function http_get(uri) {
  try {
    const response = await fetch(uri);
    if (!response.ok) {
      throw new Error(`Response status: ${response.status}`);
    }
    return response;
  } catch (error) {
    console.error(error.message);
  }
}

export async function ping_backend() {
  console.log(`Trying to reach backend at ${HOST_ADDRESS}:${HOST_PORT}`)
  return await http_get(`http://${HOST_ADDRESS}:${HOST_PORT}/web/ping`);
}

export async function get_nodes() {
  return await http_get(`http://${HOST_ADDRESS}:${HOST_PORT}/web/nodes`);
}

export async function get_node_by_id(nodeid) {
  return await http_get(`http://${HOST_ADDRESS}:${HOST_PORT}/web/node?id=${nodeid}`);
}
