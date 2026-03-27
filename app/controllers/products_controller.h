#ifndef PRODUCTS_CONTROLLER_H
#define PRODUCTS_CONTROLLER_H

#include "http.h"
#include "cuprite.h"
#include "models/generated/product.h"

#include "http.h" /* the HTTP facil.io extension */
#include "fiobj.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

void products_show(http_s* request);
void products_index(http_s* request);
void products_new(http_s* request);
void products_create(http_s* request);
void products_edit(http_s* request);
void products_update(http_s* request);
void products_destroy(http_s* request);

#define http_404_if_null(o) { \
    if (!(o)) { \
        http_send_error(request, 404); \
        return NULL; \
    } \
}

#define get_id_from_param_or_404(request) ({ \
    FIOBJ _id_obj = fiobj_hash_get(request->params, fiobj_str_new("id", 2)); \
    http_404_if_null(_id_obj); \
    _id_obj; \
})

void private_render_products_view(http_s* request, char* view, Product* p) {
    FIOBJ data = product_to_fiobj(p);
    render(request, view, data);
    fiobj_free(data);
    return ;
}

Product* find_product_by_request_id(http_s* request) {
    FIOBJ id_obj = get_id_from_param_or_404(request);
    Product* p = product_find((int)fiobj_obj2num(id_obj));
    http_404_if_null(p);
    return p;
}

void products_index(http_s* request) {
    int count = 0;
    Product** products = product_all(&count);
    FIOBJ data = products_to_fiobj(products, count);
    
    render(request, "products/index", data);
    
    fiobj_free(data);
    for (int i = 0; i < count; i++) {
        product_free(products[i]);
    }
    free(products);
}

void products_show(http_s* request) {
    Product* p = find_product_by_request_id(request);
    if (!p) { return; }
    private_render_products_view(request, "products/show", p);
    product_free(p);
}

void products_new(http_s* request){
    FIOBJ data = fiobj_hash_new();
    render(request, "products/new", data);
    fiobj_free(data);
}

void products_create(http_s* request) {
    if (!request->body) {
        http_send_error(request, 400);
        return;
    }

    fio_str_info_s body = fiobj_data_gets(request->body);
    strtok(body.data, "=\n"); // consumes the "name" form key
    char* product_name = strtok(NULL, "=\n"); // gents the value of the name form key 
    char* decoded_text = malloc(strlen(product_name) + 1);
    url_decode(decoded_text, product_name);

    Product* p = product_new();
    p->name = decoded_text;
    
    int id = product_save(p);
    printf("Created and saved new product with id: %d\n", id);
    
    private_render_products_view(request, "products/show", p);
    // product_free(p); // THIS fails for some reason... don't know why.
}

void products_edit(http_s* request) {
    Product* p = find_product_by_request_id(request);
    if (!p) { return; }

    FIOBJ data = product_to_fiobj(p);
    render(request, "products/edit", data);
    fiobj_free(data);
}


void products_update(http_s* request) {
    Product* p = find_product_by_request_id(request);
    if (!p) { return; }
    
    fio_str_info_s body = fiobj_data_gets(request->body);
    strtok(body.data, "=\n"); // consumes the "name" form key
    char* product_name = strtok(NULL, "=\n"); // gents the value of the name form key
    char* decoded_text = malloc(strlen(product_name) + 1);
    url_decode(decoded_text, product_name);

    p->name = decoded_text;
    product_save(p);
    private_render_products_view(request, "products/show", p);
    return;
}

void products_destroy(http_s* request) {
    FIOBJ id_obj = fiobj_hash_get(request->params, fiobj_str_new("id", 2));
    if (!id_obj) {
        http_send_error(request, 404);
        return ;
    }
    int product_id = (int)fiobj_obj2num(id_obj);
    product_destroy(product_id);
    products_index(request);
    return ;
}

#endif