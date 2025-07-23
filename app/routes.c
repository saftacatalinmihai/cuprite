#include "router.h"
#include "controllers/products_controller.h"
#include "controllers/todos_controller.h"

void initialize_routes(void) {
    // PRODUCTS
    route_get("/products", products_index);

    route_get("/products/new", products_new);
    route_post("/products", products_create);

    route_get("/products/:id", products_show);

    route_get("/products/:id/edit", products_edit);
    route_patch("/products/:id", products_update);
    route_put("/products/:id", products_update);

    route_delete("/products/:id", products_destroy);

    // TODOS
    route_get("/todos", todos_index);

    route_post("/todos", todos_create);

    route_patch("/todos", todos_mark_all_complete);
    route_patch("/todos/:id", todos_update);
    route_put("/todos/:id", todos_update);

    route_delete("/todos/:id", todos_destroy);
}