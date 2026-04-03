#include "esp_camera.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "config.h"
#include "camera_config.h"
#include "telegram_notify.h"

#define PIR_PIN            13
#define LED_PIN             4
#define LED_STATUS         33

#define MOTION_COOLDOWN_MS  10000
#define WIFI_RETRY_COUNT       20
#define CAPTURE_DELAY_MS      500

bool motionDetected       = false;
unsigned long lastAlertTime = 0;
int alertCount            = 0;

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

  Serial.begin(115200);
  Serial.println("\n[BOOT] IoT Anti-Theft Detection System");

  pinMode(PIR_PIN,    INPUT);
  pinMode(LED_PIN,    OUTPUT);
  pinMode(LED_STATUS, OUTPUT);
  digitalWrite(LED_PIN,    LOW);
  digitalWrite(LED_STATUS, LOW);

  if (!initCamera()) {
    Serial.println("[ERROR] Camera init failed — rebooting in 5s");
    delay(5000);
    ESP.restart();
  }
  Serial.println("[OK] Camera initialised");

  connectWiFi();

  blinkStatus(3, 200);
  Serial.println("[READY] System armed. Monitoring for motion...");
}

void loop() {
  int pirState = digitalRead(PIR_PIN);

  if (pirState == HIGH) {
    unsigned long now = millis();

    if (now - lastAlertTime > MOTION_COOLDOWN_MS) {
      Serial.println("[ALERT] Motion detected!");
      lastAlertTime = now;
      alertCount++;

      digitalWrite(LED_STATUS, HIGH);
      delay(CAPTURE_DELAY_MS);

      camera_fb_t* fb = captureImage();
      if (fb) {
        Serial.printf("[INFO] Image captured (%u bytes)\n", fb->len);
        sendTelegramAlert(fb, alertCount);
        esp_camera_fb_return(fb);
      } else {
        Serial.println("[WARN] Capture failed — sending text alert only");
        sendTelegramTextAlert(alertCount);
      }

      digitalWrite(LED_STATUS, LOW);
    } else {
      Serial.println("[INFO] Motion suppressed (cooldown active)");
    }
  }

  delay(100);
}

camera_fb_t* captureImage() {
  digitalWrite(LED_PIN, HIGH);
  delay(100);
  camera_fb_t* fb = esp_camera_fb_get();
  digitalWrite(LED_PIN, LOW);

  if (!fb) {
    Serial.println("[ERROR] esp_camera_fb_get() returned NULL");
    return nullptr;
  }
  return fb;
}

void connectWiFi() {
  Serial.printf("[WIFI] Connecting to %s", WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  int retries = 0;
  while (WiFi.status() != WL_CONNECTED && retries < WIFI_RETRY_COUNT) {
    delay(500);
    Serial.print(".");
    retries++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.printf("\n[WIFI] Connected. IP: %s\n", WiFi.localIP().toString().c_str());
    blinkStatus(2, 100);
  } else {
    Serial.println("\n[WIFI] Failed — continuing offline");
  }
}

void blinkStatus(int times, int ms) {
  for (int i = 0; i < times; i++) {
    digitalWrite(LED_STATUS, HIGH);
    delay(ms);
    digitalWrite(LED_STATUS, LOW);
    delay(ms);
  }
}
