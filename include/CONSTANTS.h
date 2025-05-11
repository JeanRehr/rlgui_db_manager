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
 
 #endif // CONSTANTS_H
