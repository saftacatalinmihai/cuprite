#include "products_controller.h"
#include "application_controller.h"
#include "models/generated/product.h"

#include "http.h" /* the HTTP facil.io extension */
#include "fiobj.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

void private_render_products_view(http_s* request, char* view, Product* p) {
    FIOBJ data = fiobj_hash_new();
    fiobj_hash_set(data, fiobj_str_new("name", 4), fiobj_str_new(p->name, strlen(p->name)));
    fiobj_hash_set(data, fiobj_str_new("id", 2), fiobj_num_new(p->id));
    render(request, view, data);
    fiobj_free(data);
    return ;
}

Product* find_product_by_request_id(http_s* request) {
    FIOBJ id_obj = fiobj_hash_get(request->params, fiobj_str_new("id", 2));
    if (!id_obj) {
        http_send_error(request, 404);
        return NULL;
    }
    Product* p = product_find((int)fiobj_obj2num(id_obj));
    if (!p) {
        http_send_error(request, 404);
        return NULL;
    }
    return p;
}

void products_index(http_s* request) {
    int count = 0;
    Product** products = product_all(&count);

    FIOBJ products_ary = fiobj_ary_new();
    for (int i = 0; i < count; i++) {
        FIOBJ product_hash = fiobj_hash_new();
        fiobj_hash_set(product_hash, fiobj_str_new("name", 4), fiobj_str_new(products[i]->name, strlen(products[i]->name)));
        fiobj_hash_set(product_hash, fiobj_str_new("id", 2), fiobj_num_new(products[i]->id));
        fiobj_ary_push(products_ary, product_hash);
    }

    FIOBJ data = fiobj_hash_new();
    fiobj_hash_set(data, fiobj_str_new("products", 8), products_ary);
    render(request, "products/index", data);
    fiobj_free(data);

    for (int i = 0; i < count; i++) {
        product_free(products[i]);
    }
    free(products);
}


void products_show(http_s* request) {
    Product* p = find_product_by_request_id(request);
    if (!p) {
        return;
    }
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

    Product* p = product_new();
    p->name = product_name;
    
    int id = product_save(p);
    printf("Created and saved new product with id: %d\n", id);
    
    private_render_products_view(request, "products/show", p);
    // product_free(p); // THIS fails for some reason... don't know why.
}

void products_edit(http_s* request) {
    Product* p = find_product_by_request_id(request);
    if (!p) {
        return;
    }

    FIOBJ data = fiobj_hash_new();
    fiobj_hash_set(data, fiobj_str_new("name", 4), fiobj_str_new(p->name, strlen(p->name)));
    fiobj_hash_set(data, fiobj_str_new("id", 2), fiobj_num_new(p->id));
    render(request, "products/edit", data);
    fiobj_free(data);
}


void products_update(http_s* request) {
    Product* p = find_product_by_request_id(request);
    if (!p) {
        return;
    }
    
    fio_str_info_s body = fiobj_data_gets(request->body);
    strtok(body.data, "=\n"); // consumes the "name" form key
    char* product_name = strtok(NULL, "=\n"); // gents the value of the name form key 

    p->name = product_name;
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