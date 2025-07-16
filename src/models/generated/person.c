#include "person.h"
#include "../../db/db.h"
#include <stdlib.h>
#include <string.h>

Person* person_new(void) {
    Person* m = malloc(sizeof(Person));
    m->id = 0;
    m->name = NULL;
    m->birth_year = 0;
    return m;
}

void person_free(Person* m) {
    if (m) {
        free(m->name);
        free(m);
    }
}

int person_save(Person* m) {
    if (m->id == 0) {
        // Create
        const char* sql = "INSERT INTO persons (name, birth_year) VALUES (?, ?)";
        sqlite3_stmt* stmt = db_prepare(sql);
        if (stmt) {
             db_bind_text(stmt, 1, m->name);
             db_bind_int(stmt, 2, m->birth_year);
            db_step(stmt);
            m->id = sqlite3_last_insert_rowid(db_handle());
            db_finalize(stmt);
        }
    } else {
        // Update
        const char* sql = "UPDATE persons SET name = ?, birth_year = ? WHERE id = ?";
        sqlite3_stmt* stmt = db_prepare(sql);
        if (stmt) {
             db_bind_text(stmt, 1, m->name);
             db_bind_int(stmt, 2, m->birth_year);
             db_bind_int(stmt, 3, m->id);
            db_step(stmt);
            db_finalize(stmt);
        }
    }
    return m->id;
}

Person* person_find(int id) {
    const char* sql = "SELECT * FROM persons WHERE id = ?";
    sqlite3_stmt* stmt = db_prepare(sql);
    Person* m = NULL;
    if (stmt) {
        db_bind_int(stmt, 1, id);
        if (db_step(stmt) == SQLITE_ROW) {
            m = person_new();
            m->id = db_column_int(stmt, 0);
            m->name = strdup((const char*)db_column_text(stmt, 1));
            m->birth_year = db_column_int(stmt, 2);
        }
        db_finalize(stmt);
    }
    return m;
}


void person_destroy(int id) {
    const char* sql = "DELETE FROM persons WHERE id = ?";
    sqlite3_stmt* stmt = db_prepare(sql);
    if (stmt) {
        db_bind_int(stmt, 1, id);
        db_step(stmt);
        db_finalize(stmt);
    }
}
