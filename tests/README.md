# Tests

| Folder | Kind | Run? | Result |
|---|---|---|---|
| `host_syntax_check/` | Parses each sketch on a PC with stub Arduino headers | Yes, 2 Nov 2026 | 5 PASS; corrupted original FAILs as expected (`RESULT_2026-11-02.txt`) |
| `host_logic_tests/` | Compiles `firmware/history/03` on a PC with an injectable fake I²C bus; checks thresholds, payload and colours | Yes, 2 Nov 2026 | 10/10 PASS (`RESULT_2026-11-02.txt`) |
| `known_sample_test/` | On-board Edge Impulse inference on one fixed recorded sample (AI-written) | **Compile only (AI-reported); never uploaded** | No predictions exist |

The stubs in `host_logic_tests/stubs/` are deliberately simplified fakes. Passing host tests says nothing about timing, I²C electrical behaviour, Wi-Fi, MQTT or the lamp. Hardware tests H1–H7 are listed in `docs/detailed-analysis/testing-report.md` (written by Claude).

The host-side checks and stubs in `host_syntax_check/` and `host_logic_tests/` were written by Claude (Anthropic) during the recovery on 2 Nov 2026. The known-sample test was written by a Codex agent on 29 Oct.

```sh
sh tests/host_syntax_check/run.sh
sh tests/host_logic_tests/run.sh      # needs g++
```

Secret scan used before archiving (prints file names and pattern names only, never matches):
```sh
grep -rIlE 'SECRET_(PASS|MQTTPASS)\s+"[^Y<]' . ; grep -rIlE '(password|passwd|token|api[_-]?key)\s*[:=]\s*"[^"$Y<]{6,}' .
```
