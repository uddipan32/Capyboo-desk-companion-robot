# Installation Instructions

## Step 1: Install Dependencies

Navigate to the app directory and install dependencies:

```bash
cd android-app/capyboo-app
npm install
```

This will install:
- `react-native-ble-plx` - For Bluetooth Low Energy communication
- All other Expo and React Native dependencies

## Step 2: Rebuild the App

Since we added a native module (react-native-ble-plx), you need to rebuild:

### For Development Build:

```bash
# Install EAS CLI (if not already installed)
npm install -g eas-cli

# Login to Expo
eas login

# Build development client
eas build --profile development --platform android
```

### For Expo Go (Limited):

Expo Go has limited support for native modules. For full BLE functionality, you'll need a development build.

## Step 3: Run the App

After building:

```bash
npm start
# or
npx expo start --dev-client
```

## Step 4: Permissions

The app will request the following permissions:
- **Bluetooth** - To scan and connect to devices
- **Location** - Required for BLE scanning on Android

Make sure to grant these permissions when prompted.

## Troubleshooting

### "Module not found: react-native-ble-plx"
- Make sure you ran `npm install`
- Rebuild the app (native modules require rebuild)

### "Bluetooth permission denied"
- Go to Android Settings > Apps > Capyboo > Permissions
- Enable Bluetooth and Location permissions

### "Device not found"
- Make sure Capyboo is powered on
- Make sure Bluetooth is enabled on your phone
- Make sure location services are enabled (required for BLE on Android)
- Try moving closer to the device

### Build Errors
- Make sure you have Android Studio and Android SDK installed
- Try: `npx expo prebuild --clean`

