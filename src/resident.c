#include <string.h>

#include "resident.h"
#include "CONSTANTS.h"

struct resident resident_init(char *name, int age, char *health_status, char *needs, bool medical_assistance, enum gender gender) {
	struct resident resident = {0};
	strcpy(resident.name, name);
	resident.age = age;
	strcpy(resident.health_status, health_status);
	strcpy(resident.needs, needs);
	resident.gender = gender;
	return resident;
}