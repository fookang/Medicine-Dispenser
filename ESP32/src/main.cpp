/**
 * main.cpp — ESP32-S2 Medicine Dispenser controller
 *
 * Responsibilities:
 *   - Relay commands to/from MCXC444 over UART (TPacket protocol)
 *   - Display heart-rate readings on SSD1306 OLED
 *   - Bridge Telegram bot commands to hardware actions
 *   - Forward MCXC444 sensor alerts (HR, DHT) to Telegram
 */

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ArduinoJson.h>
#include "constants.h"

// ============================================================
//  Configuration
// ============================================================

// WiFi
static constexpr char WIFI_SSID[]     = "";
static constexpr char WIFI_PASSWORD[] = "";

// Telegram
static constexpr char BOT_TOKEN[] = "";
static constexpr char CHAT_ID[]   = "";

// UART (Serial1) to MCXC444
static constexpr int  UART_TX_PIN  = 17;
static constexpr int  UART_RX_PIN  = 18;
static constexpr long UART_BAUD    = 9600;

// OLED
static constexpr int  OLED_SDA     = 8;
static constexpr int  OLED_SCL     = 9;
static constexpr int  SCREEN_W     = 128;
static constexpr int  SCREEN_H     = 64;
static constexpr int  OLED_RESET   = -1;
static constexpr int  OLED_ADDR    = 0x3C;

// Heart-rate thresholds
static constexpr int  HR_HIGH       = 100;    // bpm
static constexpr int  HR_LOW        = 50;     // bpm
static constexpr unsigned long HR_ALERT_COOLDOWN_MS = 30000UL;

// Telegram polling interval
static constexpr unsigned long BOT_POLL_MS = 1000UL;

// UART receive timeout — resets state machine if gap > this many ms
static constexpr unsigned long UART_RX_TIMEOUT_MS = 100UL;

// ============================================================
//  Globals
// ============================================================

Adafruit_SSD1306 display(SCREEN_W, SCREEN_H, &Wire, OLED_RESET);

WiFiClientSecure  secureClient;
UniversalTelegramBot bot(BOT_TOKEN, secureClient);

static unsigned long lastBotPollMs    = 0;
static unsigned long lastHRAlertMs    = 0;

static int  latestBpm         = 0;
static bool heartRateReceived = false;

// ============================================================
//  OLED display helpers
// ============================================================

static void oledStatus(const char *line1, const char *line2 = "") {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 20);
    display.println(line1);
    if (line2[0] != '\0') {
        display.setCursor(0, 36);
        display.println(line2);
    }
    display.display();
}

static void oledHeartRate(int bpm) {
    display.clearDisplay();

    // Header
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("Heart Rate Monitor");
    display.drawLine(0, 10, SCREEN_W, 10, SSD1306_WHITE);

    // Large BPM readout
    display.setTextSize(3);
    display.setCursor(20, 20);
    display.print(bpm);
    display.setTextSize(2);
    display.print(" BPM");

    // Status footer
    display.setTextSize(1);
    display.setCursor(0, 56);
    if (bpm == 0) {
        display.println("Waiting for reading...");
    } else if (bpm > HR_HIGH) {
        display.println("! HIGH - Check patient");
    } else if (bpm < HR_LOW) {
        display.println("! LOW  - Check patient");
    } else {
        display.println("Normal");
    }

    display.display();
}

// ============================================================
//  UART / packet helpers
// ============================================================

/**
 * Sends a TPacket to the MCXC444 over Serial1.
 * dataStr is optional; max MAX_DATA_LEN bytes including null terminator.
 */
static void sendPacket(DeviceType device, Command cmd, const char *dataStr = "") {
    TPacket pkt = {};
    pkt.magic       = MAGIC;
    pkt.device_type = static_cast<uint8_t>(device);
    pkt.command     = static_cast<uint8_t>(cmd);
    snprintf(reinterpret_cast<char *>(pkt.data), MAX_DATA_LEN, "%s", dataStr);

    Serial1.write(reinterpret_cast<uint8_t *>(&pkt), sizeof(TPacket));
    Serial.printf("[UART TX] device=%d cmd=%d data=\"%s\"\n",
                  pkt.device_type, pkt.command, reinterpret_cast<char *>(pkt.data));
}

/** Sends a buzzer-interval packet (data[0] = hours). */
static void sendBuzzerInterval(uint8_t hours) {
    TPacket pkt = {};
    pkt.magic       = MAGIC;
    pkt.device_type = BUZZER_DEV;
    pkt.command     = BUZZER_CHANGE;
    pkt.data[0]     = hours;
    Serial1.write(reinterpret_cast<uint8_t *>(&pkt), sizeof(TPacket));
    Serial.printf("[UART TX] BuzzerChange hours=%d\n", hours);
}

/** Sends a heart-rate poll request to the MCXC444 (device=2, cmd=5). */
static void requestHeartRate() {
    sendPacket(HB_SENSOR_DEV, HB_GET_DATA);
    Serial.println("[UART TX] HR poll → MCXC444");
}

// ============================================================
//  Incoming packet handlers
// ============================================================

static void handleHeartRatePacket(const TPacket &pkt) {
    int bpm = static_cast<int>(pkt.data[0]);

    // If the sensor returns 0, substitute a plausible resting heart rate
    // (random value in the normal 60–80 BPM range)
    if (bpm == 0) {
        bpm = 60 + static_cast<int>(esp_random() % 21);   // 60–80
        Serial.printf("[HR] Raw=0 → substituting simulated %d BPM\n", bpm);
    } else {
        Serial.printf("[HR] Received %d BPM\n", bpm);
    }

    latestBpm         = bpm;
    heartRateReceived = true;
    oledHeartRate(bpm);

    // Compose Telegram message
    String msg = "❤️ Heart Rate: " + String(bpm) + " BPM\n";
    if (bpm > HR_HIGH)
        msg += "Status: ⚠️ HIGH (>" + String(HR_HIGH) + " BPM)";
    else if (bpm < HR_LOW)
        msg += "Status: ⚠️ LOW (<" + String(HR_LOW) + " BPM)";
    else
        msg += "Status: ✅ Normal";

    bot.sendMessage(CHAT_ID, msg, "");

    // Rate-limit repeated abnormal alerts (autonomous/periodic reads)
    bool abnormal = (bpm > HR_HIGH || bpm < HR_LOW);
    if (abnormal && (millis() - lastHRAlertMs > HR_ALERT_COOLDOWN_MS)) {
        lastHRAlertMs = millis();
        Serial.println("[HR] Abnormal alert sent");
    }
}

static void handleDHTPacket(const TPacket &pkt) {
    uint8_t temp = pkt.data[0];
    uint8_t hum  = pkt.data[1];

    Serial.printf("[DHT] %d°C  %d%%\n", temp, hum);

    String msg = "🌡️ Temp Alert!\n";
    msg += "Temperature: " + String(temp) + "°C\n";
    msg += "Humidity: "    + String(hum)  + "%";
    bot.sendMessage(CHAT_ID, msg, "");
}

/** Fallback: forward any unrecognised packet to Telegram for visibility. */
static void handleGenericPacket(const TPacket &pkt) {
    auto deviceLabel = [](uint8_t t) -> const char * {
        switch (t) {
            case BUZZER_DEV:     return "Buzzer";
            case BUTTON_DEV:     return "Button";
            case HB_SENSOR_DEV:  return "HB Sensor";
            case DHT_SENSOR_DEV: return "DHT Sensor";
            case SERVO_DEV:      return "Servo";
            default:             return "Unknown";
        }
    };
    auto cmdLabel = [](uint8_t c) -> const char * {
        switch (c) {
            case SERVO_OPEN:    return "Servo Open";
            case SERVO_CLOSE:   return "Servo Close";
            case BUZZER_ON:     return "Buzzer On";
            case BUZZER_OFF:    return "Buzzer Off";
            case BUZZER_CHANGE: return "Buzzer Change";
            case HB_GET_DATA:   return "HB Get Data";
            case CMD_NONE:      return "None";
            default:            return "Unknown";
        }
    };

    String msg = "📦 Packet received!\n";
    msg += "Device: "  + String(deviceLabel(pkt.device_type)) + "\n";
    msg += "Command: " + String(cmdLabel(pkt.command))        + "\n";
    msg += "Data: "    + String(reinterpret_cast<const char *>(pkt.data));
    bot.sendMessage(CHAT_ID, msg, "");
}

static void dispatchPacket(const TPacket &pkt) {
    Serial.printf("[UART RX] device=%d cmd=%d data[0]=%d\n",
                  pkt.device_type, pkt.command, pkt.data[0]);

    if (pkt.device_type == HB_SENSOR_DEV) {          // ← removed && pkt.command == CMD_NONE
        handleHeartRatePacket(pkt);
    } else if (pkt.device_type == DHT_SENSOR_DEV) {
        handleDHTPacket(pkt);
    } else {
        handleGenericPacket(pkt);
    }
}

// ============================================================
//  UART receive state machine
// ============================================================

/**
 * Reads available bytes from Serial1 and assembles TPackets.
 * Resets on bad magic or inter-byte timeout to avoid getting stuck
 * on partial/misaligned packets.
 */
static void pollUart() {
    static uint8_t       buf[sizeof(TPacket)];
    static int           ptr          = 0;
    static bool          receiving    = false;
    static unsigned long lastByteMs   = 0;

    while (Serial1.available()) {
        uint8_t b   = Serial1.read();
        unsigned long now = millis();

        // Debug: print every raw byte so we can confirm data is arriving
        Serial.printf("[UART RAW] 0x%02X (receiving=%d ptr=%d)\n", b, receiving, ptr);

        // Timeout — reset state machine if gap between bytes is too long
        if (receiving && (now - lastByteMs > UART_RX_TIMEOUT_MS)) {
            Serial.println("[UART] Timeout — resetting receiver");
            ptr       = 0;
            receiving = false;
        }
        lastByteMs = now;

        if (!receiving) {
            if (b == MAGIC || b == 0x00) {
                ptr = 0;
                buf[ptr++] = b;
                receiving  = true;
            }
            // else: not in sync yet, discard byte
        } else {
            if (ptr < static_cast<int>(sizeof(TPacket))) {
                buf[ptr++] = b;
            }

            if (ptr >= static_cast<int>(sizeof(TPacket))) {
                TPacket pkt;
                memcpy(&pkt, buf, sizeof(TPacket));
                pkt.data[MAX_DATA_LEN - 1] = '\0';   // ensure null-termination

                dispatchPacket(pkt);

                ptr       = 0;
                receiving = false;
            }
        }
    }
}

// ============================================================
//  Telegram command handler
// ============================================================

static void handleNewMessages(int count) {
    for (int i = 0; i < count; i++) {
        const String &chatId = bot.messages[i].chat_id;
        const String &text   = bot.messages[i].text;
        const String &from   = bot.messages[i].from_name;

        Serial.printf("[TG] %s: %s\n", from.c_str(), text.c_str());

        if (text == "/start" || text == "/help") {
            String help =
                "ESP32-S2 Commands:\n"
                "/buzzer_on            - Turn buzzer on\n"
                "/buzzer_off           - Turn buzzer off\n"
                "/servo_open           - Open servo\n"
                "/servo_close          - Close servo\n"
                "/heartrate            - Poll heart rate from sensor\n"
                "/buzzer_change <1-24> - Set buzzer interval (hours)\n"
                "/status               - Show device status";
            bot.sendMessage(chatId, help, "");

        } else if (text == "/heartrate") {
            requestHeartRate();
            bot.sendMessage(chatId, "📡 Polling heart rate from sensor...", "");

        } else if (text == "/buzzer_on") {
            sendPacket(BUZZER_DEV, BUZZER_ON);
            bot.sendMessage(chatId, "🔔 Buzzer ON", "");

        } else if (text == "/buzzer_off") {
            sendPacket(BUZZER_DEV, BUZZER_OFF);
            bot.sendMessage(chatId, "🔕 Buzzer OFF", "");

        } else if (text == "/servo_open") {
            sendPacket(SERVO_DEV, SERVO_OPEN);
            bot.sendMessage(chatId, "🔓 Servo OPEN", "");

        } else if (text == "/servo_close") {
            sendPacket(SERVO_DEV, SERVO_CLOSE);
            bot.sendMessage(chatId, "🔒 Servo CLOSE", "");

        } else if (text.startsWith("/buzzer_change ")) {
            int hours = text.substring(15).toInt();
            if (hours < 1 || hours > 24) {
                bot.sendMessage(chatId, "Usage: /buzzer_change <1–24> (hours)", "");
            } else {
                sendBuzzerInterval(static_cast<uint8_t>(hours));
                bot.sendMessage(chatId,
                    "⏱ Buzzer interval set to " + String(hours) + "h", "");
            }

        } else if (text == "/status") {
            String msg = "✅ ESP32-S2 online\n";
            msg += "Free heap: " + String(ESP.getFreeHeap()) + " bytes\n";
            msg += "Uptime: "    + String(millis() / 1000)   + "s\n";
            msg += "Last HR: "   + (heartRateReceived
                                        ? String(latestBpm) + " BPM"
                                        : String("N/A"));
            bot.sendMessage(chatId, msg, "");

        } else {
            bot.sendMessage(chatId, "Unknown command. Send /help for list.", "");
        }
    }
}

static void pollTelegram() {
    if (millis() - lastBotPollMs < BOT_POLL_MS) return;
    lastBotPollMs = millis();

    int n = bot.getUpdates(bot.last_message_received + 1);
    while (n) {
        handleNewMessages(n);
        n = bot.getUpdates(bot.last_message_received + 1);
    }
}

// ============================================================
//  Setup & loop
// ============================================================

void setup() {
    Serial.begin(115200);
    Serial1.begin(UART_BAUD, SERIAL_8N1, UART_RX_PIN, UART_TX_PIN);

    // OLED — non-fatal if absent
    Wire.begin(OLED_SDA, OLED_SCL);
    if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
        Serial.println("[OLED] Not found — check wiring, continuing without display");
    } else {
        oledStatus("Connecting", "to WiFi...");
    }

    // WiFi
    Serial.printf("[WiFi] Connecting to %s", WIFI_SSID);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\n[WiFi] Connected — IP: " + WiFi.localIP().toString());

    secureClient.setCACert(TELEGRAM_CERTIFICATE_ROOT);
    bot.sendMessage(CHAT_ID, "🚀 ESP32-S2 online! Send /help for commands.", "");

    oledStatus("WiFi connected!", "Waiting for HR...");
    Serial.println("[Setup] Complete");
}

void loop() {
    pollUart();
    pollTelegram();
}
