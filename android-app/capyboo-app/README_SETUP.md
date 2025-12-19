# Capyboo Android App Setup Guide

This is a React Native app built with Expo for controlling the Capyboo robot via Bluetooth and MQTT.

## Prerequisites

1. **Node.js** (v18 or later)

   - Download from [nodejs.org](https://nodejs.org/)
   - Verify: `node --version`

2. **npm** (comes with Node.js)

   - Verify: `npm --version`

3. **Expo CLI** (optional, but recommended)

   ```bash
   npm install -g expo-cli
   ```

4. **Android Development Setup** (for Android):
   - Android Studio with Android SDK
   - Android emulator OR physical Android device
   - Enable USB debugging on physical device

## Installation

1. **Navigate to the app directory:**

   ```bash
   cd android-app/capyboo-app
   ```

2. **Install dependencies:**
   ```bash
   npm install
   ```

## Running the App

### Option 1: Using Expo Go (Easiest)

1. **Install Expo Go** on your Android device from Google Play Store

2. **Start the development server:**

   ```bash
   npm start
   # or
   npx expo start
   ```

3. **Scan the QR code** with:
   - Expo Go app (Android)
   - Camera app (iOS)

### Option 2: Android Emulator

1. **Start Android Studio** and launch an emulator

2. **Start Expo:**
   ```bash
   npm run android
   # or
   npx expo start --android
   ```

### Option 3: Physical Android Device

1. **Connect your device** via USB and enable USB debugging

2. **Start Expo:**
   ```bash
   npm run android
   ```

## Development

### Project Structure

```
capyboo-app/
├── app/                    # Main app screens (file-based routing)
│   ├── (tabs)/            # Tab navigation screens
│   │   ├── index.tsx      # Home screen
│   │   └── explore.tsx    # Explore screen
│   └── _layout.tsx        # Root layout
├── components/            # Reusable components
├── constants/             # App constants
└── hooks/                 # Custom React hooks
```

### Key Files

- `app/(tabs)/index.tsx` - Main home screen
- `app.json` - Expo configuration
- `package.json` - Dependencies and scripts

## Adding BLE Functionality

To connect to the Capyboo robot via Bluetooth, you'll need to:

1. **Install BLE library:**

   ```bash
   npm install react-native-ble-plx
   ```

2. **Add permissions** to `app.json`:

   ```json
   {
     "expo": {
       "plugins": [
         [
           "react-native-ble-plx",
           {
             "bluetoothAlwaysPermission": "Allow $(PRODUCT_NAME) to connect to Bluetooth devices"
           }
         ]
       ]
     }
   }
   ```

3. **Create a BLE service** to handle connections

## Adding MQTT Functionality

To connect via MQTT:

1. **Install MQTT library:**

   ```bash
   npm install react-native-mqtt
   ```

2. **Configure MQTT connection** with your HiveMQ credentials

## Building for Production

### Android APK

```bash
# Install EAS CLI
npm install -g eas-cli

# Login to Expo
eas login

# Configure build
eas build:configure

# Build APK
eas build --platform android --profile preview
```

## Troubleshooting

### "Cannot connect to Metro bundler"

- Make sure your device/emulator is on the same network
- Try restarting: `npm start -- --reset-cache`

### "Module not found"

- Delete `node_modules` and reinstall: `rm -rf node_modules && npm install`

### BLE not working

- Check Android permissions in `app.json`
- Ensure location services are enabled (required for BLE on Android)

## Next Steps

1. **Add BLE connection screen** to connect to "Capyboo" device
2. **Add WiFi setup form** to send `wifi:SSID:password` commands
3. **Add MQTT connection** for remote control
4. **Add animation controls** to trigger robot animations
5. **Add status display** to show robot state

## Resources

- [Expo Documentation](https://docs.expo.dev/)
- [React Native BLE PLX](https://github.com/dotintent/react-native-ble-plx)
- [React Native MQTT](https://github.com/robtweed/react-native-mqtt)
