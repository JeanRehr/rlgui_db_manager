/**
 * @file utilsfn.h
 * @brief Utility Functions for Input Handling and Formatting
 *
 * This header provides various utility functions for input validation,
 * text formatting, and date handling.
 */

#ifndef UTILSFN_H
#define UTILSFN_H

#include <stdbool.h>

/**
 * @def SET_FLAG(flag, flags)
 * @brief Sets specified bits in a flag variable
 * 
 * Performs a bitwise OR operation to set one or more flags.
 * 
 * @param flag Pointer to the flag variable to modify
 * @param flags Bitmask of flags to set
 * @note Example: SET_FLAG(&status, STATUS_ACTIVE | STATUS_VISIBLE)
 */

/**
 * @def CLEAR_FLAG(flag, flags)
 * @brief Clears specified bits in a flag variable
 * 
 * Performs a bitwise AND with the complement to clear one or more flags.
 * 
 * @param flag Pointer to the flag variable to modify
 * @param flags Bitmask of flags to clear
 * @note Example: CLEAR_FLAG(&status, STATUS_ACTIVE)
 */

/**
 * @def IS_FLAG_SET(flag, flags)
 * @brief Tests if specified bits are set in a flag variable
 * 
 * Performs a bitwise AND to check if one or more flags are set.
 * 
 * @param flag Pointer to the flag variable to check
 * @param flags Bitmask of flags to test
 * @return true if all specified flags are set, false otherwise
 * @note Example: if (IS_FLAG_SET(&status, STATUS_READY)) {...}
 */
#define SET_FLAG(flag, flags) ((*flag) |= (flags))
#define CLEAR_FLAG(flag, flags) ((*flag) &= ~(flags))
#define IS_FLAG_SET(flag, flags) (((*flag) & (flags)) != 0)

/**
 * @brief Validates if an integer string falls within length bounds
 *
 * Checks if the string representation of an integer has a length between
 * the specified minimum and maximum values (inclusive).
 *
 * @param[in] input The string to validate
 * @param[in] min_len Minimum allowed length (inclusive)
 * @param[in] max_len Maximum allowed length (inclusive)
 * @return true if length is within bounds, false otherwise
 * @note Does not validate if the string contains only digits
 */
bool is_int_between_min_max(const char *input, const int min_len, const int max_len);

/**
 * @brief Wraps text to fit within a specified pixel width
 *
 * Inserts line breaks into text to ensure it doesn't exceed the specified
 * display width when rendered. Uses space characters as word boundaries.
 *
 * @param[in] input The text to wrap
 * @param[out] output Buffer to store the wrapped text (must be large enough)
 * @param[in] wrap_width Maximum pixel width for each line
 * @warning The output buffer should be at least as large as the input buffer
 * @note Uses the current GUI font size for measurement
 */
void wrap_text(const char *input, char *output, const int wrap_width);

/**
 * @brief Filters non-digit characters from an input string
 *
 * Processes an input string to remove all non-digit characters and
 * truncate to the maximum allowed length.
 *
 * @param[in,out] input The string to filter (modified in-place)
 * @param[in] max_len Maximum allowed length for the result
 * @note The input buffer must be null-terminated
 * @warning Will truncate the string to MAX_INPUT-1 characters if needed
 */
void filter_integer_input(char *input, const int max_len);

/**
 * @brief Validates a date's components
 *
 * Checks if the provided year, month, and day form a valid calendar date,
 * including leap year handling for February.
 *
 * @param[in] year The year component (1-9999)
 * @param[in] month The month component (1-12)
 * @param[in] day The day component (1-31, depending on month)
 * @return true if the date is valid, false otherwise
 * @note Uses Gregorian calendar rules for leap years
 */
bool validate_date(int year, int month, int day);

#endif //UTILSFN_H
