#ifndef UTILSFN_H
#define UTILSFN_H

#include <stdbool.h>

#define SET_FLAG(flag, flags) ((*flag) |= (flags))
#define CLEAR_FLAG(flag, flags) ((*flag) &= ~(flags))
#define IS_FLAG_SET(flag, flags) (((*flag) & (flags)) != 0)

// Validate if integer input is between valid range of digits
bool is_valid_integer_input(const char *input, const int min_len, const int max_len);

// The function takes a block of text (input) and attempts to format it into multiple lines with a maximum width (maxWidth).
// Constructs the wrapped text into the output buffer, inserting newline characters (\n) where line breaks are made.
// The buffer output must be larger than the initial input due to the newlines
void wrap_text(const char *input, char *output, const int wrap_width);

// Function to filter integer input, max length will prevent the input from getting past the chosen number
// Input will only get written with digits, while it is less than maxlength
void filter_integer_input(char *input, const int max_len);

bool validate_date(int year, int month, int day);

void int_to_str(int num, char *str);

#endif //UTILSFN_H