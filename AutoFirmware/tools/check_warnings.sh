#!/usr/bin/env bash
# Strict-warning gate for our own code in src/.
# Any warning counts as an error. main.cpp and Imu.cpp are skipped (need Arduino).
# Run from AutoFirmware/:   bash tools/check_warnings.sh
set -u

CXX="${CXX:-c++}"
FLAGS=(-std=gnu++11 -Iinclude -fsyntax-only
       -Wall -Wextra -Wpedantic -Werror
       -Wconversion -Wsign-conversion -Wshadow -Wdouble-promotion
       -Wfloat-equal -Wcast-qual -Wundef -Wformat=2 -Wmissing-declarations)

status=0
for f in src/*.cpp; do
    if [ "$f" = "src/main.cpp" ] || [ "$f" = "src/Imu.cpp" ]; then
        continue
    fi
    if ! "$CXX" "${FLAGS[@]}" "$f"; then
        status=1
    fi
done

if [ "$status" -eq 0 ]; then
    echo "check_warnings: no warnings in src/"
else
    echo "check_warnings: FAILED, fix the warnings above"
fi
exit "$status"