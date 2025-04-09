#ifndef PERSON_H
#define PERSON_H

#include <string.h>

#include "CONSTANTS.h"

enum Gender {
	GENDER_OTHER = 0,
	GENDER_MALE,
	GENDER_FEMALE
};

struct Person {
	const char cpf[MAX_CPF_LENGTH];
	char name[MAX_INPUT];
	int age;
	char healthStatus[MAX_INPUT];
	char needs[MAX_INPUT];
	enum Gender gender;
};

struct Person personInit(char *name, int age, char *healthStatus, char *needs, enum Gender gender);

#endif // PERSON_H