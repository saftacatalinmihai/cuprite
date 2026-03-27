#include "products_controller.h"
#include "application_controller.h"
#include "../models/generated/product.h"

#include "http.h" /* the HTTP facil.io extension */
#include "fiobj.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

void products_controller_index(http_s* request) {
    int count = 0;
    Product** products = product_all(&count);

    FIOBJ products_ary = fiobj_ary_new();
    for (int i = 0; i < count; i++) {
        FIOBJ product_hash = fiobj_hash_new();
        fiobj_hash_set(product_hash, fiobj_str_new("name", 4), fiobj_str_new(products[i]->name, strlen(products[i]->name)));
        char idStr[128];
        sprintf(idStr, "%d", products[i]->id);
        fiobj_hash_set(product_hash, fiobj_str_new("id", 2), fiobj_str_new(idStr, strlen(idStr)));
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

void products_controller_show(http_s* request) {
    FIOBJ id_obj = fiobj_hash_get(request->params, fiobj_str_new("id", 2));
    if (!id_obj) {
        http_send_error(request, 404);
        return;
    }
    int product_id = (int)fiobj_obj2num(id_obj);
    Product* p = product_find(product_id);
    if (p) {
        FIOBJ data = fiobj_hash_new();
        fiobj_hash_set(data, fiobj_str_new("product_name", 12), fiobj_str_new(p->name, strlen(p->name)));
        render(request, "products/show", data);
        fiobj_free(data);
        product_free(p);
    } else {
        http_send_error(request, 404);
    }
}