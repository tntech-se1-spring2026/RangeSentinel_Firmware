#ifndef DATABASE_MANAGER_H
#define DATABASE_MANAGER_H

#include <array>
#include "shared_types.h"

#define MAX_NODES 10
#define MAX_LOG_ENTRIES 50

// officially declared in main.cpp
extern std::array<NodeRecord, MAX_NODES> networkDatabase;
extern std::array<NodeRecord, MAX_LOG_ENTRIES> eventLog;
extern int logHead;
extern bool needsPersistence;

// helper function to update database
void updateDatabase(MeshPacket incoming);

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
