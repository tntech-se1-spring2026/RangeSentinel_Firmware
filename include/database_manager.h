#ifndef DATABASE_MANAGER_H
#define DATABASE_MANAGER_H

#include <array>
#include "shared_types.h"

#define MAX_NODES 10

// helper function to update database
void updateDatabase(NodeStatus incoming);

// converts entire active database to JSON array
String getDatabaseAsJson();

#endif
