#ifndef UTILSFN_H
#define UTILSFN_H

// Validate if integer input is between valid range of digits
bool is_valid_integer_input(const char *input, const int minLength, const int maxLength);

// The function takes a block of text (input) and attempts to format it into multiple lines with a maximum width (maxWidth).
// Constructs the wrapped text into the output buffer, inserting newline characters (\n) where line breaks are made.
// The buffer output must be larger than the initial input due to the newlines
void wrap_text(const char *input, char *output, int maxWidth);

// Function to filter integer input, max length will prevent the input from getting past the chosen number
// Input will only get written with digits, while it is less than maxlength
void filter_integer_input(char *input, const int maxLength);

#endif //UTILSFN_H