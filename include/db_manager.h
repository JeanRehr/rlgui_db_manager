#ifndef DB_MANAGER_H
#define DB_MANAGER_H

#include "person.h"
#include <sqlite3.h>

int db_init();

int db_insert_person(const char *cpf, const char *name, int age, const char *healthStatus, const char *needs,
					 int gender);

int db_update_person(const char *cpf, const char *name_input, int age_input, const char *health_status_input,
					 const char *needs_input, int gender_input);

int db_delete_person_by_cpf(const char *cpf);

bool db_check_cpf_exists(const char *cpf);

bool db_get_person_by_cpf(const char *cpf, struct person *person);

void db_get_all_persons();

int db_insert_food_batch(const char *name, int quantity, bool isPerishable, const char *expirationDate, float dailyConsumptionRate);

void db_get_all_food();

#endif // DB_MANAGER_H