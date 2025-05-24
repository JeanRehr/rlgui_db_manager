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
};

/**
 * @enum clothing_size
 * @brief Clothing size enumeration
 * 
 * This enumeration defines clothing size options.
 */
enum clothing_size {
    XXS = 0,
    XS,
    S,
    M,
    L,
    XL,
    XXL,
};

/**
 * @enum clothing_gender
 * @brief Clothing gender enumeration
 * 
 * This enumeration defines clothing gender options.
 */
enum clothing_gender {
    UNISSEX = 0,
    MALE,
    FEMALE,
};

/**
 * @enum clothing_color
 * @brief Clothing color enumeration
 * 
 * This enumeration defines clothing color options.
 * 
 * @note As this is a shelter management system, granularity or well-defined colors are really
 *       not necessary, should not be important
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
};

/**
 * @enum clothing_color
 * @brief Clothing color enumeration
 * 
 * This enumeration defines clothing color options.
 * 
 * @note As this is a shelter management system, granularity or well-defined colors are really
 *       not necessary, should not be important
 */
enum clothing_condition {
    NEW = 0,
    GOOD,
    WORN,
    NEEDS_REPAIR,
};

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
