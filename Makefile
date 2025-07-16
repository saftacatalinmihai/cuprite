CC = gcc
CFLAGS = -Iinclude -Isrc -I$(FACIL_IO_DIR)/lib/facil -I$(FACIL_IO_DIR)/lib/facil/http -I$(FACIL_IO_DIR)/lib/facil/cli -I$(FACIL_IO_DIR)/lib/facil/fiobj -I$(FACIL_IO_DIR)/lib/facil/http/parsers -I$(FACIL_IO_DIR)/lib/facil/legacy -I$(FACIL_IO_DIR)/lib/facil/redis -I$(FACIL_IO_DIR)/lib/facil/tls -Wall -Wextra -std=gnu11 -include errno.h -D_GNU_SOURCE -DFIO_FORCE_MALLOC
FACIL_IO_DIR = lib/facil.io

SRCDIR = src
OBJDIR = obj
BINDIR = bin

SOURCES = $(wildcard $(SRCDIR)/*.c) \
          $(wildcard $(SRCDIR)/db/*.c) \
          $(wildcard $(SRCDIR)/models/generated/*.c) \
          $(wildcard $(SRCDIR)/controllers/*.c) \
          $(FACIL_IO_DIR)/lib/facil/fio.c \
          $(FACIL_IO_DIR)/lib/facil/http/http.c \
          $(FACIL_IO_DIR)/lib/facil/fiobj/fiobject.c \
          $(FACIL_IO_DIR)/lib/facil/fiobj/fiobj_str.c \
          $(FACIL_IO_DIR)/lib/facil/fiobj/fiobj_data.c \
          $(FACIL_IO_DIR)/lib/facil/fiobj/fiobj_numbers.c \
          $(FACIL_IO_DIR)/lib/facil/fiobj/fiobj_hash.c \
          $(FACIL_IO_DIR)/lib/facil/fiobj/fiobj_ary.c \
          $(FACIL_IO_DIR)/lib/facil/http/http_internal.c \
          $(FACIL_IO_DIR)/lib/facil/http/http1.c \
          $(FACIL_IO_DIR)/lib/facil/http/websockets.c \
          $(FACIL_IO_DIR)/lib/facil/cli/fio_cli.c \
          $(FACIL_IO_DIR)/lib/facil/fiobj/fiobj_json.c \
          $(FACIL_IO_DIR)/lib/facil/fiobj/fiobj_mustache.c \
          $(FACIL_IO_DIR)/lib/facil/redis/redis_engine.c \
          $(FACIL_IO_DIR)/lib/facil/tls/fio_tls_missing.c \
          $(FACIL_IO_DIR)/lib/facil/tls/fio_tls_openssl.c

OBJECTS = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(filter $(SRCDIR)/%,$(SOURCES))) \
          $(patsubst $(FACIL_IO_DIR)/lib/%.c,$(OBJDIR)/lib/%.o,$(filter $(FACIL_IO_DIR)/lib/%,$(SOURCES)))
EXECUTABLE = $(BINDIR)/cuprite
MIGRATE_EXECUTABLE = $(BINDIR)/migrate

all: download_facil_io $(EXECUTABLE) $(MIGRATE_EXECUTABLE)

$(MIGRATE_EXECUTABLE): $(OBJDIR)/migrate.o $(filter %db.o,$(OBJECTS))
	@mkdir -p $(BINDIR)
	$(CC) $^ -o $@ -lsqlite3

$(OBJDIR)/migrate.o: scripts/migrate.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -Isrc -c $< -o $@

download_facil_io:
	@mkdir -p $(FACIL_IO_DIR)
	@if [ ! -d "$(FACIL_IO_DIR)/.git" ]; then \
		git clone https://github.com/boazsegev/facil.io $(FACIL_IO_DIR); \
	else \
		echo "facil.io already cloned. Skipping."; \
	fi

$(EXECUTABLE): $(OBJECTS)
	@mkdir -p $(BINDIR)
	$(CC) $(OBJECTS) -o $@ -lm -lc -lsqlite3

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/lib/%.o: $(FACIL_IO_DIR)/lib/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJDIR) $(BINDIR)

test:
	$(CC) $(CFLAGS) -Isrc tests/product_model_test.c src/db/db.c src/models/generated/product.c -lsqlite3 -o bin/product_model_test
	./bin/product_model_test

debug:
	@$(MAKE) clean
	@$(MAKE) CFLAGS="$(CFLAGS) -g" all

start-debug: debug
	gdb $(EXECUTABLE)

.PHONY: all clean migrate test debug start-debug