#ifndef PRODUCT_GENERATED_H
#define PRODUCT_GENERATED_H

#include "../product.h"

Product* product_new(void);
void product_free(Product* m);
int product_save(Product* m);
Product* product_find(int id);
Product** product_all(int* count);
void product_destroy(int id);

#endif
