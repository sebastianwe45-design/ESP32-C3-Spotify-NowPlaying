# ESP32-C3 Spotify Now Playing Display

A compact Spotify "Now Playing" display built with an ESP32-C3 Mini and a 64x48 SSD1306 OLED.

The display shows the currently playing Spotify track including:

- Song title with scrolling text for long names
- Artist name
- Current playback position
- Total song duration
- Playback progress bar

## Hardware

- ESP32-C3 Mini
- SSD1306 OLED Display (64x48 pixels)
- Wi-Fi connection

## Features

✅ Displays the currently playing Spotify track

✅ Automatic token refresh using Spotify OAuth

✅ Smooth scrolling song titles

✅ Progress bar with playback progress

✅ Current and total track duration

✅ Optimized layout for 64x48 OLED displays

## Required Libraries

Install the following libraries through the Arduino Library Manager:

### ESP32 Core

```
esp32 by Espressif Systems
```

### OLED Driver

```
ESP8266 and ESP32 OLED Driver for SSD1306 displays
```

Author:

```
ThingPulse
```

### JSON

```
ArduinoJson
```

## Spotify Setup

### 1. Create a Spotify App

Open:

https://developer.spotify.com/dashboard

Create a new app and note:

- Client ID
- Client Secret

### 2. Configure Redirect URI

Add the following Redirect URI:

```text
http://127.0.0.1:8888/callback
```

### 3. Request Access

Open the authorization URL in your browser:

```text
https://accounts.spotify.com/authorize?client_id=YOUR_CLIENT_ID&response_type=code&redirect_uri=http://127.0.0.1:8888/callback&scope=user-read-currently-playing%20user-read-playback-state
```

After login, copy the value after:

```text
code=
```

### 4. Generate Refresh Token

Use the Authorization Code together with your Client ID and Client Secret to request an Access Token and Refresh Token from Spotify.

The generated Refresh Token can then be added to the sketch.

## Configuration

Before uploading the sketch, replace the following values:

```cpp
const char* ssid = "YOUR_WIFI";
const char* password = "YOUR_WIFI_PASSWORD";

const char* clientId = "YOUR_CLIENT_ID";
const char* clientSecret = "YOUR_CLIENT_SECRET";
const char* refreshToken = "YOUR_REFRESH_TOKEN";
```

## Uploading

1. Install all required libraries.
2. Enter your Wi-Fi and Spotify credentials.
3. Select the correct ESP32-C3 board in Arduino IDE.
4. Upload the sketch.
5. Open the Serial Monitor at:

```text
115200 Baud
```

## Troubleshooting

### OLED stays blank

Verify:

- OLED I2C address (commonly `0x3C`)
- SDA and SCL wiring
- Correct display geometry:

```cpp
SSD1306 display(0x3c, 8, 10, GEOMETRY_64_48);
```

### Spotify returns HTTP 401

The Access Token may have expired.

Check:

- Client ID
- Client Secret
- Refresh Token

The sketch will automatically attempt to refresh the token.

### Spotify returns HTTP 400

Verify that the Refresh Token is valid and belongs to the configured Spotify application.
