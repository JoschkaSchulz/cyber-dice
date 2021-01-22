# WIP

## cyber-dice
transfer physical dice into virtual cyberspace

![example workflow name](https://github.com/JoschkaSchulz/cyber-dice/workflows/Lint/badge.svg)

## Idea & Progress
`✅Button` → `✅ESP32-CAM` → `✅❌Node Server` → `❌Foundry VTT`

## Setup ESP32-CAM
Copy the wifi_config.h.example file and remove the .example. Add your SSID and Password there and it should be fine then.

```
# make a copy of wifi_config.h file
cp firmware/wifi_config.h.example firmware/wifi_config.h

# fill in your ssid and password, therefore the ESP can connect to your wifi
# compile then with arduino and it should work
```

## Setup Server
There are a few simple steps for the first time setup:

```
# make a copy of .env file
cp .env.example .env

# fill in the missing variables in .env
docker-compose up
```