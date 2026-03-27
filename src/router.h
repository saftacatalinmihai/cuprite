#ifndef ROUTER_H
#define ROUTER_H

#include "http.h"

typedef void (*controller_action)(http_s*);

void route_get(char* path, controller_action action);
void route_post(char* path, controller_action action);
void route_request(http_s* request);
void initialize_routes(void);

#endif