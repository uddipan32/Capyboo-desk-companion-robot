#include "bluetooth.h"
#include "display.h"
#include <Wire.h>

#include "face_animation.h"
#include "weather.h"
#include "dino_game.h"
#include "clock.h"

// Touch sensor pin (from definitions.h)
const int TOUCH_SENSOR_PIN = 4;

// Mode definitions
enum Mode {
    MODE_ANIMATION,
    MODE_WEATHER,
    MODE_GAME,
    MODE_CLOCK
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

String mood = "idle";

Mode currentMode = MODE_ANIMATION;  // Default mode


void setup() {
    Serial.begin(115200);

    
    // Initialize BLE Serial
    initBLESerial("Capyboo");

    // Initialize touch sensor
    pinMode(TOUCH_SENSOR_PIN, INPUT);

    // Initialize display
    Wire.begin();
    if (!display.begin(0x3C, true)) {
        Serial.println(F("SH1106 allocation failed"));
        for (;;); // Don't proceed, loop forever
    }

    display.clearDisplay();
    display.setTextColor(SH110X_WHITE);
    display.setTextSize(3);
    
    // Center the text
    int16_t x1, y1;
    uint16_t w, h;
    display.getTextBounds("Capyboo", 0, 0, &x1, &y1, &w, &h);
    int16_t x = (128 - w) / 2;
    int16_t y = (64 - h) / 2;
    display.setCursor(x, y);
    display.println("Capyboo");
    display.display();
    delay(2000);
    
    // Play wakeup animation once at startup
    playWakeupAnimation();
    
    // Initialize random seed for sequence selection
    randomSeed(analogRead(0));
    
    // Select initial animation sequence based on mood
    selectAnimationSequence();
    
    // Display current mode
    displayCurrentMode();
}

// Function to display current mode
void displayCurrentMode() {
    String modeText = "Mode: ";
    switch (currentMode) {
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
    }
}

// Animation sequence state
int animationIndex = 0;
unsigned long lastAnimationTime = 0;
const unsigned long ANIMATION_DELAY = 1000; // Default delay between animations in ms
int currentSequenceIndex = -1; // Track which sequence we're currently playing

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
};

AnimationEntry EnjoyingAnimationSequence[] = {
    ANIM_WITH_DELAY(playEnjoyStartAnimation, 1000),             // 0 - 1 second delay
    ANIM(playEnjoyingAnimation),
    ANIM(playEnjoyingAnimation),
    ANIM(playEnjoyingAnimation),
    ANIM(playEnjoyingAnimation),
    ANIM(playEnjoyingAnimation),
    ANIM(playEnjoyEndAnimation),             // 2 - 1 second delay
    ANIM_WITH_DELAY(playEnjoyEndAnimation, 2000),             // 3 - 1 second delay
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
    ANIM_WITH_DELAY(playIdleToSadAnimation, 1000),             // 0 - 1 second delay
    ANIM_WITH_DELAY(playSadToCryAnimation, 2000),
    ANIM(playCryingAnimation),
    ANIM(playCryingAnimation),
    ANIM(playCryingAnimation),
    ANIM(playCryingAnimation),
    ANIM(playCryingAnimation),
    ANIM(playCryingAnimation),
    ANIM(playCryingAnimation),
    ANIM_WITH_DELAY(playCryToSadAnimation, 2000),
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
    ANIM(playLoveAnimation),
    ANIM(playLoveAnimation),
    ANIM(playLoveAnimation),
    ANIM(playLoveAnimation),
    ANIM_WITH_DELAY(playLoveEndAnimation, 2000),
};

AnimationEntry SleepAnimationSequence[] = {
    ANIM_WITH_DELAY(playSleepStartAnimation, 1000),
    ANIM(playSleepAnimation),
    ANIM(playSleepAnimation),
    ANIM(playSleepAnimation),
    ANIM(playSleepAnimation),   
    ANIM(playSleepAnimation),
    ANIM(playSleepAnimation),
    ANIM_WITH_DELAY(playSleepEndAnimation, 2000),
};
// Array of all available animation sequences (defined after all sequences)
SequenceInfo allSequences[] = {
    {idleAnimationSequence, sizeof(idleAnimationSequence) / sizeof(idleAnimationSequence[0])},
    {happyAnimationSequence, sizeof(happyAnimationSequence) / sizeof(happyAnimationSequence[0])},
    {EnjoyingAnimationSequence, sizeof(EnjoyingAnimationSequence) / sizeof(EnjoyingAnimationSequence[0])},
    {AngryAnimationSequence, sizeof(AngryAnimationSequence) / sizeof(AngryAnimationSequence[0])},
    {SadAnimationSequence, sizeof(SadAnimationSequence) / sizeof(SadAnimationSequence[0])},
    {VerySadAnimationSequence, sizeof(VerySadAnimationSequence) / sizeof(VerySadAnimationSequence[0])},
    {CryAnimationSequence, sizeof(CryAnimationSequence) / sizeof(CryAnimationSequence[0])},
    {FunnyAnimationSequence, sizeof(FunnyAnimationSequence) / sizeof(FunnyAnimationSequence[0])},
    {LoveAnimationSequence, sizeof(LoveAnimationSequence) / sizeof(LoveAnimationSequence[0])},
    {SleepAnimationSequence, sizeof(SleepAnimationSequence) / sizeof(SleepAnimationSequence[0])},
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
    }  else {
        // No specific mood set - randomly select from all sequences
        int randomIndex = random(0, TOTAL_SEQUENCES);
        
        // Make sure we don't pick the same sequence twice in a row
        while (randomIndex == currentSequenceIndex && TOTAL_SEQUENCES > 1) {
            randomIndex = random(0, TOTAL_SEQUENCES);
        }
        
        currentSequenceIndex = randomIndex;
        currentAnimationSequence = allSequences[currentSequenceIndex].sequence;
        currentAnimationSequenceLength = allSequences[currentSequenceIndex].length;
        
        Serial.print("Selected random sequence: ");
        Serial.println(currentSequenceIndex);
    }
    
    animationIndex = 0; // Reset to start of new sequence
}

void loop() {
    // Handle BLE connection/disconnection (required for BLE communication)
    handleBLESerial();

    if (bleSerialAvailable()) {
        String command = bleSerialRead();
        command.trim();
        String lowerCommand = command;
        lowerCommand.toLowerCase();
        
        Serial.print("Received BLE command: ");
        Serial.println(command);
        // Handle mode switching commands
        if (lowerCommand.startsWith("mode:")) {
            String modeStr = lowerCommand.substring(5); // Get text after "mode:"
            modeStr.trim();
            
            if (modeStr == "weather") {
                currentMode = MODE_WEATHER;
                bleSerialPrintln("Switched to Weather mode");
                displayCurrentMode();
            } else if (modeStr == "game") {
                currentMode = MODE_GAME;
                bleSerialPrintln("Switched to Game mode");
                displayCurrentMode();
            } else if (modeStr == "animation") {
                currentMode = MODE_ANIMATION;
                bleSerialPrintln("Switched to Animation mode");
                displayCurrentMode();
            } else if (modeStr == "clock") {
                currentMode = MODE_CLOCK;
                bleSerialPrintln("Switched to Clock mode");
                displayCurrentMode();
            } else {
                String errorMsg = "Unknown mode: " + modeStr;
                display_text(errorMsg.c_str());
                bleSerialPrintln("Unknown mode. Use: mode:animation, mode:weather, or mode:game");
                delay(2000);
            }
        } else if (lowerCommand.startsWith("weather:")) {
            Serial.print("Weather command: ");
            String weatherCommand = lowerCommand.substring(8); // Get text after "weather:"
            weatherCommand.trim();
            Serial.print("Parsing: ");
            Serial.println(weatherCommand);
            // command format: weather:city:temperature:feels_like:humidity:description
            // Parse the colon-separated values
            int pos = 0;
            int nextPos = weatherCommand.indexOf(":", pos);
            
            // Parse city (required)
            if (nextPos > 0) {
                currentCity = weatherCommand.substring(pos, nextPos);
                pos = nextPos + 1;
                Serial.print("City: ");
                Serial.println(currentCity);
            } else if (nextPos == 0) {
                // Colon at start - empty city
                bleSerialPrintln("Invalid weather format: city cannot be empty");
            } else {
                // No colon found - city is the entire string
                currentCity = weatherCommand;
                pos = weatherCommand.length();
                Serial.print("City (no more fields): ");
                Serial.println(currentCity);
            }
            
            // Parse temperature (required)
            if (pos < weatherCommand.length()) {
                nextPos = weatherCommand.indexOf(":", pos);
                if (nextPos > pos) {
                    currentTemperature = weatherCommand.substring(pos, nextPos).toFloat();
                    pos = nextPos + 1;
                    Serial.print("Temperature: ");
                    Serial.println(currentTemperature);
                } else if (nextPos == -1) {
                    // No more colons - temperature is the rest
                    currentTemperature = weatherCommand.substring(pos).toFloat();
                    pos = weatherCommand.length();
                    Serial.print("Temperature (last field): ");
                    Serial.println(currentTemperature);
                } else {
                    bleSerialPrintln("Invalid weather format: missing temperature");
                    pos = weatherCommand.length(); // Skip rest
                }
            }
            
            // Parse feels_like (required)
            if (pos < weatherCommand.length()) {
                nextPos = weatherCommand.indexOf(":", pos);
                if (nextPos > pos) {
                    currentFeelsLike = weatherCommand.substring(pos, nextPos).toFloat();
                    pos = nextPos + 1;
                    Serial.print("Feels like: ");
                    Serial.println(currentFeelsLike);
                } else if (nextPos == -1) {
                    // No more colons - feels_like is the rest
                    currentFeelsLike = weatherCommand.substring(pos).toFloat();
                    pos = weatherCommand.length();
                    Serial.print("Feels like (last field): ");
                    Serial.println(currentFeelsLike);
                } else {
                    bleSerialPrintln("Invalid weather format: missing feels_like");
                    pos = weatherCommand.length(); // Skip rest
                }
            }
            
            // Parse humidity (required, but description is optional)
            if (pos < weatherCommand.length()) {
                nextPos = weatherCommand.indexOf(":", pos);
                if (nextPos > pos) {
                    // Has description after humidity
                    currentHumidity = weatherCommand.substring(pos, nextPos).toInt();
                    pos = nextPos + 1;
                    currentDescription = weatherCommand.substring(pos);
                    Serial.print("Humidity: ");
                    Serial.println(currentHumidity);
                    Serial.print("Description: ");
                    Serial.println(currentDescription);
                } else if (nextPos == -1) {
                    // No description - humidity is the rest
                    currentHumidity = weatherCommand.substring(pos).toInt();
                    currentDescription = "";
                    Serial.print("Humidity (last field): ");
                    Serial.println(currentHumidity);
                } else {
                    bleSerialPrintln("Invalid weather format: missing humidity");
                }
                
                // Display the weather data
                displayWeatherOnOLED(currentCity, currentTemperature, currentFeelsLike, currentHumidity, currentDescription);
                bleSerialPrintln("Weather data updated");
            } else {
                bleSerialPrintln("Invalid weather format: missing humidity");
            }
        }
        else if (lowerCommand.startsWith("message:")) {
            message = lowerCommand.substring(8); // Get text after "message:"
            message.trim();
        } else if (lowerCommand.startsWith("mood:")) {
            String moodStr = lowerCommand.substring(5); // Get text after "mood:"
            moodStr.trim();
            moodStr.toLowerCase();
            mood = moodStr;
            bleSerialPrintln("Mood set to: " + mood);
            // Immediately switch to the mood's animation sequence
            if (currentMode == MODE_ANIMATION) {
                selectAnimationSequence();
            }
        }  else if (lowerCommand.startsWith("time:")) {
            String clockCommand = lowerCommand.substring(5); // Get text after "time:"
            clockCommand.trim();

            // command format: clock:HH:MM:SS or clock:HH:MM:SS DD/MM/YYYY
            // Use the setTimeFromString function from clock.h
            if (setTimeFromString(clockCommand)) {
                bleSerialPrintln("Clock time set successfully");
            } else {
                bleSerialPrintln("Invalid clock format. Use: clock:HH:MM:SS or clock:HH:MM:SS DD/MM/YYYY");
            }
        }

    }

   

    // if touch sensor is pressed, play tickle animation
    if (digitalRead(TOUCH_SENSOR_PIN) == HIGH && currentMode != MODE_GAME) {
        playTickleStartAnimation();
        playTickleAnimation();
        playTickleAnimation();
        playTickleEndAnimation();
        message = "";
    }

    if (message.length() > 0) {
        display_text(message.c_str());
        delay(500);
        return;
    }

    // Handle different modes
    switch (currentMode) {
        case MODE_ANIMATION: {
            unsigned long currentTime = millis();

            // If no sequence is selected or sequence is complete, select sequence based on mood (or random)
            if (currentAnimationSequence == nullptr || animationIndex >= currentAnimationSequenceLength) {
                selectAnimationSequence();
            }
            
            // Calculate delay for current animation (use custom delay or default)
            unsigned long requiredDelay = ANIMATION_DELAY;
            if (currentAnimationSequence != nullptr && animationIndex < currentAnimationSequenceLength) {
                if (currentAnimationSequence[animationIndex].delayMs > 0) {
                    requiredDelay = currentAnimationSequence[animationIndex].delayMs;
                }
            }
            
            // Check if enough time has passed (non-blocking delay)
            if (currentTime - lastAnimationTime >= requiredDelay) {
                // Play next animation in sequence
                if (currentAnimationSequence != nullptr && animationIndex < currentAnimationSequenceLength) {
                    currentAnimationSequence[animationIndex].func(); // Call the animation function
                }
                
                animationIndex++;
                // When sequence completes, select a new random sequence on next loop iteration
                if (animationIndex >= currentAnimationSequenceLength) {
                    // Sequence complete - will select new random sequence on next loop
                    animationIndex = currentAnimationSequenceLength; // Set to length so condition above triggers
                }
                
                lastAnimationTime = currentTime;
            }
            break;
        }
        case MODE_WEATHER: {
            // Weather mode - fetch and display weather
            displayWeatherOnOLED(currentCity, currentTemperature, currentFeelsLike, currentHumidity, currentDescription);
            break;
        }
            
        case MODE_GAME: {
            // Game mode - run dino game
            bool touchPressed = digitalRead(TOUCH_SENSOR_PIN) == HIGH;
            runDinoGame(touchPressed);
            break;
        } 
        case MODE_CLOCK: {
            // Clock mode - display clock
            updateClock();

            break;
        }
    }    
}
