#ifndef RESIDENT_DB_H
#define RESIDENT_DB_H

#include "db_manager.h"

#include <stdbool.h>

#include "resident.h" // to get the definition of a struct resident to populate

int resident_db_create_table(database *db);

int resident_db_insert(
    database *db,
    const char *cpf,
    const char *name,
    int age,
    const char *health_status,
    const char *needs,
    bool medical_assistance,
    int gender
);

// Will not update CPF
int resident_db_update(
    database *db,
    const char *cpf,
    const char *name_input,
    int age_input,
    const char *health_status_input,
    const char *needs_input,
    int medical_assistance_input,
    int gender_input
);

int resident_db_delete_by_cpf(database *db, const char *cpf);

bool resident_db_check_cpf_exists(database *db, const char *cpf);

// Populates the struct resident with the valus from the db if it exists, if not, it exits early without populating
int resident_db_get_by_cpf(database *db, const char *cpf, struct resident *resident);

int resident_db_get_all(database *db);

#endif // RESIDENT_DB_H