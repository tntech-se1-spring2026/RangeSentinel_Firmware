#ifndef DATABASE_MANAGER_H
#define DATABASE_MANAGER_H

/* ************************IMPORTANT NOTE************************
* Anytime the networkDatabase object is read or written from, you
* must use the meshMutex to lock and unlock. Else you risk both
* cores attempting to simultaneously access it.
* 
* USAGE EXAMPLE:
* if(xSemaphoreTake(meshMutex, portMAX_DELAY)){ // LOCK
* 	// Put database access in place of this line
*	xSemaphoreGive(meshMutex); // UNLOCK
* }
*/

#include "shared_types.h"
#include "hardware_manager.h"

#include <LittleFS.h>

#define MAX_NODES 10
#define MAX_LOG_ENTRIES 50

extern std::array<NodeRecord, MAX_NODES> networkDatabase;
extern std::array<NodeRecord, MAX_LOG_ENTRIES> eventLog;
extern int logHead;
extern bool needsPersistence;// global flag to track if we need to add to persistent memory
extern size_t numNodesInNetwork;
extern SemaphoreHandle_t meshMutex; // Mutex prevents cores from looking at shared data simultaneously

/// @brief function used to add a new node to the database; returns true if it succeeded, else returns false.
/// @param newStatus takes the newNode to append
/// @return returns true if succeeded
bool appendToNetwork(NodeRecord newStatus);

/// @brief this function updates our database with the most current incoming packet
/// @param incoming pass an incoming packet from a known node
/// @param nodeID the nodeID of the node that sent the incoming packet
void updateDatabase(MeshPacket incoming, uint8_t nodeID);

/// @brief  Converts active database into a simplified JSON for alerts for web UI
/// @return JSON string formatted for front end dashboard alerts
String getActiveAlertsAsJson();

/// @brief Converts active database into a simplified JSON string for web UI
/// @return JSON string formatted for the frontend dashboard
String getDatabaseForWeb();

/// @brief converts entire active database to a full JSON string
/// @brief JSON string of complete live view of database
String getDatabaseAsJson();

/// @brief converts entire history log to JSON array
/// @return JSON string of entire log history of data
String getEventLogAsJson();

/// @brief This function saves the database to the LittleFS, allowing database persistence.
bool saveDatabaseToFS();

/// @brief This function gets the database from the LittleFS
void getDatabaseFromFS();

/// @brief Decides if incoming reading constitutes an alert
bool evaluateAlert(const Reading& r);

/// @brief Manually clear the latch for a specific node
bool clearAlertLatch(uint8_t nodeId);

/// @brief Wipes the databases (networkDatabase, eventLog, & LittleFS). This will be used for when the user makes a subtractive change to their network.
void clearAllData();

/// @brief This function returns the index of the node with the given MAC address in the networkDatabase; returns -1 if not found
/// @param mac the MAC address of the node you are searching for
/// @return Returns the index of the node with the given MAC; returns -1 if not found
int findNodeIndexByMAC(uint8_t* mac);

/// @brief Adds a new node to the networkDatabase 
/// @param firstTransmission The first meshPacket transmission of the new node
/// @return returns the nodeID that will be used to assign the new node
uint8_t addNodeToNetworkDatabase(MeshPacket firstTransmission);

/// @brief  Updates a node's name to a custom input
/// @param nodeId The ID of the node to rename
/// @param newName The custom string to assign as the name
/// @return True if successful, False otherwise
bool updateNodeName(uint8_t nodeId, const char* newName);

/// @brief This function updates the viewer node's information in the database
void viewerHeartBeatUpdate();

#endif