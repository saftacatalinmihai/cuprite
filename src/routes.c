#include "router.h"
#include "controllers/products_controller.h"

void initialize_routes(void) {
    route_get("/products", products_index);

    route_get("/products/new", products_new);
    route_post("/products", products_create);

    route_get("/products/:id", products_show);

    route_get("/products/:id/edit", products_edit);
    route_patch("/products/:id", products_update);
    route_put("/products/:id", products_update);

    route_delete("/products/:id", products_destroy);
}