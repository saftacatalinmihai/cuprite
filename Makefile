CC = gcc
FACIL_IO_DIR = lib/facil.io
CFLAGS = -Iinclude -Isrc -I$(FACIL_IO_DIR)/lib/facil -I$(FACIL_IO_DIR)/lib/facil/http -I$(FACIL_IO_DIR)/lib/facil/cli -I$(FACIL_IO_DIR)/lib/facil/fiobj -Wall -Wextra -std=gnu11 -include errno.h -D_GNU_SOURCE
LDFLAGS = -L$(FACIL_IO_DIR)/tmp -lfacil -lsqlite3 -lm -lc -lpthread

SRCDIR = src
BINDIR = bin

SOURCES = $(shell find $(SRCDIR) -name '*.c')
EXECUTABLE = $(BINDIR)/cuprite

all: download_facil_io $(FACIL_IO_DIR)/tmp/libfacil.a $(EXECUTABLE)

download_facil_io:
	@mkdir -p $(FACIL_IO_DIR)
	@if [ ! -d "$(FACIL_IO_DIR)/.git" ]; then \
		git clone https://github.com/boazsegev/facil.io $(FACIL_IO_DIR); \
	else \
		echo "facil.io already cloned. Skipping."; \
	fi

$(FACIL_IO_DIR)/tmp/libfacil.a:
	$(MAKE) -C $(FACIL_IO_DIR) lib

$(EXECUTABLE): $(SOURCES)
	@mkdir -p $(BINDIR)
	$(CC) $(CFLAGS) -o $@ $(SOURCES) $(LDFLAGS)

clean:
	rm -rf $(BINDIR)
	$(MAKE) -C $(FACIL_IO_DIR) clean

test:
	$(CC) $(CFLAGS) -Isrc tests/product_model_test.c src/db/db.c src/models/generated/product.c -lsqlite3 -o bin/product_model_test && ./bin/product_model_test

debug:
	@$(MAKE) clean
	@$(MAKE) CFLAGS="$(CFLAGS) -g" all

start-debug: debug
	gdb $(EXECUTABLE)

migrate:
	@mkdir -p $(BINDIR)
	$(CC) $(CFLAGS) -o $(BINDIR)/migrate src/migrate.c src/db/db.c $(LDFLAGS)

.PHONY: all clean test debug start-debug migrate