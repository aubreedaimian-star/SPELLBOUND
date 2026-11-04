# Development notes and serial evidence

This folder contains the author's handwritten planning pages and screenshots from serial-monitor checks. Filenames describe the visible topic. One wand-layout photo has an unclear date, so no date is included in its filename.

## Handwritten project notes

- [19 Oct — early idea branching](2026-10-19-spellbound-idea-branching.jpg)
- [Wand layout and sensor ideas — date unclear](wand-layout-and-sensor-ideas-date-unclear.jpg)
- [19 Oct — Edge Impulse model iterations](2026-10-19-edge-impulse-model-iterations.jpg). The page records 97.2% validation and 94.87% testing for an iteration; the exact model/version and evaluation setup are not identified here, so these figures should not be presented as current model performance.
- [26 Oct — board migration and compile notes](2026-10-26-board-migration-compile-notes.jpg)
- [1 Nov — ESP32, MPU6050 and button wiring](2026-11-01-esp32-mpu6050-button-wiring.jpg)
- [2 Nov — IMU sampling window](2026-11-02-imu-sampling-window.jpg)
- [3 Nov — button and capture state flow](2026-11-03-button-capture-state-flow.jpg)
- [3 Nov — ESP32 to MQTT to Vespera architecture](2026-11-03-esp32-mqtt-vespera-architecture.jpg)

## Serial-monitor screenshots

- [Ascendio and Incendio predictions](../serial_logs/2026-11-04-serial-01-ascendio-incendio.png)
- [Low-confidence fallback followed by Ascendio](../serial_logs/2026-11-04-serial-02-fallback-ascendio.png)
- [Ascendio and Leviosa predictions](../serial_logs/2026-11-04-serial-03-ascendio-leviosa.png)

The screenshots show classifier output from individual runs, not an accuracy study. Some visible runs report that MQTT was disconnected, so those screenshots alone do not demonstrate that the luminaire received the effects.
