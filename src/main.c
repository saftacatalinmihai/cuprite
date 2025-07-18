#include "fio.h"
#include "http.h" /* the HTTP facil.io extension */
#include "db/db.h"
#include "router.h"
#include "application_controller.h"
#include "fiobj.h"
#include "models/generated/product.h"
#include <stdio.h>

void on_request(http_s *request) {
  route_request(request);
}

void on_thread_exit(void *arg) {
  db_thread_close();
}

// Listen to HTTP requests and start facil.io
int main(void) {
  db_init_with_filename("cuprite.db");
  // db_migrate();
  initialize_routes();
  template_hash = fiobj_hash_new();

  // SEED a test product so product_find(1) works
  if (!product_find(1)) {
    for (size_t i = 0; i < 100; ++i)
    {
      Product *p_seed = product_new();
      char buffer[2000];
      sprintf(buffer, "Test Product %zu", i);
      p_seed->name = strdup(buffer);
      product_save(p_seed);
      product_free(p_seed);
    }
  }

  fio_state_callback_add(FIO_CALL_ON_FINISH, on_thread_exit, NULL);
  http_listen("3001", NULL, .on_request = on_request, .log = 1);
  // fio_start();
  fio_start(.threads = 1);
  // fio_start(.threads = 32);
}