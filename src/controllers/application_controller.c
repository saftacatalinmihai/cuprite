#include "application_controller.h"
#include "http.h"
#include "fiobj.h"

#include "fiobj_mustache.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

FIOBJ template_hash;

#define USE_TEMPLATE_HASH 0

static mustache_s* load_template(char* path) {
    #if USE_TEMPLATE_HASH
        FIOBJ key = fiobj_str_new(path, strlen(path));
        FIOBJ template_obj = fiobj_hash_get(template_hash, key);

        if (template_obj) {
            // fprintf(stderr, "Loading cached template: %s\n", path);
            fiobj_free(key);
            return (mustache_s*)(uintptr_t)fiobj_obj2num(template_obj);
        }
    #endif

    fprintf(stderr, "Loading template: %s\n", path);
    mustache_s* template = fiobj_mustache_load((fio_str_info_s){.data = path, .len = strlen(path)});
    
    #if USE_TEMPLATE_HASH
        if (template) {
            template_obj = fiobj_num_new((intptr_t)template);
            fiobj_hash_set(template_hash, key, template_obj);
        } else {
            fiobj_free(key);
        }
    #endif

    return template;
}

void render(http_s* request, char* view, FIOBJ data) {
    char view_path[256];
    snprintf(view_path, sizeof(view_path), "src/views/%s.html", view);

    char layout_path[256];
    snprintf(layout_path, sizeof(layout_path), "src/views/layouts/application.html");

    mustache_s* view_template = load_template(view_path);
    mustache_s* layout_template = load_template(layout_path);

    if (!layout_template || !view_template) {
        if (!view_template) fprintf(stderr, "Error: Could not load view template: %s\n", view_path);
        if (!layout_template) fprintf(stderr, "Error: Could not load layout template: %s\n", layout_path);
        http_send_error(request, 500);
        return;
    }

    FIOBJ view_content_obj = fiobj_mustache_build(view_template, data);
    fio_str_info_s view_content_str = fiobj_obj2cstr(view_content_obj);
    fiobj_hash_set(data, fiobj_str_new("yield", 5), fiobj_str_new(view_content_str.data, view_content_str.len));

    FIOBJ response_body = fiobj_mustache_build(layout_template, data);
    fiobj_free(view_content_obj);
    fio_str_info_s body = fiobj_obj2cstr(response_body);

    http_set_header(request, HTTP_HEADER_CONTENT_TYPE, http_mimetype_find("html", 4));
    http_send_body(request, body.data, body.len);

    fiobj_free(response_body);
}