#include "display.h"
#include <Wire.h>

#include "face_animation.h"
#include "dino_game.h"
#include "clock.h"

const int SPEAKER_PIN = 3;

enum Mode {
    MODE_HORSE,
    MODE_ANIMATION,
    MODE_WEATHER,
    MODE_GAME,
    MODE_CLOCK,
    MODE_PROGRESS
};

String message = "";
String currentCity = "";
float currentTemperature = 0;
float currentFeelsLike = 0;
int currentHumidity = 0;
String currentDescription = "";
int currentHour = 0;
int currentMinute = 0;
int currentSecond = 0;

String mood = "random";
Mode currentMode = MODE_ANIMATION; // overridden in setup if serial host is connected

// Progress mode state
int progressPercent = 0;
String progressTop = "Progress";
String progressBottom = "";

// Horse mode: speed from host CPU percent (0 = slow, 100 = fast)
int cpuPercent = 0;
int horseFrameIndex = 0;
const unsigned long HORSE_DELAY_SLOW_MS = 120; // at 0% CPU
const unsigned long HORSE_DELAY_FAST_MS = 20;  // at 100% CPU

// Serial input (non-blocking)
String serialLineBuffer = "";
bool serialJumpPressed = false;
unsigned long serialLastByteMs = 0;
const unsigned long SERIAL_LINE_TIMEOUT_MS = 80; // complete command if no newline arrives

unsigned long horseFrameDelayFromCpu(int percent) {
    if (percent < 0) percent = 0;
    if (percent > 100) percent = 100;
    // Higher CPU → shorter delay → faster gallop
    return HORSE_DELAY_SLOW_MS -
           ((HORSE_DELAY_SLOW_MS - HORSE_DELAY_FAST_MS) * (unsigned long)percent) / 100UL;
}


// Display weather on OLED
void displayWeatherOnOLED(String city, float temp, float feelsLike, int humidity, String desc) {
    display.clearDisplay();
    display.drawRect(0, 0, 128, 64, SSD1306_WHITE);
    display.drawLine(0, 12, 128, 12, SSD1306_WHITE);

    display.setTextSize(1);
    display.setCursor(64 - (city.length() * 3), 2);
    display.print(city);

    display.setTextSize(3);
    display.setCursor(5, 18);
    display.print(temp, 0);

    display.setTextSize(1);
    display.setCursor(45, 20);
    display.print("o");
    display.setCursor(50, 18);
    display.setTextSize(2);
    display.print("C");

    display.setTextSize(1);
    display.setCursor(85, 18);
    display.print("Feels");
    display.setCursor(85, 28);
    display.print(feelsLike, 0);
    display.print("o");

    display.setCursor(5, 48);
    display.print("H:");
    display.print(humidity);
    display.print("%");

    display.setCursor(50, 48);
    String shortDesc = desc;
    if (shortDesc.length() > 12) {
        shortDesc = shortDesc.substring(0, 9) + "...";
    }
    if (shortDesc.length() > 0) {
        char first = shortDesc.charAt(0);
        if (first >= 'a' && first <= 'z') {
            shortDesc.setCharAt(0, first - 32);
        }
    }
    display.print(shortDesc);

    display.drawLine(0, 42, 128, 42, SSD1306_WHITE);
    display.display();
}

// Progress mode: top title, centered bar, bottom status from serial
void displayProgressScreen() {
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);

    int16_t x1, y1;
    uint16_t w, h;

    // Top text
    String top = progressTop;
    if (top.length() > 21) {
        top = top.substring(0, 21);
    }
    display.getTextBounds(top.c_str(), 0, 0, &x1, &y1, &w, &h);
    display.setCursor((128 - (int)w) / 2, 4);
    display.print(top);

    // Progress bar
    const int barX = 8;
    const int barY = 24;
    const int barW = 112;
    const int barH = 14;
    int fill = progressPercent;
    if (fill < 0) fill = 0;
    if (fill > 100) fill = 100;

    display.drawRect(barX, barY, barW, barH, SSD1306_WHITE);
    int fillW = ((barW - 4) * fill) / 100;
    if (fillW > 0) {
        display.fillRect(barX + 2, barY + 2, fillW, barH - 4, SSD1306_WHITE);
    }

    // Percent under bar
    String pct = String(fill) + "%";
    display.getTextBounds(pct.c_str(), 0, 0, &x1, &y1, &w, &h);
    display.setCursor((128 - (int)w) / 2, 42);
    display.print(pct);

    // Bottom text from command
    String bottom = progressBottom;
    if (bottom.length() > 21) {
        bottom = bottom.substring(0, 21);
    }
    display.getTextBounds(bottom.c_str(), 0, 0, &x1, &y1, &w, &h);
    display.setCursor((128 - (int)w) / 2, 54);
    display.print(bottom);

    display.display();
}


void setup() {
    Serial.setRxBufferSize(1024);
    Serial.begin(115200);
    delay(200);

    // Default: horse when a serial host is connected, otherwise face animation
    unsigned long serialWaitUntil = millis() + 1500;
    while (millis() < serialWaitUntil) {
        if (Serial) {
            break;
        }
        delay(10);
    }
    currentMode = Serial ? MODE_HORSE : MODE_ANIMATION;

    pinMode(SPEAKER_PIN, OUTPUT);

    Wire.begin();
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println(F("SSD1306 allocation failed"));
        for (;;);
    }

    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);

    // Welcome splash
    int16_t x1, y1;
    uint16_t w, h;
    display.getTextBounds("Welcome back", 0, 0, &x1, &y1, &w, &h);
    display.setCursor((128 - (int)w) / 2, 18);
    display.print(F("Welcome back"));

    display.getTextBounds("Underground Editor", 0, 0, &x1, &y1, &w, &h);
    display.setCursor((128 - (int)w) / 2, 34);
    display.print(F("Underground Editor"));

    display.display();
    delay(2000);

    playWakeupAnimation();

    randomSeed(analogRead(0));
    selectAnimationSequence();
    displayCurrentMode();

    Serial.println();
    Serial.println(F("Ready (serial only)."));
    Serial.println(F("Serial Monitor: 115200 baud, set line ending to Newline."));
    Serial.println(F("Commands: mode:horse|animation|weather|game|clock|progress"));
    Serial.println(F("  cpu:<0-100>  mood:<name>  weather:city:temp:feels:humidity:desc"));
    Serial.println(F("  time:HH:MM:SS  message:<text>  jump  tickle  loveyou"));
    Serial.println(F("  progress:<0-100>  progress:<0-100>:<bottom>"));
    Serial.println(F("  progress:<0-100>:<top>:<bottom>"));
    Serial.println(F("Type 'ping' to test serial."));
}

// Function to display current mode
void displayCurrentMode() {
    String modeText = "Mode: ";
    switch (currentMode) {
        case MODE_HORSE:
            modeText += "Horse";
            break;
        case MODE_ANIMATION:
            modeText += "Animation";
            break;
        case MODE_WEATHER:
            modeText += "Weather";
            break;
        case MODE_GAME:
            modeText += "Game";
            break;
        case MODE_CLOCK:
            modeText += "Clock";
            break;
        case MODE_PROGRESS:
            modeText += "Progress";
            break;
    }
}

// Animation sequence state
int animationIndex = 0;
unsigned long lastAnimationTime = 0;
const unsigned long ANIMATION_DELAY = 0; // Default delay between animations in ms
int currentSequenceIndex = -1; // Track which sequence we're currently playing

// Simple function to play sound using analogWrite
void play_sound() {
    // Speaker - using analog output (PWM)
    analogWrite(SPEAKER_PIN, 128); // 50% duty cycle (0-255 range, or 0-4095 on ESP32)
    delay(1000);
    analogWrite(SPEAKER_PIN, 0); // Turn off
}

// Function to play a tone at specific frequency and duration
void playTonePattern(int frequency, int duration) {
    if (frequency == 0) {
        analogWrite(SPEAKER_PIN, 0);
        delay(duration);
        return;
    }
    
    // Calculate half period in microseconds based on frequency
    // Frequency in Hz, so period = 1/frequency seconds = 1000000/frequency microseconds
    unsigned long halfPeriod = 500000 / frequency; // Half period in microseconds
    unsigned long startTime = millis();
    int dutyCycle = 128; // 50% duty cycle
    
    while (millis() - startTime < duration) {
        analogWrite(SPEAKER_PIN, dutyCycle); // ON
        delayMicroseconds(halfPeriod);
        analogWrite(SPEAKER_PIN, 0); // OFF
        delayMicroseconds(halfPeriod);
    }
    analogWrite(SPEAKER_PIN, 0); // Turn off
}

// Animation function pointer type
typedef void (*AnimationFunction)();

// Animation entry with optional delay
struct AnimationEntry {
    AnimationFunction func;
    unsigned long delayMs;  // Delay after this animation (0 = use default)
};

// Structure to hold a sequence with its length
struct SequenceInfo {
    AnimationEntry* sequence;
    int length;
};

// Pointer to current animation sequence (can point to any sequence)
AnimationEntry* currentAnimationSequence = nullptr;
int currentAnimationSequenceLength = 0;

// Helper macros to create animation entries
#define ANIM_WITH_DELAY(func, delay) {func, delay}
#define ANIM(func) {func, 0}  // Use default delay

AnimationEntry idleAnimationSequence[] = {
    ANIM_WITH_DELAY(playLookRightFromMiddleAnimation, 1000),      // 0 - 1 second delay
    ANIM_WITH_DELAY(playLookMiddleFromRightAnimation, 2000),     // 1 - 1 second delay
    ANIM_WITH_DELAY(playLookLeftFromMiddleAnimation, 1000),       // 2 - 1 second delay
    ANIM_WITH_DELAY(playLookMiddleFromLeftAnimation, 2000),      // 3 - 1 second delay
};

AnimationEntry happyAnimationSequence[] = {
    ANIM_WITH_DELAY(playIdleToHappyAnimation, 1000),             // 0 - 1 second delay
    ANIM_WITH_DELAY(playHappyToIdleAnimation, 2000),             // 1 - 1 second delay
    ANIM(playEnjoyStartAnimation),             // 0 - 1 second delay
    ANIM(playEnjoyingAnimation),
    ANIM(playEnjoyingAnimation),
    ANIM(playEnjoyingAnimation),
    ANIM(playEnjoyingAnimation),
    ANIM(playEnjoyingAnimation),
    ANIM(playEnjoyEndAnimation),   
    ANIM(playThumbStartAnimation),
    ANIM(playThumbAnimation),
    ANIM(playThumbAnimation),
    ANIM(playThumbAnimation),
    ANIM(playThumbAnimation),
    ANIM(playThumbEndAnimation),
    ANIM(playWaveStartAnimation),
    ANIM(playWaveAnimation),
    ANIM(playWaveAnimation),
    ANIM(playWaveAnimation),
    ANIM(playWaveAnimation),
    ANIM(playWaveEndAnimation),

};

AnimationEntry EnjoyingAnimationSequence[] = {
    ANIM(playEnjoyStartAnimation),             // 0 - 1 second delay
    ANIM(playEnjoyingAnimation),
    ANIM(playEnjoyingAnimation),
    ANIM(playEnjoyingAnimation),
    ANIM(playEnjoyingAnimation),
    ANIM(playEnjoyingAnimation),
    ANIM(playEnjoyEndAnimation),             // 3 - 1 second delay
};

AnimationEntry AngryAnimationSequence[] = {
    ANIM_WITH_DELAY(playIdleToAngryAnimation, 3000),             // 0 - 1 second delay
    ANIM_WITH_DELAY(playAngryToIdleAnimation, 2000),             // 1 - 1 second delay
};

AnimationEntry SadAnimationSequence[] = {
    ANIM_WITH_DELAY(playIdleToSadAnimation, 1000),             // 0 - 1 second delay
    ANIM_WITH_DELAY(playSadToIdleAnimation, 2000),             // 1 - 1 second delay
};

AnimationEntry VerySadAnimationSequence[] = {
    ANIM_WITH_DELAY(playIdleToSadAnimation, 1000),             // 0 - 1 second delay
    ANIM(playTearAnimation),
    ANIM(playTearAnimation),
    ANIM(playTearAnimation),
    ANIM(playTearAnimation),
    ANIM(playTearAnimation),
    ANIM(playTearAnimation),
    ANIM_WITH_DELAY(playSadToIdleAnimation, 2000),   
};

AnimationEntry CryAnimationSequence[] = {
    ANIM_WITH_DELAY(playIdleToSadAnimation, 1000),
    ANIM(playSadToCryAnimation),
    ANIM(playCryingAnimation),
    ANIM(playCryingAnimation),
    ANIM(playCryingAnimation),
    ANIM(playCryingAnimation),
    ANIM(playCryingAnimation),
    ANIM(playCryingAnimation),
    ANIM(playCryingAnimation),
    ANIM(playCryToSadAnimation),
    ANIM_WITH_DELAY(playSadToIdleAnimation, 2000),
};

AnimationEntry FunnyAnimationSequence[] = {
    ANIM_WITH_DELAY(playNormalToFunnyEyesAnimation, 1000),
    ANIM(playTongueOutAnimation),
    ANIM(playTongueOutAnimation),
    ANIM(playTongueOutAnimation),
    ANIM(playTongueOutAnimation),
    ANIM(playTongueOutAnimation),
    ANIM_WITH_DELAY(playFunnyEyesToNormalAnimation, 2000),
};

AnimationEntry LoveAnimationSequence[] = {
    ANIM_WITH_DELAY(playLoveStartAnimation, 1000),
    ANIM_WITH_DELAY(playLoveAnimation, 100),
    ANIM_WITH_DELAY(playLoveAnimation, 100),
    ANIM_WITH_DELAY(playLoveAnimation, 100),
    ANIM_WITH_DELAY(playLoveAnimation, 100),
    ANIM_WITH_DELAY(playLoveAnimation, 100),
    ANIM_WITH_DELAY(playLoveEndAnimation, 1000),
};

AnimationEntry SleepAnimationSequence[] = {
    ANIM_WITH_DELAY(playSleepStartAnimation, 1000),
    ANIM_WITH_DELAY(playSleepAnimation, 100),
    ANIM_WITH_DELAY(playSleepAnimation, 100),
    ANIM_WITH_DELAY(playSleepAnimation, 100),
    ANIM_WITH_DELAY(playSleepAnimation, 100),
    ANIM_WITH_DELAY(playSleepAnimation, 100),
    ANIM_WITH_DELAY(playSleepAnimation, 100),
    ANIM_WITH_DELAY(playSleepAnimation, 100),
    ANIM_WITH_DELAY(playSleepEndAnimation, 2000),
};

AnimationEntry ThumbAnimationSequence[] = { 
    ANIM(playThumbStartAnimation),
    ANIM(playThumbAnimation),
    ANIM(playThumbAnimation),
    ANIM(playThumbAnimation),
    ANIM(playThumbAnimation),
    ANIM(playThumbEndAnimation),
};

AnimationEntry WaveAnimationSequence[] = {
    ANIM(playWaveStartAnimation),
    ANIM(playWaveAnimation),
    ANIM(playWaveAnimation),
    ANIM(playWaveAnimation),
    ANIM(playWaveAnimation),
    ANIM(playWaveEndAnimation),
};

// Array of all available animation sequences (defined after all sequences)
SequenceInfo allSequences[] = {
    {idleAnimationSequence, sizeof(idleAnimationSequence) / sizeof(idleAnimationSequence[0])},
    {idleAnimationSequence, sizeof(idleAnimationSequence) / sizeof(idleAnimationSequence[0])},
    {idleAnimationSequence, sizeof(idleAnimationSequence) / sizeof(idleAnimationSequence[0])},
    {happyAnimationSequence, sizeof(happyAnimationSequence) / sizeof(happyAnimationSequence[0])},
    {EnjoyingAnimationSequence, sizeof(EnjoyingAnimationSequence) / sizeof(EnjoyingAnimationSequence[0])},
    {AngryAnimationSequence, sizeof(AngryAnimationSequence) / sizeof(AngryAnimationSequence[0])},
    {idleAnimationSequence, sizeof(idleAnimationSequence) / sizeof(idleAnimationSequence[0])},
    {idleAnimationSequence, sizeof(idleAnimationSequence) / sizeof(idleAnimationSequence[0])},
    {SadAnimationSequence, sizeof(SadAnimationSequence) / sizeof(SadAnimationSequence[0])},
    {VerySadAnimationSequence, sizeof(VerySadAnimationSequence) / sizeof(VerySadAnimationSequence[0])},
    {CryAnimationSequence, sizeof(CryAnimationSequence) / sizeof(CryAnimationSequence[0])},
    {FunnyAnimationSequence, sizeof(FunnyAnimationSequence) / sizeof(FunnyAnimationSequence[0])},
    {EnjoyingAnimationSequence, sizeof(EnjoyingAnimationSequence) / sizeof(EnjoyingAnimationSequence[0])},
    {EnjoyingAnimationSequence, sizeof(EnjoyingAnimationSequence) / sizeof(EnjoyingAnimationSequence[0])},
    {EnjoyingAnimationSequence, sizeof(EnjoyingAnimationSequence) / sizeof(EnjoyingAnimationSequence[0])},
    {EnjoyingAnimationSequence, sizeof(EnjoyingAnimationSequence) / sizeof(EnjoyingAnimationSequence[0])},
    {EnjoyingAnimationSequence, sizeof(EnjoyingAnimationSequence) / sizeof(EnjoyingAnimationSequence[0])},
    {LoveAnimationSequence, sizeof(LoveAnimationSequence) / sizeof(LoveAnimationSequence[0])},
    {SleepAnimationSequence, sizeof(SleepAnimationSequence) / sizeof(SleepAnimationSequence[0])},
    {ThumbAnimationSequence, sizeof(ThumbAnimationSequence) / sizeof(ThumbAnimationSequence[0])},
    {WaveAnimationSequence, sizeof(WaveAnimationSequence) / sizeof(WaveAnimationSequence[0])},
};

const int TOTAL_SEQUENCES = sizeof(allSequences) / sizeof(allSequences[0]);

// Function to select animation sequence based on mood, or random if mood not set
void selectAnimationSequence() {
    // Check mood and select corresponding sequence
    if (mood == "idle") {
        currentAnimationSequence = idleAnimationSequence;
        currentAnimationSequenceLength = sizeof(idleAnimationSequence) / sizeof(idleAnimationSequence[0]);
        currentSequenceIndex = 0;
    } else if (mood == "happy") {
        currentAnimationSequence = happyAnimationSequence;
        currentAnimationSequenceLength = sizeof(happyAnimationSequence) / sizeof(happyAnimationSequence[0]);
        currentSequenceIndex = 1;
    } else if (mood == "enjoying") {
        currentAnimationSequence = EnjoyingAnimationSequence;
        currentAnimationSequenceLength = sizeof(EnjoyingAnimationSequence) / sizeof(EnjoyingAnimationSequence[0]);
        currentSequenceIndex = 2;
    } else if (mood == "angry") {
        currentAnimationSequence = AngryAnimationSequence;
        currentAnimationSequenceLength = sizeof(AngryAnimationSequence) / sizeof(AngryAnimationSequence[0]);
        currentSequenceIndex = 3;
    } else if (mood == "sad") {
        currentAnimationSequence = SadAnimationSequence;
        currentAnimationSequenceLength = sizeof(SadAnimationSequence) / sizeof(SadAnimationSequence[0]);
        currentSequenceIndex = 4;
    } else if (mood == "verysad") {
        currentAnimationSequence = VerySadAnimationSequence;
        currentAnimationSequenceLength = sizeof(VerySadAnimationSequence) / sizeof(VerySadAnimationSequence[0]);
        currentSequenceIndex = 5;
    } else if (mood == "cry") {
        currentAnimationSequence = CryAnimationSequence;
        currentAnimationSequenceLength = sizeof(CryAnimationSequence) / sizeof(CryAnimationSequence[0]);
        currentSequenceIndex = 6;
    } else if (mood == "funny") {
        currentAnimationSequence = FunnyAnimationSequence;
        currentAnimationSequenceLength = sizeof(FunnyAnimationSequence) / sizeof(FunnyAnimationSequence[0]);
        currentSequenceIndex = 7;
    } else if (mood == "love") {
        currentAnimationSequence = LoveAnimationSequence;
        currentAnimationSequenceLength = sizeof(LoveAnimationSequence) / sizeof(LoveAnimationSequence[0]);
        currentSequenceIndex = 8;
    } else if (mood == "sleep") {
        currentAnimationSequence = SleepAnimationSequence;
        currentAnimationSequenceLength = sizeof(SleepAnimationSequence) / sizeof(SleepAnimationSequence[0]);
        currentSequenceIndex = 9;
    } else if (mood == "thumbup") {
        currentAnimationSequence = ThumbAnimationSequence;
        currentAnimationSequenceLength = sizeof(ThumbAnimationSequence) / sizeof(ThumbAnimationSequence[0]);
        currentSequenceIndex = 10;
    } else if (mood == "wave") {
        currentAnimationSequence = WaveAnimationSequence;
        currentAnimationSequenceLength = sizeof(WaveAnimationSequence) / sizeof(WaveAnimationSequence[0]);
        currentSequenceIndex = 11;
    } else {
        // No specific mood set - randomly select from all sequences
        int randomIndex = random(0, TOTAL_SEQUENCES);
        
        // Make sure we don't pick the same sequence twice in a row
        while (randomIndex == currentSequenceIndex && TOTAL_SEQUENCES > 1) {
            randomIndex = random(0, TOTAL_SEQUENCES);
        }
        
        currentSequenceIndex = randomIndex;
        currentAnimationSequence = allSequences[currentSequenceIndex].sequence;
        currentAnimationSequenceLength = allSequences[currentSequenceIndex].length;
        
        // Serial.print("Selected random sequence: ");
        Serial.println(currentSequenceIndex);
    }
    
    animationIndex = 0; // Reset to start of new sequence
}

void handleSerialCommand(String command) {
    command.trim();
    if (command.length() == 0) {
        return;
    }

    String lowerCommand = command;
    lowerCommand.toLowerCase();

    Serial.print(F("Received: "));
    Serial.println(command);

    if (lowerCommand == "ping") {
        Serial.println(F("pong"));
        display_text("pong");
        delay(500);
        message = "";
        return;
    }

    if (lowerCommand == "jump") {
        serialJumpPressed = true;
        return;
    }

    if (lowerCommand == "tickle") {
        playTickleStartAnimation();
        playTickleAnimation();
        playTickleAnimation();
        playTickleEndAnimation();
        message = "";
        return;
    }

    if (lowerCommand == "loveyou") {
        playLoveYouAnimation();
        message = "";
        return;
    }

    if (lowerCommand.startsWith("mode:")) {
        String modeStr = lowerCommand.substring(5);
        modeStr.trim();

        if (modeStr == "weather") {
            currentMode = MODE_WEATHER;
            Serial.println(F("Switched to Weather mode"));
        } else if (modeStr == "game") {
            currentMode = MODE_GAME;
            serialJumpPressed = false;
            Serial.println(F("Switched to Game mode. Send 'jump' to play."));
        } else if (modeStr == "horse") {
            currentMode = MODE_HORSE;
            Serial.println(F("Switched to Horse mode"));
        } else if (modeStr == "animation") {
            currentMode = MODE_ANIMATION;
            Serial.println(F("Switched to Animation mode"));
        } else if (modeStr == "clock") {
            currentMode = MODE_CLOCK;
            Serial.println(F("Switched to Clock mode"));
        } else if (modeStr == "progress") {
            currentMode = MODE_PROGRESS;
            message = "";
            Serial.println(F("Switched to Progress mode"));
            Serial.println(F("Use: progress:<0-100>:<top>:<bottom>"));
        } else {
            display_text(("Unknown mode: " + modeStr).c_str());
            Serial.println(F("Unknown mode. Use: mode:horse, mode:animation, mode:weather, mode:game, mode:clock, or mode:progress"));
            delay(2000);
        }
        displayCurrentMode();
    } else if (lowerCommand.startsWith("cpu:")) {
        String percentStr = lowerCommand.substring(4);
        percentStr.trim();
        int value = (int)percentStr.toFloat();
        if (value < 0) value = 0;
        if (value > 100) value = 100;
        cpuPercent = value;
        currentMode = MODE_HORSE;
        Serial.print(F("CPU "));
        Serial.print(cpuPercent);
        Serial.println(F("%"));
    } else if (lowerCommand.startsWith("progress:")) {
        // Formats:
        //   progress:50
        //   progress:50:bottom text
        //   progress:50:top text:bottom text
        String payload = command.substring(9); // keep original case for labels
        payload.trim();

        int firstColon = payload.indexOf(':');
        String percentStr = (firstColon < 0) ? payload : payload.substring(0, firstColon);
        percentStr.trim();

        int value = percentStr.toInt();
        if (value < 0) value = 0;
        if (value > 100) value = 100;
        progressPercent = value;

        if (firstColon >= 0) {
            String rest = payload.substring(firstColon + 1);
            int secondColon = rest.indexOf(':');
            if (secondColon < 0) {
                progressBottom = rest;
                progressBottom.trim();
            } else {
                progressTop = rest.substring(0, secondColon);
                progressTop.trim();
                progressBottom = rest.substring(secondColon + 1);
                progressBottom.trim();
            }
        }

        currentMode = MODE_PROGRESS;
        message = "";
        Serial.print(F("Progress "));
        Serial.print(progressPercent);
        Serial.print(F("% | "));
        Serial.print(progressTop);
        Serial.print(F(" | "));
        Serial.println(progressBottom);
    } else if (lowerCommand.startsWith("weather:")) {
        String weatherCommand = lowerCommand.substring(8);
        weatherCommand.trim();

        int pos = 0;
        int nextPos = weatherCommand.indexOf(":", pos);

        if (nextPos > 0) {
            currentCity = weatherCommand.substring(pos, nextPos);
            pos = nextPos + 1;
        } else if (nextPos == 0) {
            Serial.println(F("Invalid weather format: city cannot be empty"));
            return;
        } else {
            currentCity = weatherCommand;
            pos = weatherCommand.length();
        }

        if (pos < weatherCommand.length()) {
            nextPos = weatherCommand.indexOf(":", pos);
            if (nextPos > pos) {
                currentTemperature = weatherCommand.substring(pos, nextPos).toFloat();
                pos = nextPos + 1;
            } else if (nextPos == -1) {
                currentTemperature = weatherCommand.substring(pos).toFloat();
                pos = weatherCommand.length();
            } else {
                Serial.println(F("Invalid weather format: missing temperature"));
                return;
            }
        }

        if (pos < weatherCommand.length()) {
            nextPos = weatherCommand.indexOf(":", pos);
            if (nextPos > pos) {
                currentFeelsLike = weatherCommand.substring(pos, nextPos).toFloat();
                pos = nextPos + 1;
            } else if (nextPos == -1) {
                currentFeelsLike = weatherCommand.substring(pos).toFloat();
                pos = weatherCommand.length();
            } else {
                Serial.println(F("Invalid weather format: missing feels_like"));
                return;
            }
        }

        if (pos < weatherCommand.length()) {
            nextPos = weatherCommand.indexOf(":", pos);
            if (nextPos > pos) {
                currentHumidity = weatherCommand.substring(pos, nextPos).toInt();
                currentDescription = weatherCommand.substring(nextPos + 1);
            } else {
                currentHumidity = weatherCommand.substring(pos).toInt();
                currentDescription = "";
            }
            Serial.println(F("Weather data updated"));
        } else {
            Serial.println(F("Invalid weather format: missing humidity"));
        }
    } else if (lowerCommand.startsWith("message:")) {
        message = lowerCommand.substring(8);
        message.trim();
    } else if (lowerCommand.startsWith("mood:")) {
        String moodStr = lowerCommand.substring(5);
        moodStr.trim();
        moodStr.toLowerCase();
        mood = moodStr;
        Serial.println("Mood set to: " + mood);
        if (currentMode == MODE_ANIMATION) {
            selectAnimationSequence();
            lastAnimationTime = 0;
        }
    } else if (lowerCommand.startsWith("time:")) {
        String clockCommand = lowerCommand.substring(5);
        clockCommand.trim();
        if (setTimeFromString(clockCommand)) {
            Serial.println(F("Clock time set successfully"));
        } else {
            Serial.println(F("Invalid clock format. Use: time:HH:MM:SS or time:HH:MM:SS DD/MM/YYYY"));
        }
    }
}

void pollSerialInput() {
    bool gotByte = false;
    while (Serial.available() > 0) {
        char c = (char)Serial.read();
        gotByte = true;
        serialLastByteMs = millis();

        if (c == '\n' || c == '\r') {
            if (serialLineBuffer.length() > 0) {
                handleSerialCommand(serialLineBuffer);
                serialLineBuffer = "";
            }
        } else if (c >= 32 && c <= 126) {
            // printable ASCII only
            serialLineBuffer += c;
            if (serialLineBuffer.length() > 256) {
                serialLineBuffer = "";
            }
        }
    }

    // If Serial Monitor uses "No line ending", finish the command after a short idle
    if (!gotByte && serialLineBuffer.length() > 0 &&
        (millis() - serialLastByteMs) >= SERIAL_LINE_TIMEOUT_MS) {
        handleSerialCommand(serialLineBuffer);
        serialLineBuffer = "";
    }
}

void loop() {
    pollSerialInput();

    // Keep default mode in sync with serial host connect/disconnect
    static bool lastSerialConnected = (bool)Serial;
    bool serialConnected = (bool)Serial;
    if (serialConnected != lastSerialConnected) {
        lastSerialConnected = serialConnected;
        if (serialConnected) {
            if (currentMode == MODE_ANIMATION) {
                currentMode = MODE_HORSE;
            }
        } else if (currentMode == MODE_HORSE) {
            currentMode = MODE_ANIMATION;
        }
    }

    if (message.length() > 0) {
        display_text(message.c_str());
        return;
    }

    switch (currentMode) {
        case MODE_HORSE: {
            playHorseFrame(horseFrameIndex, horseFrameDelayFromCpu(cpuPercent));
            break;
        }
        case MODE_ANIMATION: {
            unsigned long currentTime = millis();

            if (currentAnimationSequence == nullptr || animationIndex >= currentAnimationSequenceLength) {
                if (mood == "love") {
                    mood = "random";
                    Serial.println(F("Love sequence completed - mood reset to 'random'"));
                }
                selectAnimationSequence();
            }

            unsigned long requiredDelay = ANIMATION_DELAY;
            if (currentAnimationSequence != nullptr && animationIndex < currentAnimationSequenceLength) {
                if (currentAnimationSequence[animationIndex].delayMs > 0) {
                    requiredDelay = currentAnimationSequence[animationIndex].delayMs;
                }
            }

            if (currentTime - lastAnimationTime >= requiredDelay) {
                if (currentAnimationSequence != nullptr && animationIndex < currentAnimationSequenceLength) {
                    pollSerialInput();
                    currentAnimationSequence[animationIndex].func();
                    pollSerialInput();
                    animationIndex++;
                    lastAnimationTime = millis();
                }
            }
            break;
        }
        case MODE_WEATHER: {
            displayWeatherOnOLED(currentCity, currentTemperature, currentFeelsLike, currentHumidity, currentDescription);
            break;
        }
        case MODE_GAME: {
            bool jump = serialJumpPressed;
            serialJumpPressed = false;
            runDinoGame(jump);
            break;
        }
        case MODE_CLOCK: {
            updateClock();
            break;
        }
        case MODE_PROGRESS: {
            displayProgressScreen();
            break;
        }
    }
}
