#!/bin/sh
for file in `find . -type f -name "*.h" -o -name "*.c"`
do
    clang-format -style=file -i "$file"
done
