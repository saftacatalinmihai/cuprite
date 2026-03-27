#!/usr/bin/env bash

perf record -F 99 -a -p $(ps aux | grep cuprite | grep -v grep | awk '{print $2}') -g -- sleep 60

perf script report flamegraph

chromium flamegraph.html

# perf script >out.perf

# ./stackcollapse-perf.pl out.perf >out.folded

# ./flamegraph.pl out.folded >out.svg
# hey -n 10000 -c 50 http://localhost:3001/products