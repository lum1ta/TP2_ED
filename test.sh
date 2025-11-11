#!/bin/bash

GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m'

for i in {1..3}; do
    echo "=== Test $i ==="
    if diff -q <(./bin/tp2.out < ./examples/input_$i.txt) ./examples/output_$i.txt >/dev/null 2>&1; then
        echo -e "${GREEN}Test $i: PASS${NC}"
    else
        echo -e "${RED}Test $i: FAIL${NC}"
        diff -u <(./bin/tp2.out < ./examples/input_$i.txt) ./examples/output_$i.txt | \
        sed '/\\ No newline at end of file/d' | \
        sed -e "s/^+/${GREEN}&${NC}/" -e "s/^-/$(echo -e $RED)&${NC}/"
    fi
done
