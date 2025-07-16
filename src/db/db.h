#ifndef DB_H
#define DB_H

#include <sqlite3.h>

void db_init_with_filename(const char* db_name);
void db_migrate(void);
void db_thread_close(void);
int db_exec(const char* sql);
sqlite3* db_handle(void);
sqlite3_stmt* db_prepare(const char* sql);
int db_bind_text(sqlite3_stmt* stmt, int index, const char* text);
int db_bind_int(sqlite3_stmt* stmt, int index, int value);
int db_step(sqlite3_stmt* stmt);
const unsigned char* db_column_text(sqlite3_stmt* stmt, int index);
int db_column_int(sqlite3_stmt* stmt, int index);
void db_finalize(sqlite3_stmt* stmt);

#endif