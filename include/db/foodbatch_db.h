#ifndef FOOBATCH_DB_H
#define FOOBATCH_DB_H

#include "db_manager.h"

#include <stdbool.h>

#include "food.h" // to get the definition of a struct foodbatch to populate

int foodbatch_db_create_table(database *db);

int foodbatch_db_insert(database *db, int batch_id, const char *name, int quantity, bool isPerishable,
						const char *expirationDate, float dailyConsumptionRate);

// Doesn't update BatchId
int foodbatch_db_update(database *db, int batch_id, const char *name_input, int quantity_input,
						bool is_perishable_input, const char *expiration_date_input,
						float daily_consumption_rate_input);

int foodbatch_db_delete_by_id(database *db, int batch_id);

// Populates the struct foodbatch with the valus from the db if it exists, if not, it exits early without populating
int foodbatch_db_get_by_batchid(database *db, int batch_id, struct foodbatch *foodbatch);

bool foodbatch_db_check_batchid_exists(database *db, int batch_id);

int foodbatch_db_get_all(database *db);

#endif // FOOBATCH_DB_H