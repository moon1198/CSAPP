#!/bin/bash

make
./csim  -v -s 2 -E 2 -b 3 -t traces/trans.trace | cat > 223.txt
./csim  -v -s 2 -E 4 -b 3 -t traces/trans.trace | cat > 243.txt
./csim-ref  -v -s 2 -E 2 -b 3 -t traces/trans.trace | cat > 223-ref.txt
./csim-ref  -v -s 2 -E 4 -b 3 -t traces/trans.trace | cat > 243-ref.txt
