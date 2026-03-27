#ifndef CUPRITE_H
#define CUPRITE_H

#include <dirent.h>
#include <sqlite3.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#include "fio.h"
#include "fiobj.h"
#include "fiobj_mustache.h"
#include "http.h"

// #include "routes.h" // Can't include this here, it causes circular dependencies. It is included in app/main.c

#pragma region Headers Router
typedef void (*controller_action)(http_s *);

void route_get(char *path, controller_action action);
void route_post(char *path, controller_action action);
void route_patch(char *path, controller_action action);
void route_put(char *path, controller_action action);
void route_delete(char *path, controller_action action);
void route_request(http_s *request);
void initialize_routes(void);

#pragma endregion Headers Router

// ----------------------------------------------

#pragma region Headers Controller

extern FIOBJ template_hash;

void url_decode(char *dst, const char *src);
void render(http_s *request, char *view, FIOBJ data);

#pragma endregion

// ----------------------------------------------

#pragma region Headers DB

void db_init_with_filename(const char *db_name);
void db_migrate(void);
void db_thread_close(void);
int db_exec(const char *sql);
sqlite3 *db_handle(void);
sqlite3_stmt *db_prepare(const char *sql);
int db_bind_text(sqlite3_stmt *stmt, int index, const char *text);
int db_bind_int(sqlite3_stmt *stmt, int index, int value);
int db_step(sqlite3_stmt *stmt);
const unsigned char *db_column_text(sqlite3_stmt *stmt, int index);
int db_column_int(sqlite3_stmt *stmt, int index);
void db_finalize(sqlite3_stmt *stmt);

#pragma endregion

// ----------------------------------------------

#pragma region Implementation Router

#define MAX_ROUTES 100

typedef struct {
  char *method;
  char *path;
  controller_action action;
} route_t;

static route_t routes[MAX_ROUTES];
static int route_count = 0;

void route_get(char *path, controller_action action) {
  if (route_count < MAX_ROUTES) {
    routes[route_count].method = "GET";
    routes[route_count].path = path;
    routes[route_count].action = action;
    route_count++;
  }
}

void route_post(char *path, controller_action action) {
  if (route_count < MAX_ROUTES) {
    routes[route_count].method = "POST";
    routes[route_count].path = path;
    routes[route_count].action = action;
    route_count++;
  }
}

void route_patch(char *path, controller_action action) {
  if (route_count < MAX_ROUTES) {
    routes[route_count].method = "PATCH";
    routes[route_count].path = path;
    routes[route_count].action = action;
    route_count++;
  }
}

void route_put(char *path, controller_action action) {
  if (route_count < MAX_ROUTES) {
    routes[route_count].method = "PUT";
    routes[route_count].path = path;
    routes[route_count].action = action;
    route_count++;
  }
}

void route_delete(char *path, controller_action action) {
  if (route_count < MAX_ROUTES) {
    routes[route_count].method = "DELETE";
    routes[route_count].path = path;
    routes[route_count].action = action;
    route_count++;
  }
}

bool send_file(http_s *request, char *path) {
  struct stat buffer;
  if (stat(path, &buffer) == 0) {
    if (S_ISDIR(buffer.st_mode) != 0) {
      return false;
    }
    int fd = open(path, O_RDONLY);
    http_sendfile(request, fd, buffer.st_size, 0);
    close(fd);
    return true;
  } else {
    return false;
  }
}

void route_request(http_s *request) {
  clock_t begin = clock();

  fio_str_info_s path_info = fiobj_obj2cstr(request->path);
  fio_str_info_s method_info = fiobj_obj2cstr(request->method);

  // printf("Route method: %s\n", method_info.data);
  // printf("Request path: %s\n", path_info.data);

  // Check for routes defined in app/routes.c
  for (int i = 0; i < route_count; i++) {
    if (strcmp(method_info.data, routes[i].method) == 0) {
      char *route_path_template = routes[i].path;
      char *request_path = path_info.data;
      char *param_value = NULL;
      int param_int = 0;

      // printf("Route path template: %s\n", route_path_template);

      if (strchr(route_path_template, ':')) {
        char *param_start = strchr(route_path_template, ':');
        // check if the prefix matches
        if (strncmp(request_path, route_path_template,
                    param_start - route_path_template) == 0) {
          param_value = request_path + (param_start - route_path_template);
          // printf("Param value: %s\n", param_value);

          // check if there is a slash in the param
          // check if suffix matches
          char *template_suffix = strchr(param_start, '/');
          char *suffix = strchr(param_value, '/');
          if (template_suffix && !suffix) {
            continue;
          }
          if (suffix && !template_suffix) {
            continue;
          }
          if (suffix && template_suffix) {
            // printf("Template suffix %s\n", template_suffix);
            // printf("Suffix %s\n", suffix);

            if (strncmp(template_suffix, suffix, strlen(template_suffix)) ==
                0) {
              param_value = strtok(param_value, "/");
              // printf("Suffix %s\n", suffix);
            } else {
              continue;
            }
          }
          param_int = atoi(param_value);
        }
      }
      if (param_int > 0) {
        // printf("Param int: %d\n", param_int);
        if (!request->params) {
          request->params = fiobj_hash_new();
        }
        FIOBJ id_str = fiobj_str_new("id", 2);
        fiobj_hash_set(request->params, id_str,
                       fiobj_str_new(param_value, strlen(param_value)));
        // printf("Routing to route: %i\n", i);
        routes[i].action(request);
        fiobj_free(id_str);
        clock_t end = clock();
        double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
        printf("Route duration %f s, %f millis, %f micros, %f nanos\n",
               time_spent, time_spent * 1000, time_spent * 1000000,
               time_spent * 1000000000);
        return;
      } else if (strcmp(request_path, route_path_template) == 0) {
        // printf("Routing to route: %i\n", i);
        routes[i].action(request);
        clock_t end = clock();
        double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
        printf("Route duration %f s, %f millis, %f micros, %f nanos\n",
               time_spent, time_spent * 1000, time_spent * 1000000,
               time_spent * 1000000000);

        return;
      }
    }
  }

  // Check for public files
  if (strcmp(method_info.data, "GET") == 0) {
    char public_file_path_in_folder[128];
    sprintf(public_file_path_in_folder, "public/%s", path_info.data + 1);
    if (send_file(request, public_file_path_in_folder)) {
      return;
    }

    // If no file found, check for index.html in the public folder directly -
    // used for static site generation like hugo.
    sprintf(public_file_path_in_folder, "public/%sindex.html",
            path_info.data + 1);
    http_set_header(request, HTTP_HEADER_CONTENT_TYPE,
                    http_mimetype_find((char *)"html", 4));
    if (send_file(request, public_file_path_in_folder)) {
      return;
    }
  }

  http_send_error(request, 404);
}

#pragma endregion

// ----------------------------------------------

#pragma region Implementation Controller

FIOBJ template_hash;

#define USE_TEMPLATE_HASH 0

void url_decode(char *dst, const char *src) {
  char a, b;
  while (*src) {
    if ((*src == '%') && ((a = src[1]) && (b = src[2])) &&
        (isxdigit(a) && isxdigit(b))) {
      if (a >= 'a')
        a -= 'a' - 'A';
      if (a >= 'A')
        a -= ('A' - 10);
      else
        a -= '0';
      if (b >= 'a')
        b -= 'a' - 'A';
      if (b >= 'A')
        b -= ('A' - 10);
      else
        b -= '0';
      *dst++ = 16 * a + b;
      src += 3;
    } else if (*src == '+') {
      *dst++ = ' ';
      src++;
    } else {
      *dst++ = *src++;
    }
  }
  *dst = '\0';
}

static mustache_s *load_template(char *path) {
#if USE_TEMPLATE_HASH
  FIOBJ key = fiobj_str_new(path, strlen(path));
  FIOBJ template_obj = fiobj_hash_get(template_hash, key);

  if (template_obj) {
    // fprintf(stderr, "Loading cached template: %s\n", path);
    fiobj_free(key);
    return (mustache_s *)(uintptr_t)fiobj_obj2num(template_obj);
  }
#endif

  // fprintf(stderr, "Loading template: %s\n", path);
  mustache_s *template =
      fiobj_mustache_load((fio_str_info_s){.data = path, .len = strlen(path)});
  // fprintf(stderr, "Loaded template: %s\n", path);

#if USE_TEMPLATE_HASH
  if (template) {
    template_obj = fiobj_num_new((intptr_t)template);
    fiobj_hash_set(template_hash, key, template_obj);
  } else {
    fiobj_free(key);
  }
#endif

  return template;
}

void render(http_s *request, char *view, FIOBJ data) {
  char view_path[256];
  snprintf(view_path, sizeof(view_path), "app/views/%s.html", view);

  char layout_path[256];
  snprintf(layout_path, sizeof(layout_path),
           "app/views/layouts/application.html");

  mustache_s *view_template = load_template(view_path);
  mustache_s *layout_template = load_template(layout_path);

  if (!layout_template || !view_template) {
    if (!view_template)
      fprintf(stderr, "Error: Could not load view template: %s\n", view_path);
    if (!layout_template)
      fprintf(stderr, "Error: Could not load layout template: %s\n",
              layout_path);
    http_send_error(request, 500);
    return;
  }

  FIOBJ view_content_obj = fiobj_mustache_build(view_template, data);
  free(view_template);
  fio_str_info_s view_content_str = fiobj_obj2cstr(view_content_obj);
  FIOBJ yield_str = fiobj_str_new("yield", 5);
  fiobj_hash_set(data, yield_str,
                 fiobj_str_new(view_content_str.data, view_content_str.len));

  FIOBJ response_body = fiobj_mustache_build(layout_template, data);
  free(layout_template);
  fiobj_free(view_content_obj);
  fio_str_info_s body = fiobj_obj2cstr(response_body);

  http_set_header(request, HTTP_HEADER_CONTENT_TYPE,
                  http_mimetype_find("html", 4));
  http_send_body(request, body.data, body.len);
  fiobj_free(yield_str);

  fiobj_free(response_body);
}

#pragma endregion

// ----------------------------------------------

#pragma region Implementation DB

static _Thread_local sqlite3 *db = NULL;
static const char *db_name_g = NULL;

void db_init_with_filename(const char *db_name) { db_name_g = db_name; }

void db_thread_close(void) {
  if (db) {
    sqlite3_close(db);
    db = NULL;
  }
}

void db_migrate(void) {
  db_exec("CREATE TABLE IF NOT EXISTS schema_migrations (version TEXT PRIMARY "
          "KEY)");

  DIR *dir;
  struct dirent *ent;
  const char *migrations_path = "db/migrations";

  if ((dir = opendir(migrations_path)) != NULL) {
    while ((ent = readdir(dir)) != NULL) {
      if (strstr(ent->d_name, ".sql")) {
        char migration_path[512];
        snprintf(migration_path, sizeof(migration_path), "%s/%s",
                 migrations_path, ent->d_name);

        // Check if migration is already applied
        char check_sql[512];
        snprintf(check_sql, sizeof(check_sql),
                 "SELECT version FROM schema_migrations WHERE version = '%s'",
                 ent->d_name);
        sqlite3_stmt *check_stmt = db_prepare(check_sql);

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

            if (db_exec(sql)) {
              char insert_sql[512];
              snprintf(insert_sql, sizeof(insert_sql),
                       "INSERT INTO schema_migrations (version) VALUES ('%s')",
                       ent->d_name);
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

int db_exec(const char *sql) {
  char *err_msg = 0;
  int rc = sqlite3_exec(db_handle(), sql, 0, 0, &err_msg);
  if (rc != SQLITE_OK) {
    fprintf(stderr, "SQL error: %s\n", err_msg);
    sqlite3_free(err_msg);
    return 0;
  }
  return 1;
}

sqlite3 *db_handle(void) {
  if (db == NULL) {
    if (sqlite3_open(db_name_g, &db)) {
      fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
      db_thread_close();
      exit(1);
    }
    char *errMsg = 0;
    sqlite3_exec(db, "PRAGMA journal_mode=WAL;", NULL, NULL, &errMsg);
    sqlite3_exec(db, "PRAGMA synchronous=OFF;", NULL, NULL, &errMsg);
  }
  return db;
}

sqlite3_stmt *db_prepare(const char *sql) {
  sqlite3_stmt *stmt;
  if (sqlite3_prepare_v2(db_handle(), sql, -1, &stmt, 0) != SQLITE_OK) {
    fprintf(stderr, "Failed to prepare statement: %s\n",
            sqlite3_errmsg(db_handle()));
    return NULL;
  }
  return stmt;
}

int db_bind_text(sqlite3_stmt *stmt, int index, const char *text) {
  return sqlite3_bind_text(stmt, index, text, -1, SQLITE_STATIC);
}

int db_bind_int(sqlite3_stmt *stmt, int index, int value) {
  return sqlite3_bind_int(stmt, index, value);
}

int db_step(sqlite3_stmt *stmt) { return sqlite3_step(stmt); }

const unsigned char *db_column_text(sqlite3_stmt *stmt, int index) {
  return sqlite3_column_text(stmt, index);
}

int db_column_int(sqlite3_stmt *stmt, int index) {
  return sqlite3_column_int(stmt, index);
}

void db_finalize(sqlite3_stmt *stmt) { sqlite3_finalize(stmt); }

#pragma endregion

// ----------------------------------------------

#pragma region Main Function

int migrate(void);

int migrate(void) {
  printf("Connecting to database...\n");
  db_init_with_filename("cuprite.db");
  printf("Running migrations...\n");
  db_migrate();
  printf("Closing database connection...\n");
  db_thread_close();
  printf("Migrations complete.\n");
  return 0;
}

void on_request(http_s *request) { route_request(request); }

void on_thread_exit(void *arg) {
  (void)arg; // Unused argument
  db_thread_close();
}

// Listen to HTTP requests and start facil.io
int run(int argc, char *argv[]) {
  if (argc <= 1) {
    fprintf(stderr, "Usage: %s [migrate|run|help]\n", argv[0]);
    return 1;
  }
  if (argc > 1 && strcmp(argv[1], "help") == 0) {
    printf("Usage: %s [migrate|run]\n", argv[0]);
    printf("Options:\n");
    printf("  migrate - Run database migrations\n");
    printf("  run     - Start the Cuprite application\n");
    printf("  help    - Show this help message\n");
    return 0;
  }

  switch (argc > 1 ? argv[1][0] : '\0') {
  case 'm':
    if (strcmp(argv[1], "migrate") == 0) {
      return migrate();
    }
    break;
  case 'r':
    if (strcmp(argv[1], "run") == 0) {
      // Continue to run the application
      printf("Starting Cuprite application...\n");
      break;
    }
    break;
  default:
    fprintf(stderr, "Unknown command: %s\n", argv[1]);
    return 1;
  }

  db_init_with_filename("cuprite.db");

  initialize_routes();
  template_hash = fiobj_hash_new();

  fio_state_callback_add(FIO_CALL_ON_FINISH, on_thread_exit, NULL);
  http_listen("3001", NULL, .on_request = on_request, .log = 1);
  fio_start();
  // fio_start(.threads = 1);
  // fio_start(.threads = 2);
  // fio_start(.threads = 16);

  return 0;
}

#pragma endregion

#endif