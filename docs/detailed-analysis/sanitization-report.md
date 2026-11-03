> **AI-generated document — not the author's own work.** Written by Claude (Anthropic, an AI assistant) on 2 Nov 2026 while recovering this project from the author's PC, before the v4 sketch was found. It is kept for reference only. Statements about who wrote which file are superseded by `docs/ai-contributions.md`; current status is in `README.md` and `docs/testing.md`.

# Sanitisation report (2 Nov 2026)

No secret value appears in this archive or in this report. Values were classified by script, and only their name and length were printed.

## Credentials found on the PC
| Original file | Item | Classification | Action |
|---|---|---|---|
| `D:\CE014\SimpleWebServerWiFi\arduino_secrets.h` | Wi-Fi SSID, Wi-Fi password | **Real password** | File excluded; new `arduino_secrets.example.h` with placeholders |
| `D:\SPELLBOUND_Vespera_Test\arduino_secrets.h` (top level) | MQTT user, MQTT password | **Real password** | Excluded |
| `D:\SPELLBOUND_Vespera_Test\SPELLBOUND_Vespera_Test.ino` | Wi-Fi SSID, Wi-Fi password, MQTT user, MQTT password as **string literals in the code** | **Real passwords** | Lines 14–17 replaced by `SECRET_*` macros; no other line changed (verified with `diff`) |
| `D:\SPELLBOUND_Vespera_Test\SPELLBOUND_Vespera_Test\arduino_secrets.h` | SSID + placeholder passwords | Placeholders only | Excluded (redundant with the example) |
| zip `SPELLBOUND_Vespera_Test/arduino_secrets.h` | identical to the example | Placeholders only | Excluded |

**Recommendation:** treat the Wi-Fi and MQTT passwords above as exposed, since they sat in plain sketch files, and change them if they are personal. If they are shared lab credentials, ask the module team before publishing anything that ever contained them.

## Other sensitive or personal information
| Item | Action |
|---|---|
| Windows user folder name in AI reports and build configs | Replaced with `%USERPROFILE%` everywhere (a scan confirms none remain) |
| Edge Impulse project owner `DAIMIAN` and project ID 1149248 in the model metadata | **Kept**. It is the author's own project and identifies the model. Remove it before publishing if you prefer anonymity. |
| Lab SSID name in the v1/v2 example templates | Kept as it was written by the author (a public lab network name, no password) |
| MQTT broker host and Vespera topic | Kept (public course infrastructure, needed to understand the system) |
| IP addresses, e-mail addresses, API keys, tokens, private keys | None found in staged files (automated scan; see `tests/` README for the command) |
| Photos, videos, personal documents | None included. `Downloads\IMG_5640.mov` (5 Oct) was **not opened**; the author may check it for device footage. |

## Repository protection
`.gitignore` blocks `arduino_secrets.h` at every depth, plus build outputs.
