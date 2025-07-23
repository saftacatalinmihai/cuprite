#ifndef TODOS_CONTROLLER_H
#define TODOS_CONTROLLER_H

#include "http.h"

void todos_index(http_s* request);
void todos_create(http_s* request);
void todos_update(http_s* request);
void todos_destroy(http_s* request);
void todos_mark_all_complete(http_s* request);

#endif