#!/bin/bash

# Unity build script for Cuprite project
# This script compiles all source files into a single binary.
# Usage: ./unity_build.sh
set -e

# Define directories
SRC_DIR="./src"
APP_DIR="./app"
BINDIR="bin"
FACIL_IO_DIR="./lib/facil.io"
FACIL_IO_LIB_DIR="$FACIL_IO_DIR/lib/facil"

# Create binary directory if it doesn't exist
mkdir -p $BINDIR

# Define compiler and flags
CC="gcc"
CFLAGS="-Wall -Wextra -std=gnu11 -include errno.h -D_GNU_SOURCE -I$SRC_DIR -I$APP_DIR -I$FACIL_IO_LIB_DIR -I$FACIL_IO_LIB_DIR/cli -I$FACIL_IO_LIB_DIR/fiobj -I$FACIL_IO_LIB_DIR/http -I$FACIL_IO_LIB_DIR/tls"
LDFLAGS="-L$FACIL_IO_DIR/tmp -lfacil -lsqlite3 -lm -lc -lpthread"

# Find all source files in the src and app directories
SOURCES=$(find $SRC_DIR $APP_DIR -name "*.c")

# Check if any source files were found
if [ -z "$SOURCES" ]; then
    echo "No source files found in $SRC_DIR or $APP_DIR"
    exit 1
fi

# Add all source files to be included in the unity build, except for main.c and migrate.c
for file in $SOURCES; do
    if [[ "$file" != "./src/main.c" && "$file" != "./src/migrate.c" ]]; then
        CFLAGS="$CFLAGS -include $file"
    fi
done

# Compile the sources into a single binary
OUTPUT="$BINDIR/cuprite"
echo "Compiling sources into $OUTPUT..."
echo "Running: $CC $CFLAGS -o $OUTPUT src/main.c $LDFLAGS"
$CC $CFLAGS -o $OUTPUT src/main.c $LDFLAGS

echo "Build completed successfully. Binary is located at $OUTPUT"