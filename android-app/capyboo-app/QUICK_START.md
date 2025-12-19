# Quick Start Guide - Capyboo Android App

## Step 1: Login to Expo

### Option A: Using EAS CLI (Recommended)

1. **Install EAS CLI globally:**

   ```bash
   npm install -g eas-cli
   ```

2. **Login to Expo:**

   ```bash
   eas login
   ```

3. **If you don't have an Expo account:**
   - It will prompt you to create one
   - Or visit [expo.dev](https://expo.dev) to sign up first
   - Then run `eas login` again

### Option B: Using Expo CLI (Alternative)

1. **Install Expo CLI:**

   ```bash
   npm install -g expo-cli
   ```

2. **Login:**
   ```bash
   expo login
   ```

## Step 2: Install Dependencies

```bash
cd android-app/capyboo-app
npm install
```

## Step 3: Build Development Client

Since we're using native BLE module, you need a development build:

```bash
# Build for Android
eas build --profile development --platform android

# Or build locally (requires Android Studio)
eas build --profile development --platform android --local
```

## Step 4: Run the App

After the build completes:

```bash
# Start the development server
npm start -- --dev-client

# Or
npx expo start --dev-client
```

## Alternative: Using Expo Go (Limited)

If you want to test without building, you can use Expo Go, but BLE won't work:

```bash
npm start
# Scan QR code with Expo Go app
```

**Note:** BLE functionality requires a development build, not Expo Go.

## Troubleshooting Login Issues

### "Command not found: eas"

- Make sure you installed EAS CLI: `npm install -g eas-cli`
- Check if npm global bin is in your PATH

### "Invalid credentials"

- Make sure you're using the correct email/password
- Try resetting password at [expo.dev](https://expo.dev)
- Or create a new account

### "Already logged in"

- Check current user: `eas whoami`
- Logout if needed: `eas logout`
- Login again: `eas login`

## Check Login Status

```bash
eas whoami
```

This shows your current Expo account email.
