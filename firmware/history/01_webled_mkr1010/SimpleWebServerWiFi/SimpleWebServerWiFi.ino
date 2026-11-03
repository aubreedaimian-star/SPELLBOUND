
#include <SPI.h>
#include <WiFiNINA.h>
#include "arduino_secrets.h"

// Wi-Fi credentials are stored in arduino_secrets.h
char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;

WiFiServer server(80);

const int LED_PIN = LED_BUILTIN;
bool ledState = false;

// ---------- HTML WEB INTERFACE ----------
const char WEB_PAGE[] = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport"
      content="width=device-width, initial-scale=1">
<title>LUMEN | IoT Control</title>

<style>
:root {
  --bg: #10151f;
  --card: #1c2533;
  --text: #f5f7fc;
  --muted: #91a0b3;
  --accent: #5de0cb;
}

* { box-sizing: border-box; }

body {
  margin: 0;
  min-height: 100vh;
  background: var(--bg);
  color: var(--text);
  font-family: -apple-system, BlinkMacSystemFont,
               "Segoe UI", sans-serif;
  display: flex;
  justify-content: center;
  align-items: center;
  padding: 20px;
}

.dashboard {
  width: 100%;
  max-width: 420px;
  background: #151d29;
  border: 1px solid #2b3646;
  border-radius: 28px;
  padding: 28px;
  box-shadow: 0 25px 70px #0005;
}

.header {
  display: flex;
  justify-content: space-between;
  align-items: center;
}

.brand {
  font-size: 23px;
  font-weight: 750;
  letter-spacing: 4px;
}

.subtitle {
  color: var(--muted);
  font-size: 10px;
  letter-spacing: 2px;
  margin-top: 6px;
}

.online {
  color: var(--accent);
  background: #193c3b;
  border-radius: 30px;
  padding: 8px 12px;
  font-size: 12px;
}

.visual {
  text-align: center;
  padding: 55px 0 35px;
}

.bulb {
  width: 150px;
  height: 150px;
  margin: auto;
  display: flex;
  align-items: center;
  justify-content: center;
  border-radius: 50%;
  background: #25303e;
  transition: all .4s ease;
}

.bulb svg {
  width: 72px;
  height: 72px;
  fill: none;
  stroke: #64748b;
  stroke-width: 1.7;
  transition: all .4s ease;
}

.bulb.active {
  background: #214944;
  box-shadow: 0 0 75px #5de0cb33;
}

.bulb.active svg {
  stroke: var(--accent);
  filter: drop-shadow(0 0 12px #5de0cb);
}

h1 {
  font-size: 26px;
  margin: 32px 0 8px;
}

.device {
  color: var(--muted);
  font-size: 14px;
}

.status {
  color: var(--muted);
  margin-top: 22px;
  font-size: 14px;
}

.status.active {
  color: var(--accent);
}

.control {
  background: var(--card);
  border-radius: 18px;
  padding: 22px;
  display: flex;
  justify-content: space-between;
  align-items: center;
}

.control-title {
  font-weight: 600;
}

.control-sub {
  color: var(--muted);
  font-size: 12px;
  margin-top: 6px;
}

.toggle {
  width: 60px;
  height: 34px;
  border: 0;
  border-radius: 30px;
  background: #475569;
  padding: 4px;
  cursor: pointer;
  transition: background .3s;
}

.toggle span {
  display: block;
  width: 26px;
  height: 26px;
  border-radius: 50%;
  background: white;
  transition: transform .3s;
}

.toggle.active {
  background: #159b87;
}

.toggle.active span {
  transform: translateX(26px);
}

.toggle:disabled {
  opacity: .5;
  cursor: wait;
}

.toggle:focus-visible {
  outline: 3px solid var(--accent);
  outline-offset: 4px;
}

.footer {
  display: flex;
  justify-content: space-between;
  color: #7c8da3;
  font-size: 10px;
  letter-spacing: 1px;
  margin-top: 28px;
}
</style>
</head>

<body>
<main class="dashboard">

  <header class="header">
    <div>
      <div class="brand">LUMEN</div>
      <div class="subtitle">
        CONNECTED ENVIRONMENTS
      </div>
    </div>
    <div class="online" id="connection">
      &#9679; CONNECTED
    </div>
  </header>

  <section class="visual">
    <div class="bulb" id="bulb">
      <svg viewBox="0 0 64 64">
        <path d="M22 44h20M24 50h16M28 56h8"/>
        <path d="M32 6c-12 0-20 9-20 20
                 0 8 5 13 10 18h20
                 c5-5 10-10 10-18
                 0-11-8-20-20-20z"/>
      </svg>
    </div>

    <h1>Light Control</h1>
    <div class="device">
      Arduino MKR WiFi 1010
    </div>
    <div class="status" id="status">
      Checking status...
    </div>
  </section>

  <section class="control">
    <div>
      <div class="control-title">
        Built-in LED
      </div>
      <div class="control-sub">
        Remote power control
      </div>
    </div>

    <button class="toggle"
            id="toggle"
            type="button"
            aria-label="Toggle LED"
            aria-pressed="false"
            disabled>
      <span></span>
    </button>
  </section>

  <footer class="footer">
    <span>LOCAL DEVICE / HTTP</span>
    <span id="footerStatus">SYSTEM READY</span>
  </footer>

</main>

<script>
let ledOn = false;
let busy = false;

const toggle = document.getElementById("toggle");
const bulb = document.getElementById("bulb");
const statusText = document.getElementById("status");
const connection = document.getElementById("connection");
const footer = document.getElementById("footerStatus");

function render(on) {
  ledOn = on;
  bulb.classList.toggle("active", on);
  toggle.classList.toggle("active", on);
  statusText.classList.toggle("active", on);
  statusText.textContent =
    on ? "● Light is ON" : "○ Light is OFF";
  toggle.setAttribute("aria-pressed", String(on));
}

async function syncStatus() {
  if (busy) return;

  try {
    const response = await fetch("/api/status",
      {cache: "no-store"});

    if (!response.ok) throw new Error("HTTP error");

    const data = await response.json();
    render(data.led);

    connection.textContent = "● CONNECTED";
    footer.textContent = "SYSTEM READY";
    toggle.disabled = false;
  } catch (error) {
    connection.textContent = "● OFFLINE";
    footer.textContent = "CONNECTION LOST";
    statusText.textContent = "Unable to reach device";
    toggle.disabled = true;
  }
}

toggle.addEventListener("click", async () => {
  if (busy) return;

  busy = true;
  toggle.disabled = true;

  const target = !ledOn;

  try {
    const response = await fetch(
      target ? "/api/on" : "/api/off",
      {cache: "no-store"});

    if (!response.ok) throw new Error("HTTP error");

    const data = await response.json();
    render(data.led);
  } catch (error) {
    statusText.textContent = "Control failed";
  } finally {
    busy = false;
    await syncStatus();
  }
});

syncStatus();
setInterval(syncStatus, 2000);
</script>
</body>
</html>
)rawliteral";

// ---------- SEND HTTP RESPONSE ----------
void sendResponse(WiFiClient &client,
                  const char *contentType,
                  const char *body) {
  client.println("HTTP/1.1 200 OK");
  client.print("Content-Type: ");
  client.println(contentType);
  client.println("Cache-Control: no-store");
  client.println("Connection: close");
  client.println();
  client.print(body);
}

void sendStatus(WiFiClient &client) {
  sendResponse(
    client,
    "application/json",
    ledState ? "{\"led\":true}" : "{\"led\":false}"
  );
}

// ---------- SETUP ----------
void setup() {
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  Serial.begin(9600);

  // Do not wait indefinitely for Serial Monitor.
  unsigned long serialStart = millis();
  while (!Serial && millis() - serialStart < 3000) {
    delay(10);
  }

  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("WiFi module not found");
    while (true) {
      delay(1000);
    }
  }

  Serial.print("Connecting to ");
  Serial.println(ssid);

  while (WiFi.status() != WL_CONNECTED) {
    WiFi.begin(ssid, pass);
    delay(5000);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi connected!");

  server.begin();

  Serial.print("Open browser: http://");
  Serial.println(WiFi.localIP());
}

// ---------- MAIN LOOP ----------
void loop() {
  WiFiClient client = server.available();

  if (!client) return;

  client.setTimeout(1000);

  String request = client.readStringUntil('\n');
  request.trim();

  // Consume remaining request headers.
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    line.trim();
    if (line.length() == 0) break;
  }

  Serial.println(request);

  if (request.startsWith("GET /api/on ")) {
    ledState = true;
    digitalWrite(LED_PIN, HIGH);
    sendStatus(client);

  } else if (request.startsWith("GET /api/off ")) {
    ledState = false;
    digitalWrite(LED_PIN, LOW);
    sendStatus(client);

  } else if (request.startsWith("GET /api/status ")) {
    sendStatus(client);

  } else if (request.startsWith("GET / ")) {
    sendResponse(client, "text/html; charset=UTF-8",
                 WEB_PAGE);

  } else {
    client.println("HTTP/1.1 404 Not Found");
    client.println("Connection: close");
    client.println();
  }

  delay(1);
  client.stop();
}
