#ifndef FOOD_H
#define FOOD_H

#include "CONSTANTS.h"

// Food struct must be in batches as to set the expiration date for one batch of all items.
// if the same item (milk) enters the database again, it will hjave a different exp date
// and the same name, so batch id must be a unique identifier
struct foodbatch {
	int batch_id; // Unique identifier for the batch
	char name[MAX_INPUT];
	int quantity; // Quantity available in this batch
	char expiration_date[11]; // Expiration date for this batch in ISO format
	bool is_perishable;
	float daily_consumption_rate; // Daily consumption rate (if stable per batch)
};

#endif // FOOD_H