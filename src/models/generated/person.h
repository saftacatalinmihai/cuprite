#ifndef PERSON_GENERATED_H
#define PERSON_GENERATED_H

#include "../person.h"

Person* person_new(void);
void person_free(Person* m);
int person_save(Person* m);
Person* person_find(int id);
void person_destroy(int id);

#endif
