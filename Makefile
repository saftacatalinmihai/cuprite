CC = gcc
# CC = clang

FACIL_IO_DIR = lib/facil.io

CUPRITE_H = cuprite.h
APP_DIR = app
APP_SOURCES = $(shell find $(APP_DIR) -type f -name '*.c' -o -name '*.h')
BINDIR = bin
EXECUTABLE = $(BINDIR)/cuprite

# 	-fsanitize=leak  # Does not work on macOS. Using the Instruments app instead.
# 	-arch arm64 \
# 	-fsanitize=address \
# 	-O0 \
# 	-O3 \
# -Wall -Wextra \

CFLAGS = \
	-g \
	-std=c11 \
	-I. \
	-I$(APP_DIR) \
	-I$(FACIL_IO_DIR)/lib/facil \
	-I$(FACIL_IO_DIR)/lib/facil/http \
	-I$(FACIL_IO_DIR)/lib/facil/fiobj \

LDFLAGS = -L$(FACIL_IO_DIR)/tmp -Wl,-rpath=$(FACIL_IO_DIR)/tmp -lfacil -lsqlite3

$(FACIL_IO_DIR)/tmp/libfacil.so:
	@if [ ! -d "$(FACIL_IO_DIR)/.git" ]; then \
		echo "Cloning facil.io..."; \
		git clone https://github.com/boazsegev/facil.io $(FACIL_IO_DIR); \
	fi
	@echo "Building facil.io library..."
	@$(MAKE) -C $(FACIL_IO_DIR) lib

$(EXECUTABLE): $(CUPRITE_H) $(APP_SOURCES) $(FACIL_IO_DIR)/tmp/libfacil.so
	@mkdir -p $(BINDIR)
	@echo "Compiling sources into $(EXECUTABLE)..."
	$(CC) $(CFLAGS) -o $@ $(APP_DIR)/main.c $(LDFLAGS)

.DEFAULT_GOAL := compile

compile: $(EXECUTABLE)

start: compile
	@$(EXECUTABLE) run

run: compile
	@$(EXECUTABLE) run

migrate: compile
	@$(EXECUTABLE) migrate

# Usage: make generate_model model=<model_name (e.g., Product)>
generate_model:
	@ruby scripts/generate_model.rb $(model)

clean:
	rm -rf $(BINDIR)
	rm -rf $(FACIL_IO_DIR)
