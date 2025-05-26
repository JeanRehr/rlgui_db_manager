/**
 * @file clothing.c
 * @brief This is just to implement the lookup tables, there are no method here (yet)
 */

#include "entities/clothing.h"

const char *clothing_type_str[NUM_CLOTHING_TYPE] = {
    "t-shirt", "shirts", "jeans",   "pants", "shorts",    "skirts",   "dresses",    "sweaters", "hoodies",
    "jackets", "coats",  "blazers", "socks", "underwear", "swimwear", "activewear", "pajamas",  "hats",
    "scarves", "gloves", "suits",   "vests", "boots",     "sneakers", "sandals",    "other"
};


const char *clothing_size_str[NUM_CLOTHING_SIZE] = { "xxs", "xs", "s", "m", "l", "xl", "xxl" };


const char *clothing_gender_str[NUM_CLOTHING_GENDER] = { "other", "male", "female" };


const char *clothing_color_str[NUM_CLOTHING_COLOR] = { "black",  "white",        "gray",      "blue", "red",
                                                       "green",  "yellow",       "brown",     "pink", "purple",
                                                       "orange", "multicolored", "patterned", "other" };


const char *clothing_condition_str[NUM_CLOTHING_CONDITION] = { "new", "good", "worn", "needs repair" };