#include <raygui.h>

#include <ctype.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

#include "CONSTANTS.h"
#include "utilsfn.h"

// Validate if integer input is between valid range of digits
bool is_valid_integer_input(const char *input, const int min_len, const int max_len)
{
	int length = strlen(input);
	return length >= min_len && length <= max_len;
}

// The function takes a block of text (input) and attempts to format it into multiple lines
// Constructs the wrapped text into the output buffer, inserting newline characters (\n) where line breaks are made.
// The buffer output must be larger than the initial input due to the newlines
void wrap_text(const char *input, char *output, const int wrap_width)
{
	char buffer[256];
	strncpy(buffer, input, sizeof(buffer) - 1);
	buffer[sizeof(buffer) - 1] = '\0'; // Ensure buffer is null-terminated
	int text_width = 0;
	int output_index = 0;

	int font_size = GuiGetStyle(DEFAULT, TEXT_SIZE);
	const int SPACE_WIDTH = MeasureText(" ", font_size);

	char *token = strtok(buffer, " ");
	while (token != NULL) {
		int word_width = MeasureText(token, font_size);

		if (text_width + word_width + SPACE_WIDTH > wrap_width) {
			output[output_index++] = '\n'; // Insert line break
			text_width = 0; // Reset line width
		}

		strcpy(&output[output_index], token);
		output_index += strlen(token);
		output[output_index++] = ' '; // Add space after word

		text_width += word_width + SPACE_WIDTH;
		token = strtok(NULL, " ");
	}

	output[output_index - 1] = '\0'; // Null-terminate string
}

// Function to filter integer input, max length will prevent the input from getting past the chosen number
// Input will only get written with digits, while it is less than max_len
void filter_integer_input(char *input, const int max_len)
{
	int index = 0;
	char filtered[MAX_INPUT] = {0};

	for (int i = 0; input[i] != '\0'; i++) {
		if (isdigit(input[i]) && index < max_len) {
			filtered[index++] = input[i];
			if (index >= MAX_INPUT) {
				break;
			}
		}
	}
	filtered[index] = '\0';

	strcpy(input, filtered);
}

bool validate_date(int year, int month, int day)
{
	if (year < 1) {
		return false;
	}

	// Check month
	switch (month) {
	// Jan, Mar, May, Jul, Aug, Oct, Dec:
	case 1: case 3: case 5: case 7: case 8: case 10: case 12:
		if (day < 1 || day > 31) {
			return false;
		}
		break;
	// Apr, Jun, Sept, Nov:
	case 4: case 6: case 9: case 11:
		if (day < 1 || day > 30) {
			return false;
		}
		break;
	//Feb:
	case 2:
		if ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)) {
			if (day < 1 || day > 29) {
				return false; // Leap year
			}
		} else {
			if (day < 1 || day > 28) {
				return false; // Non-leap year
			}
		}
		break;
	// Will never happen in this app
	default:
		return false; // Invalid month
	}

	return true;
}

void int_to_str(int num, char *str)
{
	int i = 0;

	while (num > 0) {
		str[i++] = num % 10 + '0';
		num /= 10;
	}

	str[i] = '\0';

	for (int j = 0, k = i - 1; j < k; j++, k--) {
		char temp = str[j];
		str[j] = str[k];
		str[k] = temp;
	}
}