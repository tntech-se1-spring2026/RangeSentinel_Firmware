#ifndef DATABASE_MANAGER_H
#define DATABASE_MANAGER_H

/* ************************IMPORTANT NOTE************************
* Anytime the networkDatabase object is read or written from, you
* must use the meshMutex to lock and unlock. Else you risk both
* cores attempting to simultaneously access it.
* 
* USAGE EXAMPLE:
* if (xSemaphoreTake(meshMutex, portMAX_DELAY)) { // LOCK
* 	// Put database access in place of this line
*	xSemaphoreGive(meshMutex); // UNLOCK
* }
*/

#include <array>
#include <LittleFS.h>
#include "shared_types.h"

#define MAX_NODES 10
#define MAX_LOG_ENTRIES 50
// officially declared in main.cpp
extern std::array<NodeStatus, MAX_NODES> networkDatabase;
extern std::array<NodeStatus, MAX_LOG_ENTRIES> eventLog;
extern int logHead;
extern bool needsPersistence;

std::array<NodeStatus, MAX_NODES> networkDatabase = {};
size_t numNodesInNetwork = 0;

// function used to add a new node to the database; returns true if it succeeded, else returns false.
bool appendToNetwork(NodeStatus newStatus){
    if(numNodesInNetwork < MAX_NODES){
        networkDatabase[numNodesInNetwork] = newStatus;
        numNodesInNetwork++;
        return true;
    }else{
        return false;
    }
}

// global flag to track if we need to add to persistent memory
bool needsPersistence = false;

// Mutex prevents cores from looking at shared data simultaneously
SemaphoreHandle_t meshMutex;

// helper function to update database
void updateDatabase(NodeStatus incoming);

// converts entire active database to JSON array
String getDatabaseAsJson();

// converts entire history log to JSON array
String getEventLogAsJson();

// saves database to LittleFS
bool saveDatabaseToFS();

// get database from LittleFS
void getDatabaseFromFS();

// development function to wipe db and logs to start fresh
// CAN'T THINK OF A USE CASE TO BE IN FINAL PRODUCT
void clearAllData();
#endif
