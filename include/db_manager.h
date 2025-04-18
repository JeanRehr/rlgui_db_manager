#ifndef DB_MANAGER_H
#define DB_MANAGER_H

#include <sqlite3.h>

#include "food.h"
#include "resident.h"

// Will initialize all needed databases at the same time
int db_init();

// Also inserts EntryDate, the current time of the system
int db_insert_resident(const char *cpf, const char *name, int age, const char *health_status, const char *needs,
					   bool medical_assistance, int gender);

// Will not update CPF or EntryDate
int db_update_resident(const char *cpf, const char *name_input, int age_input, const char *health_status_input,
					   const char *needs_input, int medical_assistance_input, int gender_input);

int db_delete_resident_by_cpf(const char *cpf);

bool db_check_cpf_exists(const char *cpf);

// Populates the struct resident with the valus from the db if it exists, if not, it exits early without populating
bool db_get_resident_by_cpf(const char *cpf, struct resident *resident);

void db_get_all_residents();

int db_insert_food_batch(int batch_id, const char *name, int quantity, bool isPerishable, const char *expirationDate,
						 float dailyConsumptionRate);

// Doesn't update BatchId
int db_update_food_batch(int batch_id, const char *name_input, int quantity_input, bool is_perishable_input,
						 const char *expiration_date_input, float daily_consumption_rate_input);

int db_delete_foodbatch_by_id(int batch_id);

// Populates the struct foodbatch with the valus from the db if it exists, if not, it exits early without populating
bool db_get_food_by_batchid(int batch_id, struct foodbatch *foodbatch);

bool db_check_batchid_exists(int batch_id);

void db_get_all_food();

#endif // DB_MANAGER_H