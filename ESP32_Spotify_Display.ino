#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include "SSD1306.h"
#include "base64.h"

// WLAN Requirements
const char* ssid = "YOUR_WIFI";
const char* password = "YOUR_PASSWORD";

// Spotify Requirements
const char* clientId = "SPOTIFY_CLIENT_ID";
const char* clientSecret = "SPOTIFY_CLIENT_SECRET";
const char* refreshToken = "SPOTIFY_REFRESH_TOKEN";

// Display
SSD1306 display(0x3c, 8, 10, GEOMETRY_64_48);

String spotifyAccessToken = "";

int scrollPos = 0;
unsigned long lastScroll = 0;

String lastTitle = "";

unsigned long lastSpotifyUpdate = 0;
unsigned long lastTokenRefresh = 0;

String currentTitle = "";
String currentArtist = "";

int currentProgress = 0;
int currentDuration = 1;

unsigned long progressTimestamp = 0;

bool refreshSpotifyToken() {
  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient https;

  https.begin(
    client,
    "https://accounts.spotify.com/api/token");

  String credentials =
    String(clientId) + ":" + String(clientSecret);

  String encoded =
    base64::encode(credentials);

  https.addHeader(
    "Authorization",
    "Basic " + encoded);

  https.addHeader(
    "Content-Type",
    "application/x-www-form-urlencoded");

  String body =
    "grant_type=refresh_token&refresh_token=" + String(refreshToken);

  int httpCode = https.POST(body);

  if (httpCode != 200) {
    Serial.printf("Token Fehler: %d\n", httpCode);

    String errorText = https.getString();

    Serial.println("Antwort:");
    Serial.println(errorText);

    https.end();
    return false;
  }

  String payload =
    https.getString();


  Serial.print("Token HTTP-Code: ");
  Serial.println(httpCode);

  Serial.println(payload);

  DynamicJsonDocument doc(2048);

  DeserializationError error =
    deserializeJson(doc, payload);

  if (error) {
    Serial.println("JSON Fehler");
    https.end();
    return false;
  }

  spotifyAccessToken =
    doc["access_token"].as<String>();

  Serial.println("Token erhalten:");
  Serial.println(spotifyAccessToken);
  Serial.println("Spotify Token erneuert");

  https.end();

  return true;
}

void setup() {
  Serial.begin(115200);
  Serial.println("ESP gestartet");

  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);

  display.clear();
  display.drawString(0, 0, "WLAN...");
  display.display();

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  display.clear();
  display.drawString(0, 0, "Verbunden");
  display.display();

  if (refreshSpotifyToken()) {
    lastTokenRefresh = millis();
    getSpotifyTrack();
  }
}

void loop() {
  // Token nach 55 Minuten erneuern
  if (millis() - lastTokenRefresh > 3300000UL) {
    if (refreshSpotifyToken()) {
      lastTokenRefresh = millis();
    }
  }

  // Spotify nur alle 6 Sekunden abfragen
  if (millis() - lastSpotifyUpdate > 6000) {
    getSpotifyTrack();
    lastSpotifyUpdate = millis();
  }

  int progressNow =
    currentProgress + (millis() - progressTimestamp);

  if (progressNow > currentDuration)
    progressNow = currentDuration;

  showTrack(
    currentTitle,
    currentArtist,
    progressNow,
    currentDuration);

  delay(50);
}

void getSpotifyTrack() {
  if (WiFi.status() != WL_CONNECTED)
    return;

  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient https;

  https.begin(
    client,
    "https://api.spotify.com/v1/me/player/currently-playing");

  https.addHeader(
    "Authorization",
    "Bearer " + spotifyAccessToken);

  int httpCode =
    https.GET();

  if (httpCode == 401) {
    Serial.println(
      "Token abgelaufen");

    refreshSpotifyToken();

    https.end();
    return;
  }

  if (httpCode == 200) {
    String payload =
      https.getString();

    DynamicJsonDocument doc(8192);

    deserializeJson(
      doc,
      payload);

    currentTitle =
      doc["item"]["name"]
        .as<String>();

    currentArtist =
      doc["item"]["artists"][0]["name"]
        .as<String>();

    currentDuration =
      doc["item"]["duration_ms"];

    currentProgress =
      doc["progress_ms"];

    progressTimestamp =
      millis();

    Serial.println(currentTitle);
  } else {
    Serial.printf(
      "HTTP Fehler: %d\n",
      httpCode);
  }

  https.end();
}

void showTrack(
  String title,
  String artist,
  int progress,
  int duration) {
  display.clear();

  if (title != lastTitle) {
    scrollPos = 0;
    lastTitle = title;
  }

  int currentSec =
    progress / 1000;

  int totalSec =
    duration / 1000;

  String currentTime =
    String(currentSec / 60) + ":" + (currentSec % 60 < 10 ? "0" : "") + String(currentSec % 60);

  String totalTime =
    String(totalSec / 60) + ":" + (totalSec % 60 < 10 ? "0" : "") + String(totalSec % 60);

  drawScrollingTitle(title);

  display.setTextAlignment(
    TEXT_ALIGN_CENTER);

  if (artist.length() > 14)
    artist =
      artist.substring(0, 14);

  display.drawString(
    32,
    12,
    artist);

  display.setTextAlignment(
    TEXT_ALIGN_LEFT);

  display.drawString(
    0,
    26,
    currentTime);

  display.setTextAlignment(
    TEXT_ALIGN_RIGHT);

  display.drawString(
    63,
    26,
    totalTime);

  int percent =
    (100L * progress) / duration;

  display.drawProgressBar(
    4,
    38,
    56,
    5,
    percent);

  display.display();
}

void drawScrollingTitle(
  String text) {
  display.setTextAlignment(
    TEXT_ALIGN_LEFT);

  int textWidth =
    display.getStringWidth(
      text);

  if (textWidth <= 64) {
    display.setTextAlignment(
      TEXT_ALIGN_CENTER);

    display.drawString(
      32,
      0,
      text);

    return;
  }

  display.drawString(
    -scrollPos,
    0,
    text);

  display.drawString(
    textWidth + 20 - scrollPos,
    0,
    text);

  if (millis() - lastScroll > 70) {
    scrollPos++;

    if (scrollPos > textWidth + 20) {
      scrollPos = 0;
    }

    lastScroll =
      millis();
  }
}