# Fix Java/JAVA_HOME Error

## Problem
```
ERROR: JAVA_HOME is not set and no 'java' command could be found in your PATH.
```

## Solution for Manjaro/Arch Linux

### Step 1: Install Java JDK

Run this command in your terminal:

```bash
sudo pacman -S jdk17-openjdk
```

Or if you prefer JDK 11:
```bash
sudo pacman -S jdk11-openjdk
```

### Step 2: Find Java Installation Path

After installation, find where Java is installed:

```bash
# For JDK 17
ls -la /usr/lib/jvm/java-17-openjdk

# Or check all Java installations
ls -la /usr/lib/jvm/
```

### Step 3: Set JAVA_HOME

Add to your `~/.zshrc` file (since you're using zsh):

```bash
# Add this line (replace with actual path if different)
export JAVA_HOME=/usr/lib/jvm/java-17-openjdk
export PATH=$PATH:$JAVA_HOME/bin
```

Then reload your shell:
```bash
source ~/.zshrc
```

### Step 4: Verify Installation

```bash
java -version
echo $JAVA_HOME
```

You should see Java version and JAVA_HOME path.

### Step 5: Retry Build

```bash
cd android-app/capyboo-app
eas build --profile development --platform android --local
```

## Alternative: Use Android Studio's Java

If you have Android Studio installed, you can use its bundled JDK:

```bash
# Find Android Studio's JDK (usually in)
export JAVA_HOME=/opt/android-studio/jbr
# or
export JAVA_HOME=$HOME/Android/Android-Studio/jbr

export PATH=$PATH:$JAVA_HOME/bin
```

## Quick One-Liner Setup

Add this to your `~/.zshrc`:

```bash
# Java setup for Android development
if [ -d "/usr/lib/jvm/java-17-openjdk" ]; then
    export JAVA_HOME=/usr/lib/jvm/java-17-openjdk
    export PATH=$PATH:$JAVA_HOME/bin
fi
```

Then: `source ~/.zshrc`



