#!/bin/bash
# Simple script to run cuprite with proper library paths
cd "$(dirname "$0")"
export LD_LIBRARY_PATH="lib/facil.io/tmp:$LD_LIBRARY_PATH"
exec ./bin/cuprite "$@"