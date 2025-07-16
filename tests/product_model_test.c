#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include "../src/db/db.h"
#include "../src/models/generated/product.h"

void run_migrations(void) {
    char* err_msg = 0;
    const char* sql = "CREATE TABLE products (id INTEGER PRIMARY KEY, name TEXT);";
    int rc = sqlite3_exec(db_handle(), sql, 0, 0, &err_msg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
    }
}

void test_create_and_find_product(void) {
    printf("Running test: test_create_and_find_product...\n");

    char* product_name = "Test Product 2";
    // Create a new product
    Product* p = product_new();

    p->name = strdup(product_name);

    // Save the product to the database
    product_save(p);
    assert(p->id > 0);
    printf("Product ID: %d\n", p->id);

    // Find the product by ID
    Product* found_product = product_find(p->id);
    assert(found_product != NULL);
    assert(strcmp(found_product->name, product_name) == 0);

    printf("Product found: %s\n", found_product->name);

    // Clean up
    product_destroy(p->id);
    product_free(p);
    product_free(found_product);

    printf("Test passed!\n");
}

int main(void) {
    remove("test.db");
    db_init("test.db");
    run_migrations();
    test_create_and_find_product();
    db_close();
    return 0;
}