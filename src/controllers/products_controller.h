#ifndef PRODUCTS_CONTROLLER_H
#define PRODUCTS_CONTROLLER_H

#include "http.h"

void products_show(http_s* request);
void products_index(http_s* request);
void products_new(http_s* request);
void products_create(http_s* request);
void products_edit(http_s* request);
void products_update(http_s* request);
void products_destroy(http_s* request);

#endif