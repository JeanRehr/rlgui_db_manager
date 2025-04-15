#include <stdio.h>
#include "utilsfn.h"

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

int main() {
    int t = 12345;
    char str[10];

    int_to_str(t, str);
    printf("string %s", str);
    return 0;
}