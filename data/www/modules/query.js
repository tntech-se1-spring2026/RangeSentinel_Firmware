///////////////////////////////////////
//            query.js               //
///////////////////////////////////////
// This file contains logic that handles
// HTTP queries.

const HOST_ADDRESS = "range-sentinel.com";
const HOST_PORT = "80";

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

// Generic HTTP POST
async function http_post(uri) {
  try {
    const response = await fetch(uri, {
      method: "POST"
    });
    if (!response.ok) {
      throw new Error(`Response status: ${response.status}`);
    }
    return response;
  } catch (error) {
    console.error(error.message);
  }
}

export function ping_backend() {
  console.log(`Trying to reach backend at ${HOST_ADDRESS}:${HOST_PORT}`)
  return http_get(`http://${HOST_ADDRESS}:${HOST_PORT}/web/ping`);
}

export function get_nodes() {
  return http_get(`http://${HOST_ADDRESS}:${HOST_PORT}/web/nodes`);
}

export function get_node_by_id(nodeid) {
  return http_get(`http://${HOST_ADDRESS}:${HOST_PORT}/web/node?id=${nodeid}`);
}

export function get_node_notification(){
  return http_get(`http://${HOST_ADDRESS}:${HOST_PORT}/web/notification`);
}

export function set_wifi_password(newPassword) {
  return http_post(`http://${HOST_ADDRESS}:${HOST_PORT}/web/wifi-password?password=${newPassword}`);
}
