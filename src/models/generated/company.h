#ifndef COMPANY_GENERATED_H
#define COMPANY_GENERATED_H

#include "../company.h"

Company* company_new(void);
void company_free(Company* m);
int company_save(Company* m);
Company* company_find(int id);
Company** company_all(int* count);
void company_destroy(int id);

#endif
