/**
 * @file clothing.h
 * @brief Clothing definition for use in database operations/code
 */
#ifndef CLOTHING_H
#define CLOTHING_H

#include <stdbool.h>

#include "global/CONSTANTS.h"

/* Automated enum to string type conversion using macros */
/* List is not too big to maintaing it manually
#define CLOTHING_TYPE_LIST \
    X(T_SHIRT) \
    X(SHIRTS) X(JEANS) X(PANTS) X(SHORTS) X(SKIRTS) X(DRESSES) X(SWEATERS) X(HOODIES) X(JACKETS) X(COATS) X(BLAZERS) \
        X(SOCKS) X(UNDERWEAR) X(SWIMWEAR) X(ACTIVEWEAR) X(PAJAMAS) X(HATS) X(SCARVES) X(GLOVES) X(SUITS) X(VESTS) \
            X(BOOTS) X(SNEAKERS) X(SANDALS) X(OTHER)

enum clothing_type {
#define X(name) name,
    CLOTHING_TYPE_LIST
#undef X
};

const char *clothing_type_str =
#define X(name) #name ";"
    CLOTHING_TYPE_LIST
#undef X
    "";
*/

/**
 * @enum clothing_type
 * @brief Clothing type enumeration
 * 
 * This enumeration defines clothing type options.
 * 
 * @warning Terminating NUM must always be last to maximize compile-time access safety
 */
enum clothing_type {
    TSHIRT = 0,
    SHIRTS,
    JEANS,
    PANTS,
    SHORTS,
    SKIRTS,
    DRESSES,
    SWEATERS,
    HOODIES,
    JACKETS,
    COATS,
    BLAZERS,
    SOCKS,
    UNDERWEAR,
    SWIMWEAR,
    ACTIVEWEAR, // (e.g., leggings, sports bras)
    PAJAMAS,
    HATS,
    SCARVES,
    GLOVES,
    SUITS,
    VESTS,
    BOOTS,
    SNEAKERS,
    SANDALS,
    OTHER_TYPE,
    NUM_CLOTHING_TYPE
};

/**
 * @enum clothing_size
 * @brief Clothing size enumeration
 * 
 * This enumeration defines clothing size options.
 * @warning Terminating NUM must always be last to maximize compile-time access safety
 */
enum clothing_size { XXS = 0, XS, S, M, L, XL, XXL, NUM_CLOTHING_SIZE };

/**
 * @enum clothing_gender
 * @brief Clothing gender enumeration
 * 
 * This enumeration defines clothing gender options.
 * 
 * @warning Terminating NUM must always be last to maximize compile-time access safety
 */
enum clothing_gender { UNISSEX = 0, MALE, FEMALE, NUM_CLOTHING_GENDER };

/**
 * @enum clothing_color
 * @brief Clothing color enumeration
 * 
 * This enumeration defines clothing color options.
 * 
 * @note As this is a shelter management system, granularity or well-defined colors are really
 *       not necessary, should not be important
 * 
 * @warning Terminating NUM must always be last to maximize compile-time access safety
 */
enum clothing_color {
    CL_BLACK = 0,
    CL_WHITE,
    CL_GRAY,
    CL_BLUE,
    CL_RED,
    CL_GREEN,
    CL_YELLOW,
    CL_BROWN,
    CL_PINK,
    CL_PURPLE,
    CL_ORANGE,
    CL_MULTICOLORED,
    CL_PATTERNED,
    CL_OTHER_COLOR,
    NUM_CLOTHING_COLOR
};

/**
 * @enum clothing_color
 * @brief Clothing color enumeration
 * 
 * This enumeration defines clothing color options.
 * 
 * @note As this is a shelter management system, granularity or well-defined colors are really
 *       not necessary, should not be important
 *
 * @warning Terminating NUM must always be last to maximize compile-time access safety
 */
enum clothing_condition { NEW = 0, GOOD, WORN, NEEDS_REPAIR, NUM_CLOTHING_CONDITION };

/**
 * @addtogroup clothing_lookup_table
 * 
 * @details If adding a string here, add it in the enum in the same order
 * 
 * @{
 */

/**
 * @brief Clothing type lookup
 */
static const char *clothing_type_str[NUM_CLOTHING_TYPE] = {
    "t-shirt", "shirts", "jeans",   "pants", "shorts",    "skirts",   "dresses",    "sweaters", "hoodies",
    "jackets", "coats",  "blazers", "socks", "underwear", "swimwear", "activewear", "pajamas",  "hats",
    "scarves", "gloves", "suits",   "vests", "boots",     "sneakers", "sandals",    "other"
};

/**
 * @brief Clothing type lookup
 */
static const char *clothing_size_str[NUM_CLOTHING_SIZE] = { "xxs", "xs", "s", "m", "l", "xl", "xxl" };

/**
 * @brief Clothing gender lookup
 */
static const char *clothing_gender_str[NUM_CLOTHING_GENDER] = { "other", "male", "female" };

/**
 * @brief Clothing color lookup
 */
static const char *clothing_color_str[NUM_CLOTHING_COLOR] = { "black",  "white",        "gray",      "blue", "red",
                                                              "green",  "yellow",       "brown",     "pink", "purple",
                                                              "orange", "multicolored", "patterned", "other" };

/**
 * @brief Clothing condition lookup
 */
const char *clothing_condition_str[NUM_CLOTHING_CONDITION] = { "new", "good", "worn", "needs repair" };

/** @} */

/**
 * @struct clothing
 * @brief Represents a pieace of clothing record in the database
 */
struct clothing {
    enum clothing_type type;           ///< Clothing type
    enum clothing_size size;           ///< Clothing size
    enum clothing_gender gender;       ///< Clothing gender
    enum clothing_color color;         ///< Clothing color
    enum clothing_condition condition; ///< Clothing condition
    int quanity;                       ///< Stock quantity
    const char *notes;                 ///< General notes if needed
};

#endif // CLOTHING_H
