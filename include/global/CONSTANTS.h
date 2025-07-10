/**
 * @file constants.h
 * @brief Global Application Constants
 *
 * Defines application-wide constants used for:
 * - Input buffer sizes
 * - UI configuration
 * - Data formatting standards
 */

#ifndef CONSTANTS_H
#define CONSTANTS_H

/**
  * @def MAX_INPUT
  * @brief Maximum length for text input buffers
  *
  * Defines the maximum number of characters (including null terminator)
  * for all text input fields in the application.
  *
  * @note Used for char arrays storing user input
  * @warning Does not include null terminator in count, 256 should be the null
  */
#define MAX_INPUT 256

/**
  * @def MAX_CPF_LENGTH
  * @brief Storage size for CPF numbers
  *
  * Specifies the character array size needed to store a CPF number
  * including its null terminator (11 digits + null).
  *
  * @note Format: "XXXXXXXXXXX\\0" (11 digits)
  * @see resident_db.h and resident.h
  */
#define MAX_CPF_LENGTH 12

/**
  * @def MAX_PHONE_NUMBER_LEN
  * @brief Storage size for phone numbers
  *
  * Specifies the character array size needed to store a phone number
  * including its null terminator (13 digits + null).
  *
  * @note Format: "5551999999999\\0" (13 digits)
  */
#define MAX_PHONE_NUMBER_LEN 14

/**
  * @def FONT_SIZE
  * @brief Base font size for UI elements
  *
  * Defines the standard font size (in pixels) to be used consistently
  * throughout all application screens.
  *
  * @note All UI elements should derive their text sizes from this value
  * @warning Changing this may require layout adjustments
  */
#define FONT_SIZE 16

/**
  * @def DATE_LEN
  * @brief Length of date for use in all date character arrays
  *
  * Defines the length of date to be used consistently
  * throughout all application screens.
  *
  * @note Should be ISO 8601 formatted date (YYYY-MM-DD + null)
  * 
  * @example 2025-06-02
  *
  */
#define DATE_LEN 11

/**
  * @def DATETIME_LEN
  * @brief Length of date for use in all datetime character arrays
  *
  * Defines the length of date and time to be used consistently
  * throughout all application screens.
  *
  * @note Should be ISO 8601 formatted datetime (YYYY-MM-DD HH:MM:SS + null)
  * 
  * @example 2025-06-02 15:04:53
  * 
  */
#define DATETIME_LEN 20

#endif // CONSTANTS_H
