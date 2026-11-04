# Two-minute crit plan

This is a speaking outline based on the project record. Use only the parts that match what you can show live.

## Timed outline

**0:00–0:20 — Idea**  
SPELLBOUND is a gesture-controlled wand for the Vespera light. I was inspired by interactive magic wands and wanted to make the light respond to a deliberate hand movement.

**0:20–0:45 — How it works**  
I press the button, wait through a short cyan preparation cue, then the wand records a two-second motion window. The MPU6050 records six motion axes at 50 Hz. The ESP32 runs the MAGIC Edge Impulse model, then sends RGB frames to Vespera #11 over MQTT.

**0:45–1:10 — Interaction**  
The light gives feedback during use: cyan while capturing, purple when a gesture is recognised, and red when confidence is too low. In the final version, ci turns the light off. The user can also preview effects from Serial Monitor.

**1:10–1:35 — Iteration and learning**  
I started with an MKR WiFi 1010, then moved to an ESP32 after running into an Edge Impulse/CMSIS compile issue. I learned that the board, sample rate, axis order and units have to match the data used to train the model.

**1:35–2:00 — Reflection**  
A classifier confidence score is not the same as real-world accuracy. The project also depends on the physical sensor, Wi-Fi, MQTT and the luminaire all working together. The updated sketch adds timing and saturation diagnostics plus a low-motion filter; I still need to check their readings with the final build, and a threshold that is too high could reject a real gesture. My next check is to save one complete log and video from this exact final sketch.

## Live demonstration order

1. Point out the button, MPU6050 and ESP32.
2. Show the serial connection and wait for `[MQTT] Connected!`.
3. Perform one gesture and show the class/confidence output and Vespera response.
4. Demonstrate `ci` turning the light off.
5. If gesture recognition is unavailable, use `1`–`6` for preview mode and say clearly that this previews the effect without testing classification.
