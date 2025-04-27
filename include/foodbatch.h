#ifndef FOODBATCH_H
#define FOODBATCH_H

#include <stdbool.h>

#include "CONSTANTS.h"

/**
 * @struct foodbatch
 * @brief Represents a food batch record in the database
 */
struct foodbatch {
    int batch_id;                 ///< Unique identifier for the batch
    char name[MAX_INPUT];         ///< Name/description of the food batch
    int quantity;                 ///< Quantity of items in the batch
    bool is_perishable;           ///< Whether the batch is perishable
    char expiration_date[11];     ///< Expiration date (ISO YYYY-MM-DD format)
    float daily_consumption_rate; ///< Expected daily consumption rate
};

#endif // FOODBATCH_H