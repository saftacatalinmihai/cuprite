#include "company.h"
#include "../../db/db.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

Company* company_new(void) {
    Company* m = malloc(sizeof(Company));
    m->id = 0;
    m->name = NULL;
    return m;
}

void company_free(Company* m) {
    if (m) {
        free(m->name);
        free(m);
    }
}

int company_save(Company* m) {
    if (m->id == 0) {
        // Create
        const char* sql = "INSERT INTO companys (name) VALUES (?)";
        sqlite3_stmt* stmt = db_prepare(sql);
        if (stmt) {
             db_bind_text(stmt, 1, m->name);
            db_step(stmt);
            m->id = sqlite3_last_insert_rowid(db_handle());
            db_finalize(stmt);
        }
    } else {
        // Update
        const char* sql = "UPDATE companys SET name = ? WHERE id = ?";
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

Company* company_find(int id) {
    Company* m = NULL;
    const char* sql = "SELECT id, name FROM companys WHERE id = ?";
    sqlite3_stmt* stmt = db_prepare(sql);

    if (stmt) {
        if (db_bind_int(stmt, 1, id) != SQLITE_OK) {
            fprintf(stderr, "Failed to bind id: %s\n", sqlite3_errmsg(db_handle()));
            db_finalize(stmt);
            return NULL;
        }
        
        int r = db_step(stmt);
        if (r == SQLITE_ROW) {
            m = company_new();
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

Company** company_all(int* count) {
    const char* sql = "SELECT id, name FROM companys";
    sqlite3_stmt* stmt = db_prepare(sql);
    
    *count = 0;
    if (!stmt) {
        return NULL;
    }

    int capacity = 0;
    Company** models = NULL;

    while (db_step(stmt) == SQLITE_ROW) {
        if (*count >= capacity) {
            capacity = (capacity == 0) ? 10 : capacity * 2;
            Company** new_models = realloc(models, sizeof(Company*) * capacity);
            if (!new_models) {
                fprintf(stderr, "Failed to reallocate memory for models\n");
                for(int i = 0; i < *count; i++) {
                    company_free(models[i]);
                }
                free(models);
                db_finalize(stmt);
                return NULL;
            }
            models = new_models;
        }
        Company* m = company_new();
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


void company_destroy(int id) {
    const char* sql = "DELETE FROM companys WHERE id = ?";
    sqlite3_stmt* stmt = db_prepare(sql);
    if (stmt) {
        db_bind_int(stmt, 1, id);
        db_step(stmt);
        db_finalize(stmt);
    }
}
