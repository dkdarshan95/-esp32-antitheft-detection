/*
 * telegram_notify.h — Send image & text alerts via Telegram Bot API
 */

#pragma once

#include <WiFiClientSecure.h>
#include "esp_camera.h"
#include "config.h"

static const char* TELEGRAM_HOST = "api.telegram.org";
static WiFiClientSecure _tlsClient;

static bool _beginTelegram() {
  _tlsClient.setInsecure();
  if (!_tlsClient.connect(TELEGRAM_HOST, 443)) {
    Serial.println("[TELEGRAM] Connection failed");
    return false;
  }
  return true;
}

void sendTelegramAlert(camera_fb_t* fb, int count) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[TELEGRAM] Wi-Fi not connected — skipping");
    return;
  }

  if (!_beginTelegram()) return;

  String caption = "🚨 *INTRUSION ALERT #" + String(count) + "*\n";
  caption += "📍 Location: " + String(LOCATION_TAG) + "\n";
  caption += "🕐 Uptime: " + String(millis() / 1000) + "s\n";
  caption += "📷 Device: " + String(DEVICE_NAME);

  String boundary = "ESP32CAMBoundary";
  String bodyStart = "--" + boundary + "\r\n";
  bodyStart += "Content-Disposition: form-data; name=\"chat_id\"\r\n\r\n";
  bodyStart += String(CHAT_ID) + "\r\n";
  bodyStart += "--" + boundary + "\r\n";
  bodyStart += "Content-Disposition: form-data; name=\"caption\"\r\n\r\n";
  bodyStart += caption + "\r\n";
  bodyStart += "--" + boundary + "\r\n";
  bodyStart += "Content-Disposition: form-data; name=\"parse_mode\"\r\n\r\nMarkdown\r\n";
  bodyStart += "--" + boundary + "\r\n";
  bodyStart += "Content-Disposition: form-data; name=\"photo\"; filename=\"alert.jpg\"\r\n";
  bodyStart += "Content-Type: image/jpeg\r\n\r\n";

  String bodyEnd = "\r\n--" + boundary + "--\r\n";
  size_t totalLen = bodyStart.length() + fb->len + bodyEnd.length();

  String request =
    "POST /bot" + String(BOT_TOKEN) + "/sendPhoto HTTP/1.1\r\n"
    "Host: " + String(TELEGRAM_HOST) + "\r\n"
    "Content-Type: multipart/form-data; boundary=" + boundary + "\r\n"
    "Content-Length: " + String(totalLen) + "\r\n"
    "Connection: close\r\n\r\n";

  _tlsClient.print(request);
  _tlsClient.print(bodyStart);
  _tlsClient.write(fb->buf, fb->len);
  _tlsClient.print(bodyEnd);

  while (_tlsClient.connected()) {
    String line = _tlsClient.readStringUntil('\n');
    if (line.startsWith("HTTP/")) {
      if (line.indexOf("200") != -1)
        Serial.println("[TELEGRAM] Photo sent successfully");
      else
        Serial.println("[TELEGRAM] Error: " + line);
      break;
    }
  }

  _tlsClient.stop();
}

void sendTelegramTextAlert(int count) {
  if (WiFi.status() != WL_CONNECTED) return;
  if (!_beginTelegram()) return;

  String message =
    "🚨 *MOTION DETECTED #" + String(count) + "*%0A"
    "📍 " + String(LOCATION_TAG) + "%0A"
    "⚠️ Camera capture failed — check device.";

  String url =
    "/bot" + String(BOT_TOKEN) +
    "/sendMessage?chat_id=" + String(CHAT_ID) +
    "&text=" + message +
    "&parse_mode=Markdown";

  _tlsClient.println("GET " + url + " HTTP/1.1");
  _tlsClient.println("Host: " + String(TELEGRAM_HOST));
  _tlsClient.println("Connection: close");
  _tlsClient.println();

  delay(500);
  _tlsClient.stop();
  Serial.println("[TELEGRAM] Text alert sent");
}
