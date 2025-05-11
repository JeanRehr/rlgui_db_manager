#ifndef RESIDENT_H
#define RESIDENT_H

#include <stdbool.h>

#include "CONSTANTS.h"

/**
 * @enum gender
 * @brief Gender enumeration for resident records
 * 
 * This enumeration defines standard gender options for resident records.
 */
enum gender {
    GENDER_OTHER = 0, ///< Non-binary/other gender identification
    GENDER_MALE,      ///< Male gender identification
    GENDER_FEMALE     ///< Female gender identification
};

/**
 * @struct resident
 * @brief Represents a resident record in the database
 */
struct resident {
    char cpf[MAX_CPF_LENGTH];      ///< Resident's CPF (Brazilian ID number)
    char name[MAX_INPUT];          ///< Resident's full name
    int age;                       ///< Resident's age
    char health_status[MAX_INPUT]; ///< Description of health status
    char needs[MAX_INPUT];         ///< Special needs or requirements
    bool medical_assistance;       ///< Whether medical assistance is required
    enum gender gender;            ///< Gender (0=Other, 1=Male, 2=Female)
    char entry_date[11];           ///< ISO 8601 formatted date (YYYY-MM-DD + null)
};

#endif // RESIDENT_H
