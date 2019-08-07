# PC Ambilight
PC Ambilight based on Lightpack library

Components:
- Arduino C++ sketch running on an ESP8266EX D1 Mini from Lolin running @ 160MHz
- WS2812B 5V LED Strip (95 LED)
- 3.3V/5V Logic Level Converter 
- 220Ω resistor
- 1000uf capacitor for 5V power stabilization
- Raspberry + Home Assistant for Web GUI, automations and MQTT server (HA is optional but an MQTT server is needed)
- Google Home Mini for Voice Recognition (optional)

NOTE: 3.3V to 5V logic level converter is not mandatory but it is really recommended, without it, some input on the led strip digital pin could be lost. If you use a 5V microcontroller like Arduino Nano or similar you don't need it.

![CIRCUITS](https://github.com/sblantipodi/pc_ambilight/blob/master/ambilight_bb.png)


PC Ambilight YouTube video

[![IMAGE ALT TEXT HERE](https://img.youtube.com/vi/phW3lJt-T38/0.jpg)](https://www.youtube.com/watch?v=phW3lJt-T38)
