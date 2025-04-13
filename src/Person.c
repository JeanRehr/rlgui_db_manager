#include <string.h>

#include "person.h"
#include "CONSTANTS.h"

typedef enum gender gender;

typedef struct person person;

person person_init(char *name, int age, char *health_status, char *needs, gender gender) {
	person person = {0};
	strcpy(person.name, name);
	person.age = age;
	strcpy(person.health_status, health_status);
	strcpy(person.needs, needs);
	person.gender = gender;
	return person;
}