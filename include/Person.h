#ifndef PERSON_H
#define PERSON_H

#include <string.h>

#include "CONSTANTS.h"

enum gender {
	GENDER_OTHER = 0,
	GENDER_MALE,
	GENDER_FEMALE
};

struct person {
	const char cpf[MAX_CPF_LENGTH];
	char name[MAX_INPUT];
	int age;
	char health_status[MAX_INPUT];
	char needs[MAX_INPUT];
	enum gender gender;
};

struct person person_init(char *name, int age, char *health_status, char *needs, enum gender gender);

#endif // PERSON_H