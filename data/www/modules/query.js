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

export function get_node_alert(){
  return http_get(`http://${HOST_ADDRESS}:${HOST_PORT}/web/alerts`);
}
export function get_node_notification(){
  return http_get(`http://${HOST_ADDRESS}:${HOST_PORT}/web/notification`);
}

export function acknowledge_alert(node_id) {
  return http_post(`http://${HOST_ADDRESS}:${HOST_PORT}/web/ack?id=${node_id}`);
}

export function set_wifi_password(newPassword) {
  return http_post(`http://${HOST_ADDRESS}:${HOST_PORT}/web/wifi-password?password=${newPassword}`);
}

export function rename_node(node_id, new_name) {
  return http_post(`http://${HOST_ADDRESS}:${HOST_PORT}/web/rename?id=${node_id}&name=${new_name}`);
}
