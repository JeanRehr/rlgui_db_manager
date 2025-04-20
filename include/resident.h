#ifndef resident_H
#define resident_H

#include <stdbool.h>

#include "CONSTANTS.h"

enum gender {
	GENDER_OTHER = 0,
	GENDER_MALE,
	GENDER_FEMALE
};

struct resident {
	const char cpf[MAX_CPF_LENGTH];
	char name[MAX_INPUT];
	int age;
	char health_status[MAX_INPUT];
	char needs[MAX_INPUT];
	bool medical_assistance;
	enum gender gender;
	const char entry_date[11]; // ISO format +null terminator
};

struct resident resident_init(char *name, int age, char *health_status, char *needs, bool medical_assistance, enum gender gender);

#endif // resident_H