#include <stdbool.h>
#include <ctype.h>
#include <string.h>

#include "utilsfn.h"
#include "CONSTANTS.h"

// Validate if integer input is between valid range of digits
bool isValidIntegerInput(const char *input, const int minLength, const int maxLength)
{
    int length = strlen(input);
    return length >= minLength && length <= maxLength;
}

// The function takes a block of text (input) and attempts to format it into multiple lines with a maximum width (maxWidth).
// Constructs the wrapped text into the output buffer, inserting newline characters (\n) where line breaks are made.
// The buffer output must be larger than the initial input due to the newlines
void WrapText(const char *input, char *output, int maxWidth) {
    int length = strlen(input);
    int lineStart = 0;
    int outputIndex = 0;

    while (lineStart < length) {
        int lineEnd = lineStart + maxWidth;

        if (lineEnd >= length) {
            strcpy(&output[outputIndex], &input[lineStart]);
            outputIndex += strlen(&output[outputIndex]);
            break;
        }

        // Find word break
        while (lineEnd > lineStart && input[lineEnd] != ' ') {
            lineEnd--;
        }

        if (lineEnd == lineStart) {
            lineEnd = lineStart + maxWidth;  // Forced break
        }

        strncpy(&output[outputIndex], &input[lineStart], lineEnd - lineStart);
        outputIndex += lineEnd - lineStart;
        output[outputIndex++] = '\n';

        lineStart = lineEnd + 1;  // Skip space
    }
    output[outputIndex] = '\0';
}

// Function to filter integer input, max length will prevent the input from getting past the chosen number
// Input will only get written with digits, while it is less than maxlength
void filterIntegerInput(char *input, const int maxLength)
{
	int index = 0;
	char filtered[MAX_INPUT] = {0};

	for (int i = 0; input[i] != '\0'; i++) {
		if (isdigit(input[i]) && index < maxLength) {
			filtered[index++] = input[i];
			if (index >= MAX_INPUT) {
				break;
			}
		}
	}
	filtered[index] = '\0';

	strcpy(input, filtered);
}