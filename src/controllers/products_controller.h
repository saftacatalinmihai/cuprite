#ifndef PRODUCTS_CONTROLLER_H
#define PRODUCTS_CONTROLLER_H

#include "http.h"

void products_controller_show(http_s* request);
void products_controller_index(http_s* request);
void products_controller_new(http_s* request);
void products_controller_create(http_s* request);

#endif