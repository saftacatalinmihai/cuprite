#include "db.h"
#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <dirent.h>
#include <string.h>

static sqlite3* db = NULL;

void db_init(const char* db_name) {
    if (sqlite3_open(db_name, &db)) {
        fprintf(stderr, "Can't open database: %s\\n", sqlite3_errmsg(db));
        db_close();
        exit(1);
    }
    char *errMsg = 0;

    sqlite3_exec(db, "PRAGMA journal_mode=WAL;", NULL, NULL, &errMsg);
    sqlite3_exec(db, "PRAGMA synchronous=OFF;", NULL, NULL, &errMsg);
    printf("Errors: %s\n", errMsg);
}

void db_migrate(void) {
    db_exec("CREATE TABLE IF NOT EXISTS schema_migrations (version TEXT PRIMARY KEY)");

    DIR *dir;
    struct dirent *ent;
    const char* migrations_path = "db/migrations";

    if ((dir = opendir(migrations_path)) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            if (strstr(ent->d_name, ".sql")) {
                char migration_path[512];
                snprintf(migration_path, sizeof(migration_path), "%s/%s", migrations_path, ent->d_name);

                // Check if migration is already applied
                char check_sql[512];
                snprintf(check_sql, sizeof(check_sql), "SELECT version FROM schema_migrations WHERE version = '%s'", ent->d_name);
                sqlite3_stmt* check_stmt = db_prepare(check_sql);
                
                int applied = 0;
                if (check_stmt) {
                    if (db_step(check_stmt) == SQLITE_ROW) {
                        applied = 1;
                    }
                    db_finalize(check_stmt);
                }

                if (!applied) {
                    FILE *fp = fopen(migration_path, "r");
                    if (fp) {
                        fseek(fp, 0, SEEK_END);
                        long fsize = ftell(fp);
                        fseek(fp, 0, SEEK_SET);

                        char *sql = malloc(fsize + 1);
                        fread(sql, 1, fsize, fp);
                        fclose(fp);
                        sql[fsize] = 0;

                        if(db_exec(sql)) {
                            char insert_sql[512];
                            snprintf(insert_sql, sizeof(insert_sql), "INSERT INTO schema_migrations (version) VALUES ('%s')", ent->d_name);
                            db_exec(insert_sql);
                        }
                        free(sql);
                    }
                }
            }
        }
        closedir(dir);
    } else {
        perror("Could not open migrations directory");
    }
}


void db_close(void) {
    if (db) {
        sqlite3_close(db);
        db = NULL;
    }
}

int db_exec(const char* sql) {
    char* err_msg = 0;
    int rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
        return 0;
    }
    return 1;
}

sqlite3* db_handle(void) {
    return db;
}

sqlite3_stmt* db_prepare(const char* sql) {
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return NULL;
    }
    return stmt;
}

int db_bind_text(sqlite3_stmt* stmt, int index, const char* text) {
    return sqlite3_bind_text(stmt, index, text, -1, SQLITE_STATIC);
}

int db_bind_int(sqlite3_stmt* stmt, int index, int value) {
    return sqlite3_bind_int(stmt, index, value);
}

int db_step(sqlite3_stmt* stmt) {
    return sqlite3_step(stmt);
}

const unsigned char* db_column_text(sqlite3_stmt* stmt, int index) {
    return sqlite3_column_text(stmt, index);
}

int db_column_int(sqlite3_stmt* stmt, int index) {
    return sqlite3_column_int(stmt, index);
}

void db_finalize(sqlite3_stmt* stmt) {
    sqlite3_finalize(stmt);
}