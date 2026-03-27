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
        return ; \
    } \
}

#define http_404_if_null_return_NULL(o) { \
    if (!(o)) { \
        http_send_error(request, 404); \
        return NULL; \
    } \
}

#define get_id_from_param_or_404(request) ({ \
    FIOBJ _id_obj = fiobj_hash_get(request->params, fiobj_str_new("id", 2)); \
    http_404_if_null_return_NULL(_id_obj); \
    _id_obj; \
})

#define render_index(find_block) do { \
    find_block; \
    private_render_products_view(request, "products/index", products, count); \
    for (int i = 0; i < count; i++) { product_free(products[i]); } \
    free(products); \
} while (0)

#define render_show(find_block) do { \
    FIOBJ id_obj = fiobj_hash_get(request->params, fiobj_str_new("id", 2)); \
    http_404_if_null(id_obj); \
    int request_params_id = (int)fiobj_obj2num(id_obj); \
    find_block; \
    if (!product) { return; } \
    private_render_product_view(request, "products/show", product); \
    product_free(product); \
} while (0)

#define render_edit(find_block) do { \
    FIOBJ id_obj = fiobj_hash_get(request->params, fiobj_str_new("id", 2)); \
    http_404_if_null(id_obj); \
    /* adds the request_params_id in the scope */ \
    int request_params_id = (int)fiobj_obj2num(id_obj); \
    find_block; \
    if (!product) { return; } \
    FIOBJ data = product_to_fiobj(product); \
    render(request, "products/edit", data); \
    fiobj_free(data); \
    product_free(product); \
} while (0)

void private_render_product_view(http_s* request, char* view, Product* p) {
    FIOBJ data = product_to_fiobj(p);
    render(request, view, data);
    fiobj_free(data);
    return ;
}

void private_render_products_view(http_s* request, char* view, Product** products, int count) {
    FIOBJ data = products_to_fiobj(products, count);
    render(request, view, data);
    fiobj_free(data);
    return ;
}


Product* find_product_by_request_id(http_s* request) {
    FIOBJ id_obj = get_id_from_param_or_404(request);
    Product* p = product_find((int)fiobj_obj2num(id_obj));
    if (!p) { 
        http_send_error(request, 404); 
        return NULL; 
    }
    return p;
}
void products_index_1(http_s* request) {
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

void products_index(http_s* request) {
    render_index(
        int count = 0;
        Product** products = product_all(&count);
    );
}

// More explicit version of products_show - not using macros.
void products_show_1(http_s* request) {
    Product* p = find_product_by_request_id(request);
    if (!p) { return; }
    private_render_product_view(request, "products/show", p);
    product_free(p);
}

// Using macros to simplify the code - make it look closer to Rails code.
void products_show(http_s* request) {
    render_show(
        Product* product = product_find(request_params_id);
    );
}

void products_new(http_s* request){
    FIOBJ data = fiobj_hash_new();
    render(request, "products/new", data);
    fiobj_free(data);
}

void products_create(http_s* request) {
    KeyValuePairArray form_data = parse_body_form(request);
    http_404_if_null(form_data.pairs);

    Product* p = product_new();
    p->name = get_value_from_kv_array(&form_data, "name");;
    int id = product_save(p);
    printf("Created and saved new product with id: %d\n", id);

    private_render_product_view(request, "products/show", p);
    body_form_free(&form_data);

    // product_free(p); // THIS fails for some reason... don't know why.
}

void products_edit_1(http_s* request) {
    Product* p = find_product_by_request_id(request);
    if (!p) { return; }
    FIOBJ data = product_to_fiobj(p);
    render(request, "products/edit", data);
    fiobj_free(data);
}

void products_edit(http_s* request) {
    render_edit(
        Product* product = product_find(request_params_id);
    );
}

void products_update(http_s* request) {
    Product* p = find_product_by_request_id(request);
    http_404_if_null(p);

    KeyValuePairArray form_data = parse_body_form(request);
    http_404_if_null(form_data.pairs);

    p->name = get_value_from_kv_array(&form_data, "name");
    product_save(p);
    private_render_product_view(request, "products/show", p);
    body_form_free(&form_data);
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