#include "product.h"
#include "../../db/db.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

Product* product_new(void) {
    Product* m = malloc(sizeof(Product));
    m->id = 0;
    m->name = NULL;
    return m;
}

void product_free(Product* m) {
    if (m) {
        free(m->name);
        free(m);
    }
}

int product_save(Product* m) {
    if (m->id == 0) {
        // Create
        const char* sql = "INSERT INTO products (name) VALUES (?)";
        sqlite3_stmt* stmt = db_prepare(sql);
        if (stmt) {
            db_bind_text(stmt, 1, m->name);
            db_step(stmt);
            m->id = sqlite3_last_insert_rowid(db_handle());
            db_finalize(stmt);
        }
    } else {
        // Update
        const char* sql = "UPDATE products SET name = ? WHERE id = ?";
        sqlite3_stmt* stmt = db_prepare(sql);
        if (stmt) {
            db_bind_text(stmt, 1, m->name);
            db_bind_int(stmt, 2, m->id);
            db_step(stmt);
            db_finalize(stmt);
        }
    }
    return m->id;
}

Product* product_find(int id) {
    Product* m = NULL;
    const char* sql = "SELECT id, name FROM products WHERE id = ?";
    sqlite3_stmt* stmt = db_prepare(sql);

    if (stmt) {
        if (db_bind_int(stmt, 1, id) != SQLITE_OK) {
            fprintf(stderr, "Failed to bind id: %s\n", sqlite3_errmsg(db_handle()));
            db_finalize(stmt);
            return NULL;
        }
        
        int r = db_step(stmt);
        if (r == SQLITE_ROW) {
            m = product_new();
            m->id = db_column_int(stmt, 0);
            const unsigned char* name = db_column_text(stmt, 1);
            if (name) {
                m->name = strdup((const char *)name);
            }
        }
        db_finalize(stmt);
    }
    return m;
}

Product** product_all(int* count) {
    const char* sql = "SELECT id, name FROM products";
    sqlite3_stmt* stmt = db_prepare(sql);
    
    *count = 0;
    if (!stmt) {
        return NULL;
    }

    int capacity = 0;
    Product** models = NULL;

    while (db_step(stmt) == SQLITE_ROW) {
        if (*count >= capacity) {
            capacity = (capacity == 0) ? 10 : capacity * 2;
            Product** new_models = realloc(models, sizeof(Product*) * capacity);
            if (!new_models) {
                fprintf(stderr, "Failed to reallocate memory for models\n");
                for(int i = 0; i < *count; i++) {
                    product_free(models[i]);
                }
                free(models);
                db_finalize(stmt);
                return NULL;
            }
            models = new_models;
        }
        Product* m = product_new();
        m->id = db_column_int(stmt, 0);
        const unsigned char* val_name = db_column_text(stmt, 1);
        if (val_name) {
            m->name = strdup((const char *)val_name);
        }
        models[*count] = m;
        (*count)++;
    }
    db_finalize(stmt);

    return models;
}


void product_destroy(int id) {
    const char* sql = "DELETE FROM products WHERE id = ?";
    sqlite3_stmt* stmt = db_prepare(sql);
    if (stmt) {
        db_bind_int(stmt, 1, id);
        db_step(stmt);
        db_finalize(stmt);
    }
}
