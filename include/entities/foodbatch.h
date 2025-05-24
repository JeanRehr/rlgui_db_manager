/**
 * @file foodbatch.h
 * @brief Foodbatch definition for use in database operations/code
 */
#ifndef FOODBATCH_H
#define FOODBATCH_H

#include <stdbool.h>

#include "global/CONSTANTS.h"

/**
 * @struct foodbatch
 * @brief Represents a food batch record in the database
 */
struct foodbatch {
    int batch_id;                 ///< Unique identifier for the batch
    char name[MAX_INPUT];         ///< Name/description of the food batch
    int quantity;                 ///< Quantity of items in the batch
    bool is_perishable;           ///< Whether the batch is perishable
    char expiration_date[11];     ///< ISO 8601 formatted date (YYYY-MM-DD + null)
    float daily_consumption_rate; ///< Expected daily consumption rate
};

#endif // FOODBATCH_H
