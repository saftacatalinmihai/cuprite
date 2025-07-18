CC = gcc
FACIL_IO_DIR = lib/facil.io
CFLAGS = -Iinclude -Isrc -Iapp -I$(FACIL_IO_DIR)/lib/facil -I$(FACIL_IO_DIR)/lib/facil/http -I$(FACIL_IO_DIR)/lib/facil/cli -I$(FACIL_IO_DIR)/lib/facil/fiobj -Wall -Wextra -std=gnu11 -include errno.h -D_GNU_SOURCE
LDFLAGS = -L$(FACIL_IO_DIR)/tmp -lfacil -lsqlite3 -lm -lc -lpthread

SRCDIR = src
APPDIR = app
BINDIR = bin
EXECUTABLE = $(BINDIR)/cuprite

SOURCES = $(filter-out $(SRCDIR)/migrate.c, $(shell find $(SRCDIR) -name '*.c')) $(shell find $(APPDIR) -name '*.c')

all: $(EXECUTABLE)

download_facil_io:
	@mkdir -p $(FACIL_IO_DIR)
	@if [ ! -d "$(FACIL_IO_DIR)/.git" ]; then \
		git clone https://github.com/boazsegev/facil.io $(FACIL_IO_DIR); \
	else \
		echo "facil.io already cloned. Skipping."; \
	fi

$(FACIL_IO_DIR)/tmp/libfacil.a: download_facil_io
	$(MAKE) -C $(FACIL_IO_DIR) lib

$(EXECUTABLE): $(SOURCES) $(FACIL_IO_DIR)/tmp/libfacil.a
	@mkdir -p $(BINDIR)
	$(CC) $(CFLAGS) -o $@ $(SOURCES) $(LDFLAGS)

clean:
	rm -rf $(BINDIR)
	$(MAKE) -C $(FACIL_IO_DIR) clean

debug:
	@$(MAKE) clean
	@$(MAKE) CFLAGS="$(CFLAGS) -g" all

start-debug: debug
	gdb $(EXECUTABLE)

migrate:
	@mkdir -p $(BINDIR)
	$(CC) $(CFLAGS) -o $(BINDIR)/migrate src/migrate.c src/db/db.c $(LDFLAGS)
	@$(BINDIR)/migrate
	
.PHONY: all clean test debug start-debug migrate