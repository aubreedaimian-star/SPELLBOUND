#!/bin/sh
# Run from repo root:  sh tests/host_logic_tests/run.sh
set -e
D=$(dirname "$0"); cp "$D/../../firmware/history/03_vespera_threshold_v2/SPELLBOUND_Vespera_Test/arduino_secrets.example.h" "$D/stubs/arduino_secrets.h"
g++ -std=gnu++17 -I"$D/stubs" "$D/test_vespera_threshold_logic.cpp" -o /tmp/spellbound_host_test && /tmp/spellbound_host_test
rm -f "$D/stubs/arduino_secrets.h"
