#ifndef APPLICATION_CONTROLLER_H
#define APPLICATION_CONTROLLER_H

#include "http.h"
#include "fiobj.h"

extern FIOBJ template_hash;

void render(http_s* request, char* view, FIOBJ data);

#endif