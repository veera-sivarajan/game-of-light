/*  
 *  Hayden Tierney & Veera Sivarajan
 *  CICS 256: Make
 *  Fall 2021
 *  Final Project: Game of Light
 *  Conway's Game of Life Simulation
 *  
 *  Rules
 *  1. Any live cell with fewer than two live neighbours dies, as if by underpopulation.
 *  2. Any live cell with two or three live neighbours lives on to the next generation.
 *  3. Any live cell with more than three live neighbours dies, as if by overpopulation.
 *  4. Any dead cell with exactly three live neighbours becomes a live cell, as if by reproduction.
 */

// ************ LIBRARIES ************
#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>

// ************ PIN/BUTTON DEFINITIONS ************
#define PIN A13
#define AUTOPLAY_BUTTON 34
#define STEPGEN_BUTTON 0

// ************ DEFINE DEBOUNCE TIME (IN MILLISECONDS) ************
#define DEBOUNCE 250

// ************ COLOR DEFINITIONS ************
#define BLACK     0x0000
#define BLUE      0x001F
#define RED       0xF800
#define GREEN     0x07E0
#define CYAN      0x07FF
#define MAGENTA   0xF81F
#define YELLOW    0xFFE0 
#define WHITE     0xFFFF

const uint16_t colors[] = { RED, BLUE, GREEN };
// ************ DEFINE DISPLAY DIMENSIONS ************
#define WIDTH     30
#define HEIGHT    30

// ************ DEFINE INITIAL POPULATION SPARCITY ************
#define SPARCITY 6

// ************ INITIALIZE CASCADING MATRIX DISPLAY ************
Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(16, 16, 2, 2, PIN,
  NEO_MATRIX_TOP + NEO_MATRIX_LEFT + NEO_MATRIX_ROWS + NEO_MATRIX_ZIGZAG
  + NEO_TILE_TOP + NEO_TILE_LEFT + NEO_TILE_ROWS + NEO_TILE_ZIGZAG,
  NEO_GRB + NEO_KHZ800);

// ************ INITIALIZE MEMORY/BUFFER MATRICES ************
int matrix1 [HEIGHT + 2][WIDTH + 2];
int matrix2 [HEIGHT][WIDTH];

volatile bool autoplay = LOW;
volatile unsigned long last_autoplay_click_time = 0;

volatile bool stepGen = LOW;
volatile unsigned long last_stepGen_click_time = 0;

void IRAM_ATTR toggleAutoplay() { // ISR
  if (millis() > last_autoplay_click_time + DEBOUNCE) {
    autoplay = !autoplay;
    last_autoplay_click_time = millis();
  }
}

void IRAM_ATTR stepGenOn() { // ISR
  if (millis() > last_stepGen_click_time + DEBOUNCE) {
    stepGen = HIGH;
    last_stepGen_click_time = millis();
  }
}

void setup() {
  matrix.begin();
  matrix.setBrightness(32);
  
  matrix.clear();
  matrix.fillScreen(0);
  
  generateRandomWorld();
  
  pinMode(AUTOPLAY_BUTTON, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(AUTOPLAY_BUTTON), toggleAutoplay, FALLING);
  
  pinMode(STEPGEN_BUTTON, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(STEPGEN_BUTTON), stepGenOn, FALLING);
  
  showWorld();
}

void loop() {
  if (autoplay) {
    getNextGen();
    delay(250);
    showWorld();
  }
  else {
    if (stepGen) {
      getNextGen();
      showWorld();
      stepGen = LOW;
    }
  }
}

void generateRandomWorld() {
  randomSeed(analogRead(0));
  for (int x = 0; x < HEIGHT + 2; x++) {
    for (int y = 0; y < WIDTH + 2; y++) {
      // Perma-dead border
      if (x == 0 || x == (HEIGHT + 1) || y == 0 || y == (WIDTH + 1)) {
        matrix1[x][y] = 2;
      }
      else {
        int cell = random(SPARCITY);
        if (cell == 0) {
          matrix1[x][y] = 1;
        }
        else {
          matrix1[x][y] = 0;
        }
      }
    }
  }
}

void showWorld() {
  for (int x = 0; x < HEIGHT + 2; x++) {
    for (int y = 0; y < WIDTH + 2; y++) {
      matrix.drawPixel(x, y, colors[matrix1[x][y]]);
    }
  }
  matrix.show();
}

// Apply Conways rules to get the next generation
void getNextGen() {
  // purge the buffer matrix (kill all cells)
  for (int x = 0; x < HEIGHT; x++) {
    for (int y = 0; y < WIDTH; y++) {
      matrix2[x][y] = 0;
    }
  }
  
  // check number of live neighbours
  for (int x = 1; x < HEIGHT + 1; x++) {
    for (int y = 1; y < WIDTH + 1; y++) {
      int neighbours = 0;
      // count how many neighbours around the cell
      if (matrix1[x - 1][y - 1] == 1) { neighbours++; }
      if (matrix1[x][y - 1] == 1) { neighbours++; }
      if (matrix1[x + 1][y - 1] == 1) { neighbours++; }
      if (matrix1[x - 1][y] == 1) { neighbours++; }
      if (matrix1[x + 1][y] == 1) { neighbours++; }
      if (matrix1[x - 1][y + 1] == 1) { neighbours++; }
      if (matrix1[x][y + 1] == 1) { neighbours++; }
      if (matrix1[x + 1][y + 1] == 1) { neighbours++; }
      
      // apply rules
      if (matrix1[x][y] == 1 && (neighbours == 2 || neighbours == 3)) { matrix2[x - 1][y - 1] = 1; }
      if (matrix1[x][y] == 0 && neighbours == 3 ) { matrix2[x - 1][y - 1] = 1; }
    }
  }

  // copy the buffer matrix while keeping the perma-dead border
  for (int x = 1; x < HEIGHT + 1; x++) {
    for (int y = 1; y < WIDTH + 1; y++) {
      matrix1[x][y] = matrix2[x - 1][y - 1];
    }
  }
}
