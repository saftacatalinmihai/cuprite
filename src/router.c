#include "router.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX_ROUTES 100

typedef struct {
    char* method;
    char* path;
    controller_action action;
} route_t;

static route_t routes[MAX_ROUTES];
static int route_count = 0;

void route_get(char* path, controller_action action) {
    if (route_count < MAX_ROUTES) {
        routes[route_count].method = "GET";
        routes[route_count].path = path;
        routes[route_count].action = action;
        route_count++;
    }
}

void route_post(char* path, controller_action action) {
    if (route_count < MAX_ROUTES) {
        routes[route_count].method = "POST";
        routes[route_count].path = path;
        routes[route_count].action = action;
        route_count++;
    }
}

void route_request(http_s* request) {
    clock_t begin = clock();

    fio_str_info_s path_info = fiobj_obj2cstr(request->path);
    fio_str_info_s method_info = fiobj_obj2cstr(request->method);

    for (int i = 0; i < route_count; i++) {
        if (strcmp(method_info.data, routes[i].method) == 0) {
            char* route_path_template = routes[i].path;
            char* request_path = path_info.data;
            char* param_value = NULL;
            int param_int = 0;

            if (strchr(route_path_template, ':')) {
                char* param_start = strchr(route_path_template, ':');
                // check if the prefix matches
                if (strncmp(request_path, route_path_template, param_start - route_path_template) == 0) {
                    param_value = request_path + (param_start - route_path_template);
                    // check if there is a slash in the param
                    if (strchr(param_value, '/')) {
                        continue;
                    }
                    param_int = atoi(param_value);
                }
            }
            if (param_int > 0) {
                if (!request->params) {
                    request->params = fiobj_hash_new();
                }
                fiobj_hash_set(request->params, fiobj_str_new("id", 2), fiobj_str_new(param_value, strlen(param_value)));
                routes[i].action(request);
                clock_t end = clock();
                double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
                printf("Route duration %f s, %f millis, %f micros, %f nanos\n", time_spent, time_spent * 1000, time_spent * 1000000, time_spent * 1000000000);
                return;
            } else if (strcmp(request_path, route_path_template) == 0) {
                routes[i].action(request);
                clock_t end = clock();
                double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
                printf("Route duration %f s, %f millis, %f micros, %f nanos\n", time_spent, time_spent * 1000, time_spent * 1000000, time_spent * 1000000000);
                
                return;
            }
        }
    }

    http_send_error(request, 404);
}