# ESP32 Weather Station

This is a simple DIY weather station project built using an ESP32-based board.

## Features

*   Displays current weather information (temperature, feels like, humidity, weather description).
*   Shows a 2-day weather forecast.
*   Displays the current date and time, synchronized via NTP.
*   Fetches weather data from the [wttr.in](https://wttr.in) service.

## Hardware

This project was built using a CYD (Cheap Yellow Display) ESP32 board purchased from AliExpress:

*   **Product Link:** [CYD ESP32 2.4 inch](https://pl.aliexpress.com/item/1005008212152877.html)

## Setup

1.  **Clone the repository:**

    ```bash
    git clone <repository-url>
    ```

2.  **Open the `weather/weather.ino` file in the Arduino IDE.**

3.  **Install the required libraries:**
    *   `TFT_eSPI`
    *   `ArduinoJson`
    *   `NTPClient`

4.  **Configure your credentials:**

    In the `weather.ino` file, you need to replace the placeholder values with your own information:

    *   `YOUR_WIFI_SSID`: Your WiFi network name.
    *   `YOUR_WIFI_PASSWORD`: Your WiFi password.
    *   `YOUR_CITY`: The city for which you want to track the weather. Use URL encoding for spaces (e.g., `New%20York`).

5.  **Upload the code to your ESP32 board.**
