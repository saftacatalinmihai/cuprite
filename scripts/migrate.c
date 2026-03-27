#include "db/db.h"
#include <stdio.h>

int main(void) {
    printf("Connecting to database...\n");
    db_init_with_filename("cuprite.db");
    printf("Running migrations...\n");
    db_migrate();
    printf("Closing database connection...\n");
    db_thread_close();
    printf("Migrations complete.\n");
    return 0;
}