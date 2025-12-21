/*
Dino Game for ESP32 OLED 128x64
Touch sensor to jump, avoid obstacles
*/

#ifndef dino_game_h
#define dino_game_h

// Game constants
#define GROUND_Y 50        // Ground level (Y position)
#define DINO_X 10          // Dino X position (fixed)
#define DINO_SIZE 12       // Dino size
#define OBSTACLE_WIDTH 8   // Obstacle width
#define OBSTACLE_HEIGHT 16 // Obstacle height
#define JUMP_HEIGHT 35     // Maximum jump height
#define GRAVITY 1          // Gravity force
#define JUMP_SPEED -10      // Initial jump velocity

// Game state variables
bool gameRunning = false;
bool gameOver = false;
int dinoY = GROUND_Y;           // Dino Y position
int dinoVelocity = 0;            // Dino vertical velocity
int obstacleX = 128;             // Obstacle X position (starts off-screen)
int obstacleY = GROUND_Y;        // Obstacle Y position
int score = 0;
int highScore = 0;
bool obstaclePassed = false;     // Track if obstacle was passed for scoring
unsigned long lastGameUpdate = 0;
unsigned long lastObstacleSpawn = 0;
int gameSpeed = 5;               // Obstacle movement speed

// Dino sprite (simple rectangle-based dino)
void drawDino(int x, int y) {
    // Dino body
    display.fillRect(x, y - DINO_SIZE, 8, DINO_SIZE, SH110X_WHITE);
    // Dino head
    display.fillRect(x + 6, y - DINO_SIZE - 4, 4, 4, SH110X_WHITE);
    // Dino legs (alternating for animation)
    static bool legState = false;
    legState = !legState;
    if (legState) {
        display.fillRect(x + 2, y, 2, 2, SH110X_WHITE);
        display.fillRect(x + 6, y, 2, 2, SH110X_WHITE);
    } else {
        display.fillRect(x + 1, y, 2, 2, SH110X_WHITE);
        display.fillRect(x + 7, y, 2, 2, SH110X_WHITE);
    }
}

// Draw obstacle (cactus)
void drawObstacle(int x, int y) {
    // Main body
    display.fillRect(x, y - OBSTACLE_HEIGHT, OBSTACLE_WIDTH, OBSTACLE_HEIGHT, SH110X_WHITE);
    // Top spike
    display.fillRect(x + 2, y - OBSTACLE_HEIGHT - 4, 4, 4, SH110X_WHITE);
    // Side spikes
    display.fillRect(x - 2, y - OBSTACLE_HEIGHT + 4, 2, 4, SH110X_WHITE);
    display.fillRect(x + OBSTACLE_WIDTH, y - OBSTACLE_HEIGHT + 6, 2, 4, SH110X_WHITE);
}

// Draw ground line
void drawGround() {
    display.drawLine(0, GROUND_Y + 2, 128, GROUND_Y + 2, SH110X_WHITE);
}

// Check collision between dino and obstacle
bool checkCollision() {
    int dinoLeft = DINO_X;
    int dinoRight = DINO_X + 8;
    int dinoTop = dinoY - DINO_SIZE;
    int dinoBottom = dinoY;
    
    int obstacleLeft = obstacleX;
    int obstacleRight = obstacleX + OBSTACLE_WIDTH;
    int obstacleTop = obstacleY - OBSTACLE_HEIGHT;
    int obstacleBottom = obstacleY;
    
    return (dinoRight > obstacleLeft && dinoLeft < obstacleRight &&
            dinoBottom > obstacleTop && dinoTop < obstacleBottom);
}

// Initialize game
void initGame() {
    gameRunning = true;
    gameOver = false;
    dinoY = GROUND_Y;
    dinoVelocity = 0;
    obstacleX = 128;
    obstacleY = GROUND_Y;
    score = 0;
    obstaclePassed = false;
    gameSpeed = 3;
    lastGameUpdate = millis();
    lastObstacleSpawn = millis();
}

// Update game physics
void updateGame() {
    if (!gameRunning || gameOver) return;
    
    unsigned long currentTime = millis();
    
    // Update dino physics (gravity and jump)
    dinoVelocity += GRAVITY;
    dinoY += dinoVelocity;
    
    // Keep dino on ground
    if (dinoY > GROUND_Y) {
        dinoY = GROUND_Y;
        dinoVelocity = 0;
    }
    
    // Update obstacle position
    if (currentTime - lastGameUpdate > 50) { // Update every 50ms
        obstacleX -= gameSpeed;
        lastGameUpdate = currentTime;
        
        // Increase score when obstacle passes dino
        if (!obstaclePassed && obstacleX + OBSTACLE_WIDTH < DINO_X) {
            score++;
            obstaclePassed = true;
            // Increase game speed every 10 points
            if (score % 10 == 0 && gameSpeed < 8) {
                gameSpeed++;
            }
        }
    }
    
    // Spawn new obstacle when current one is off-screen
    if (obstacleX + OBSTACLE_WIDTH < 0) {
        obstacleX = 128;
        obstaclePassed = false;
        // Randomize obstacle height slightly (0-4 pixels up)
        obstacleY = GROUND_Y - (random(0, 3) * 2);
    }
    
    // Check collision
    if (checkCollision()) {
        gameOver = true;
        gameRunning = false;
        if (score > highScore) {
            highScore = score;
        }
    }
}

// Make dino jump
void dinoJump() {
    if (dinoY >= GROUND_Y && gameRunning && !gameOver) {
        dinoVelocity = JUMP_SPEED;
    }
}

// Draw game screen
void drawGame() {
    display.clearDisplay();
    
    if (gameOver) {
        // Game Over screen
        display.setTextColor(SH110X_WHITE);
        display.setTextSize(2);
        display.setCursor(15, 15);
        display.print("GAME");
        display.setCursor(20, 35);
        display.print("OVER");
        
        display.setTextSize(1);
        display.setCursor(10, 55);
        display.print("Score: ");
        display.print(score);
        
        display.setCursor(70, 55);
        display.print("HS: ");
        display.print(highScore);
        
    } else if (!gameRunning) {
        // Start screen
        display.setTextColor(SH110X_WHITE);
        display.setTextSize(1);
        display.setCursor(25, 20);
        display.print("DINO GAME");
        display.setCursor(15, 35);
        display.print("Touch to jump");
        display.setCursor(20, 50);
        display.print("Touch to start");
        
    } else {
        // Game running
        drawGround();
        drawDino(DINO_X, dinoY);
        drawObstacle(obstacleX, obstacleY);
        
        // Draw score
        display.setTextColor(SH110X_WHITE);
        display.setTextSize(1);
        display.setCursor(90, 5);
        display.print("Score:");
        display.setCursor(90, 15);
        display.print(score);
    }
    
    display.display();
}

// Main game loop function
void runDinoGame(bool touchPressed) {
    static bool lastTouchState = false;
    bool touchJustPressed = touchPressed && !lastTouchState;
    lastTouchState = touchPressed;
    
    if (!gameRunning && touchJustPressed) {
        // Start game
        initGame();
    }
    
    if (gameRunning && !gameOver) {
        // Handle jump
        if (touchJustPressed) {
            dinoJump();
        }
        
        // Update game
        updateGame();
    }
    
    if (gameOver && touchJustPressed) {
        // Restart game
        initGame();
    }
    
    // Draw game
    drawGame();
}

#endif

