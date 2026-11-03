SPELLBOUND Integrated ESP32 Feather / MPU6050 / Edge Impulse / Vespera #11

1. Copy arduino_secrets.example.h to arduino_secrets.h in this sketch folder.
2. Insert your actual Wi-Fi and MQTT credentials locally; never publish them.
3. Confirm the Edge Impulse header name matches your already-working sketch:
   MAGIC_inferencing.h vs MAGIC2_inferencing.h.
4. Confirm ACCEL_IN_G and axis order match the exact training data and working gesture sketch.
5. Select esp32:esp32:featheresp32, upload, Serial Monitor at 115200 baud.
6. Wait for [MQTT] Connected!; type 1..6 to preview six effects, o to turn off.
7. Type g and perform a gesture during the 2-second capture.

Mappings: 1 Ascendio; 2 ci; 3 incendio; 4 leviosa; 5 oppugno; 6 stupefy.
The model has SIX labels, although you may choose to test five gestures first.
MQTT topic student/CASA0014/luminaire/11, 72 RGB LEDs = 216 raw bytes.
Publish accepted is not proof of physical display; verify in Lumi / Vespera.
No compile/hardware test was run on this generated integrated sketch.
