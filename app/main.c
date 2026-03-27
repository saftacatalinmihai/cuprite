#include "cuprite.h"
#include "models/generated/product.h"
#include "models/generated/todo.h"
#include "controllers/products_controller.h"
#include "controllers/todos_controller.h"
#include "routes.h"

int main(int argc, char *argv[]) {
  db_init_with_filename("cuprite.db");
  // SEED a test product so product_find(1) works
  if (!product_find(2)) {
    for (size_t i = 0; i < 100; ++i) {
      Product *p_seed = product_new();
      char buffer[2000];
      sprintf(buffer, "Test Product %zu", i);
      p_seed->name = strdup(buffer);
      product_save(p_seed);
      product_free(p_seed);
    }
  }

  // SEED a test product so product_find(1) works
  if (!todo_find(1)) {
    for (size_t i = 0; i < 10; ++i) {
      Todo *t_seed = todo_new();
      char buffer[2000];
      sprintf(buffer, "Test TODO %zu", i);
      t_seed->text = strdup(buffer);
      t_seed->completed = true;
      todo_save(t_seed);
      todo_free(t_seed);
    }
  }

  return run(argc, argv);
}