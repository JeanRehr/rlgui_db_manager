#include "Person.h"
#include "CONSTANTS.h"

typedef enum Gender Gender;

typedef struct Person Person;

Person personInit(char *name, int age, char *healthStatus, char *needs, Gender gender) {
	Person person = {0};
	strcpy(person.name, name);
	person.age = age;
	strcpy(person.healthStatus, healthStatus);
	strcpy(person.needs, needs);
	person.gender = gender;
	return person;
}