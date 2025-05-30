/**
 * @file supply.h
 * @brief Supply definition for use in database operations/code
 */
#ifndef SUPPLY_H
#define SUPPLY_H

#include "global/CONSTANTS.h"

/** 
 * @struct supply
 *
 * @brief Represents a supply record in the database
 *
 */
struct supply {
    char name[MAX_INPUT];         ///< e.g. "diaper", "tampon"
    char category[MAX_INPUT]; ///< e.g. "hygiene", "cleaning", "personal care"
    char size[MAX_INPUT];         ///< e.g. "adult", "small", "xxl"
    char unit[MAX_INPUT];     ///< e.g. "piece", "pack", "box"
    int stock;                    ///< Current count in inventory
    char notes[MAX_INPUT];        ///< General notes if needed
};

#endif // SUPPLY_H
