# Setting Up Credentials

This project uses a `secrets.h` file to store sensitive credentials like WiFi passwords and MQTT credentials. This file is excluded from version control for security.

## Setup Instructions

1. **Copy the example file:**

   ```bash
   cp secrets.h.example secrets.h
   ```

2. **Edit `secrets.h`** and fill in your credentials:

   - WiFi SSID and password
   - MQTT broker URL (HiveMQ Cloud cluster)
   - MQTT username and password

3. **Never commit `secrets.h` to git** - it's already in `.gitignore`

## File Structure

- `secrets.h.example` - Template file (safe to commit)
- `secrets.h` - Your actual credentials (gitignored)

## Example secrets.h

```cpp
#ifndef SECRETS_H
#define SECRETS_H

// WiFi Credentials
const char* WIFI_SSID = "YourWiFiNetwork";
const char* WIFI_PASSWORD = "YourWiFiPassword";

// MQTT Credentials (HiveMQ Cloud)
const char* MQTT_BROKER = "your-cluster-id.s1.eu.hivemq.cloud";
const char* MQTT_USERNAME = "your-username";
const char* MQTT_PASSWORD = "your-password";

#endif // SECRETS_H
```

## Security Notes

- ✅ `secrets.h` is in `.gitignore` - your credentials won't be committed
- ✅ `secrets.h.example` is a template without real credentials
- ⚠️ Never share your `secrets.h` file
- ⚠️ If you accidentally commit credentials, rotate them immediately
