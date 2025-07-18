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

void route_patch(char* path, controller_action action) {
    if (route_count < MAX_ROUTES) {
        routes[route_count].method = "PATCH";
        routes[route_count].path = path;
        routes[route_count].action = action;
        route_count++;
    }
}

void route_put(char* path, controller_action action) {
    if (route_count < MAX_ROUTES) {
        routes[route_count].method = "PUT";
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

            // printf("Route method: %s\n", method_info.data);
            // printf("Route path template: %s\n", route_path_template);
            // printf("Request path: %s\n", request_path);

            if (strchr(route_path_template, ':')) {
                char* param_start = strchr(route_path_template, ':');
                // check if the prefix matches
                if (strncmp(request_path, route_path_template, param_start - route_path_template) == 0) {
                    param_value = request_path + (param_start - route_path_template);
                    // printf("Param value: %s\n", param_value);
                    
                    // check if there is a slash in the param
                    // check if suffix matches
                    char* template_suffix = strchr(param_start, '/');
                    char* suffix = strchr(param_value, '/');
                    if (template_suffix && !suffix) {
                        continue;
                    }
                    if (suffix && !template_suffix) {
                        continue;
                    }
                    if (suffix && template_suffix){
                        // printf("Template suffix %s\n", template_suffix);
                        // printf("Suffix %s\n", suffix);

                        if (strncmp(template_suffix, suffix, strlen(template_suffix)) == 0) {
                            param_value = strtok(param_value, "/");
                            // printf("Suffix %s\n", suffix);
                        } else {
                            continue;
                        }
                    }
                    param_int = atoi(param_value);
                }
            }
            if (param_int > 0) {
                // printf("Param int: %d\n", param_int);
                if (!request->params) {
                    request->params = fiobj_hash_new();
                }
                fiobj_hash_set(request->params, fiobj_str_new("id", 2), fiobj_str_new(param_value, strlen(param_value)));
                // printf("Routing to route: %i\n", i);
                routes[i].action(request);
                clock_t end = clock();
                double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
                printf("Route duration %f s, %f millis, %f micros, %f nanos\n", time_spent, time_spent * 1000, time_spent * 1000000, time_spent * 1000000000);
                return;
            } else if (strcmp(request_path, route_path_template) == 0) {
                // printf("Routing to route: %i\n", i);
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