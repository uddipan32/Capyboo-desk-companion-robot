# Fix Android SDK (ANDROID_HOME) Error

## Problem

```
SDK location not found. Define a valid SDK location with an ANDROID_HOME environment variable
```

## Solution

Your Android SDK is installed at: `~/Android/Sdk`

### Step 1: Set ANDROID_HOME in your shell

Add these lines to your `~/.zshrc` file:

```bash
# Android SDK
export ANDROID_HOME=$HOME/Android/Sdk
export PATH=$PATH:$ANDROID_HOME/emulator
export PATH=$PATH:$ANDROID_HOME/platform-tools
export PATH=$PATH:$ANDROID_HOME/tools
export PATH=$PATH:$ANDROID_HOME/tools/bin
```

### Step 2: Reload your shell

```bash
source ~/.zshrc
```

### Step 3: Verify

```bash
echo $ANDROID_HOME
# Should output: /home/uddipanb/Android/Sdk

adb version
# Should show adb version
```

### Step 4: Retry Build

```bash
cd android-app/capyboo-app
eas build --profile development --platform android --local
```

## Quick Setup Script

Run this to add to your `~/.zshrc`:

```bash
cat >> ~/.zshrc << 'EOF'

# Android SDK Setup
export ANDROID_HOME=$HOME/Android/Sdk
export PATH=$PATH:$ANDROID_HOME/emulator
export PATH=$PATH:$ANDROID_HOME/platform-tools
export PATH=$PATH:$ANDROID_HOME/tools
export PATH=$PATH:$ANDROID_HOME/tools/bin
EOF

source ~/.zshrc
```

## Alternative: Set for Current Session Only

If you just want to test without modifying `.zshrc`:

```bash
export ANDROID_HOME=$HOME/Android/Sdk
export PATH=$PATH:$ANDROID_HOME/platform-tools

# Then run your build command
```

## Verify Android SDK Installation

Check if you have the required components:

```bash
ls $ANDROID_HOME/platforms
ls $ANDROID_HOME/build-tools
```

You should see Android platform versions and build tools installed.


