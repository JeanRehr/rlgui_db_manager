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
 * 
 * @brief Represents a food batch record in the database
 * 
 */
struct foodbatch {
    int batch_id;                   ///< Unique identifier for the batch
    char name[MAX_INPUT];           ///< Name/description of the food batch
    float quantity;                 ///< For both weight and count
    char unit[MAX_INPUT];           ///< e.g. "L", "KG", "Piece", "cans", "lbs"
    bool is_perishable;             ///< Whether the batch is perishable
    char arrival_date[DATE_LEN];    ///< ISO 8601 formatted date (YYYY-MM-DD + null)
    char expiration_date[DATE_LEN]; ///< ISO 8601 formatted date (YYYY-MM-DD + null)
};

#endif // FOODBATCH_H
