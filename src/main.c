#include "http.h" /* the HTTP facil.io extension */
#include "db/db.h"
#include "router.h"
#include "controllers/application_controller.h"
#include "fiobj.h"
#include "models/generated/product.h"
#include <stdio.h>

// We'll use this callback in `http_listen`, to handles HTTP requests
void on_request(http_s *request);

void test_seed();

// Listen to HTTP requests and start facil.io
int main(void) {
  // initialize database
  db_init("cuprite.db");

  // initialize routes
  initialize_routes();
  template_hash = fiobj_hash_new();

  // SEED a test product so product_find(1) works
  if(!product_find(1)) {
      for (size_t i = 0; i<1000; ++i) {
        Product* p_seed = product_new();
        char buffer[2000];
        sprintf(buffer, "Test Product %zu", i);
        p_seed->name = strdup(buffer);
        product_save(p_seed);
        product_free(p_seed);
      }
  }

  // listen on port 3001 and any available network binding (NULL == 0.0.0.0)
  http_listen("3001", NULL, .on_request = on_request, .log = 1);
  // start the server
  // fio_start();
  fio_start(.threads = 1);
  // fio_start(.threads = -2);
  // deallocating the common values
  db_close();
}

void on_request(http_s *request) {
  route_request(request);
}