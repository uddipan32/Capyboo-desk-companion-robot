# WiFi Setup via Bluetooth

The Capyboo robot can now be configured to connect to WiFi using Bluetooth commands. The WiFi credentials are stored in the ESP32's non-volatile memory, so they persist across reboots.

## Features

- ✅ Set WiFi credentials via Bluetooth
- ✅ Credentials stored in ESP32 flash memory (persistent)
- ✅ Automatic reconnection on restart
- ✅ Clear stored credentials if needed

## How to Use

### 1. Connect via Bluetooth

Connect to the "Capyboo" BLE device using a Bluetooth terminal app (like Serial Bluetooth Terminal, nRF Connect, etc.)

### 2. Set WiFi Credentials

Send the following command format:

```
wifi:YOUR_SSID:YOUR_PASSWORD
```

**Example:**

```
wifi:MyHomeNetwork:MyPassword123
```

**Important:**

- Use colons (`:`) to separate the command, SSID, and password
- SSID and password are case-sensitive
- No spaces around the colons

### 3. Verify Connection

After sending the command:

- The display will show "Setting WiFi..."
- Then "WiFi connected!" if successful
- Or "WiFi failed!" if connection fails
- You'll also receive a response via BLE

### 4. Automatic Reconnection

Once credentials are saved, the robot will automatically:

- Load stored credentials on startup
- Connect to WiFi automatically
- Reconnect if WiFi is lost and restored

### 5. Clear WiFi Credentials

To clear stored WiFi credentials, send:

```
clearwifi
```

or

```
wificlear
```

This will:

- Delete stored credentials
- Disconnect from WiFi
- Display "WiFi cleared"

## Command Reference

| Command              | Description                                 |
| -------------------- | ------------------------------------------- |
| `wifi:SSID:password` | Set WiFi credentials and connect            |
| `clearwifi`          | Clear stored WiFi credentials               |
| `wificlear`          | Clear stored WiFi credentials (alternative) |

## First-Time Setup

On first boot:

1. If no WiFi credentials are stored, the robot will try to use credentials from `secrets.h` (if available)
2. If connection succeeds, those credentials are automatically saved
3. On next boot, stored credentials will be used instead

## Troubleshooting

**WiFi connection fails:**

- Check SSID and password are correct
- Ensure the WiFi network is in range
- Try the `clearwifi` command and set again

**Credentials not persisting:**

- Check that the ESP32 has enough flash memory
- Try clearing and setting again

**Can't connect via BLE:**

- Make sure BLE is enabled on your device
- Check that the robot is powered on
- Try restarting the robot

## Technical Details

- Uses ESP32 Preferences library for non-volatile storage
- Credentials stored in flash memory (survives power loss)
- Maximum SSID length: 63 characters
- Maximum password length: 63 characters
