#!/bin/sh
# Host-side SYNTAX check with stub headers (tests/host_logic_tests/stubs).
# This only proves the C++ parses against simplified APIs. It is NOT an
# Arduino/SAMD/ESP32 build and says nothing about hardware behaviour.
ROOT=$(cd "$(dirname "$0")/../.." && pwd); STUBS="$ROOT/tests/host_logic_tests/stubs"
chk(){ d=$(dirname "$1"); t=$(mktemp -d); cp "$d"/* "$t"/ 2>/dev/null
  [ -f "$t/arduino_secrets.example.h" ] && cp "$t/arduino_secrets.example.h" "$t/arduino_secrets.h"
  cp "$1" "$t/x.cpp"; sed -i '1i #include <Arduino.h>' "$t/x.cpp"
  if g++ -std=gnu++17 -fsyntax-only -I"$STUBS" -I"$t" "$t/x.cpp" 2>"$t/e"; then echo "PASS $2"; else echo "FAIL $2"; head -4 "$t/e" | sed "s|$t/||"; fi; rm -rf "$t"; }
chk "$ROOT/firmware/history/01_webled_mkr1010/SimpleWebServerWiFi/SimpleWebServerWiFi.ino" webled
chk "$ROOT/firmware/history/02_vespera_mqtt_test_v1/SPELLBOUND_Vespera_Test/SPELLBOUND_Vespera_Test.ino" vespera_v1
chk "$ROOT/firmware/history/03_vespera_threshold_v2/SPELLBOUND_Vespera_Test/SPELLBOUND_Vespera_Test.ino" vespera_v2
chk "$ROOT/firmware/history/04_datacollector_mkr1010_repaired/SPELLBOUND_DataCollector/SPELLBOUND_DataCollector.ino" datacollector_mkr1010_repaired
chk "$ROOT/firmware/history/05_datacollector_esp32/sketch_nov1a/sketch_nov1a.ino" datacollector_esp32
chk "$ROOT/evidence/original_as_found/SPELLBOUND_DataCollector.CORRUPTED_AS_FOUND.ino.txt" datacollector_CORRUPTED_original_expected_FAIL
