#ifndef UTILSFN_H
#define UTILSFN_H

#include <stdbool.h>

#define SET_FLAG(flag, flags) ((*flag) |= (flags))
#define CLEAR_FLAG(flag, flags) ((*flag) &= ~(flags))
#define IS_FLAG_SET(flag, flags) (((*flag) & (flags)) != 0)

/*
 * Checks if the length of the given input string is between the specified minimum and maximum LENGTH (inclusive).
 * Example: min_len = 3, max_len = 6 will allow input int to be in the range of 100 - 999999
 */
bool is_int_between_min_max(const char *input, const int min_len, const int max_len);

/*
 * The function takes a block of text (input) and attempts to format it into multiple lines
 * Constructs the wrapped text into the output buffer, inserting newline characters (\n) where line breaks are made.
 * The buffer output must be larger than the initial input due to the newlines
 */
void wrap_text(const char *input, char *output, const int wrap_width);

/*
 * Will filter input to only allow integers to be written, until max_len
 */
void filter_integer_input(char *input, const int max_len);

/*
 * Validates if the given year, month and day is valid
 */
bool validate_date(int year, int month, int day);

#endif //UTILSFN_H