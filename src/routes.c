#include "router.h"
#include "controllers/products_controller.h"

void initialize_routes(void) {
    route_get("/products", products_controller_index);
    route_get("/products/:id", products_controller_show);
}