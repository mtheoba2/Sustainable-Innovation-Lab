#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <math.h>

// ---------- TFT ----------
#define TFT_CS   10
#define TFT_DC    9
#define TFT_RST   8
Adafruit_ILI9341 tft(TFT_CS, TFT_DC, TFT_RST);

// ---------- Ultrasonic ----------
#define TRIG1 6
#define ECHO1 7
#define TRIG2 4
#define ECHO2 5

float readDistanceCM(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 30000); // 30ms timeout
  if (duration == 0) return -1;                 // no echo
  return duration * 0.0343f / 2.0f;
}

// ---------- Buzzer (tone) ----------
#define BUZZER_PIN 3

void playGreenSound() {
  // C-E-G, with slightly longer G
  tone(BUZZER_PIN, 262, 120); // C4
  delay(150);
  tone(BUZZER_PIN, 330, 120); // E4
  delay(150);
  tone(BUZZER_PIN, 392, 350); // G4 (longer)
  delay(380);
  noTone(BUZZER_PIN);
}

// ---------- Behavior tuning ----------
const float NORMAL_CM = 17.0;
const float TOLERANCE_CM = 2.0;
const unsigned long TRANSIENT_MAX_MS = 1000;
const unsigned long GREEN_HOLD_MS = 2000;
const unsigned long ALARM_CLEAR_MS = 2000;

// checks deviation from sensor distances
bool isNormalDistance(float d) {
  if (d < 0) return true;
  return fabs(d - NORMAL_CM) <= TOLERANCE_CM;
}

// ---------- Color palette (set after tft.begin) ----------
uint16_t NEUTRAL_BG;   // navy idle background
uint16_t SUCCESS_BG;   // green success background
uint16_t TEXT_IDLE;    // idle text/icon color
uint16_t TEXT_OK;      // success text/icon color

// ---------- Drawing helpers ----------
void drawCenteredText(const char* msg, int16_t y, uint8_t size, uint16_t fg, uint16_t bg) {
  tft.setTextSize(size);
  tft.setTextWrap(false);

  int16_t x1, y1;
  uint16_t w, h;
  tft.getTextBounds((char*)msg, 0, y, &x1, &y1, &w, &h);

  int16_t x = (tft.width() - (int16_t)w) / 2;
  if (x < 0) x = 0;

  tft.setTextColor(fg, bg);
  tft.setCursor(x, y);
  tft.print(msg);
}

void drawSmiley(int16_t cx, int16_t cy, int16_t r, uint16_t color, bool happy) {
  tft.drawCircle(cx, cy, r, color);

  int16_t eyeR = max(2, r / 8);
  int16_t eyeOffX = r / 3;
  int16_t eyeOffY = r / 4;
  tft.fillCircle(cx - eyeOffX, cy - eyeOffY, eyeR, color);
  tft.fillCircle(cx + eyeOffX, cy - eyeOffY, eyeR, color);

  int16_t mouthW = (r * 11) / 10;
  int16_t mouthY = cy + r / 4;
  int16_t leftX  = cx - mouthW / 2;
  int16_t rightX = cx + mouthW / 2;
  int16_t d = r / 5;

  if (happy) {
    tft.drawLine(leftX, mouthY, cx, mouthY + d, color);
    tft.drawLine(cx, mouthY + d, rightX, mouthY, color);
  } else {
    tft.drawLine(leftX, mouthY + d, cx, mouthY, color);
    tft.drawLine(cx, mouthY, rightX, mouthY + d, color);
  }
}

void drawArrowRight(int16_t x, int16_t y, int16_t length, int16_t height, uint16_t color) {
  // shaft
  tft.fillRect(x, y - height/6, length, height/3, color);

  // head
  int16_t tipX = x + length + height;
  tft.fillTriangle(
    x + length, y - height/2,
    x + length, y + height/2,
    tipX,       y,
    color
  );
}

// ---------- Screens ----------
void drawNormalScreen() {
  tft.fillScreen(NEUTRAL_BG);

  int16_t cx = tft.width() / 2;
  int16_t cy = tft.height() / 2 - 10;
  int16_t r  = min(tft.width(), tft.height()) / 5;

  drawSmiley(cx, cy, r, TEXT_IDLE, true);
  drawCenteredText("I have space!", tft.height() - 50, 2, TEXT_IDLE, NEUTRAL_BG);
}

void drawGreenScreen() {
  tft.fillScreen(SUCCESS_BG);

  int16_t cx = tft.width() / 2;
  int16_t cy = tft.height() / 2 - 20;
  int16_t r  = min(tft.width(), tft.height()) / 5;

  drawSmiley(cx, cy, r, TEXT_OK, true);
  drawCenteredText("thanks for keeping", tft.height() - 70, 2, TEXT_OK, SUCCESS_BG);
  drawCenteredText("this city clean!",   tft.height() - 45, 2, TEXT_OK, SUCCESS_BG);
}

void drawRedScreen() {
  tft.fillScreen(ILI9341_RED);

  int16_t cx = tft.width() / 2;
  int16_t cy = tft.height() / 2 - 25;
  int16_t r  = min(tft.width(), tft.height()) / 5;

  drawCenteredText("I am full!", 18, 3, ILI9341_WHITE, ILI9341_RED);
  drawSmiley(cx, cy, r, ILI9341_WHITE, false);

  // Arrow + guidance at bottom (text placed safely to the right)
  int16_t y = tft.height() - 55;

  int16_t arrowX      = 15;
  int16_t arrowLen    = 120;
  int16_t arrowHeight = 40;
  int16_t textX       = arrowX + arrowLen + arrowHeight + 10; // padding to avoid overlap

  drawArrowRight(arrowX, y, arrowLen, arrowHeight, ILI9341_WHITE);

  tft.setTextSize(2);
  tft.setTextColor(ILI9341_WHITE, ILI9341_RED);
  tft.setCursor(textX, tft.height() - 65);
  tft.print("next bin");
  tft.setCursor(textX, tft.height() - 40);
  tft.print("200m");
}

// ---------- State machine ----------
enum Mode { MODE_NORMAL, MODE_CHECKING, MODE_GREEN_HOLD, MODE_ALARM };
Mode mode = MODE_NORMAL;

unsigned long changeStartMs = 0;
unsigned long greenStartMs  = 0;
unsigned long alarmClearStartMs = 0;

Mode lastDrawnMode = (Mode)255; // force first draw

void setup() {
  Serial.begin(9600);

  // Ultrasonic setup
  pinMode(TRIG1, OUTPUT);
  pinMode(ECHO1, INPUT);
  pinMode(TRIG2, OUTPUT);
  pinMode(ECHO2, INPUT);
  digitalWrite(TRIG1, LOW);
  digitalWrite(TRIG2, LOW);

  // Buzzer setup
  pinMode(BUZZER_PIN, OUTPUT);
  noTone(BUZZER_PIN);

  // TFT setup
  tft.begin();
  tft.setRotation(1);

  // ----- Apply palette AFTER tft.begin() -----
  NEUTRAL_BG = tft.color565(10, 30, 80);   // navy idle
  SUCCESS_BG = tft.color565(30, 130, 30);  // green success
  TEXT_IDLE  = ILI9341_WHITE;              // white on navy
  TEXT_OK    = ILI9341_BLACK;              // black on green

  drawNormalScreen();
  lastDrawnMode = MODE_NORMAL;
}

void loop() {
  unsigned long now = millis();

  // Measure distances (used for logic, not displayed)
  float d1 = readDistanceCM(TRIG1, ECHO1);
  delay(50);
  float d2 = readDistanceCM(TRIG2, ECHO2);

  bool normalNow = isNormalDistance(d1) && isNormalDistance(d2);

  switch (mode) {
    case MODE_NORMAL:
      if (!normalNow) {
        mode = MODE_CHECKING;
        changeStartMs = now;
      }
      break;

    case MODE_CHECKING:
      if (normalNow) {
        // Returned to normal: was it a brief transient?
        if (now - changeStartMs < TRANSIENT_MAX_MS) {
          mode = MODE_GREEN_HOLD;
          greenStartMs = now;
          playGreenSound(); // play C-E-G when screen turns green
        } else {
          mode = MODE_NORMAL;
        }
      } else {
        // Still abnormal: if it lasts long enough, alarm
        if (now - changeStartMs >= TRANSIENT_MAX_MS) {
          mode = MODE_ALARM;
          alarmClearStartMs = 0;
        }
      }
      break;

    case MODE_GREEN_HOLD:
      if (now - greenStartMs >= GREEN_HOLD_MS) {
        mode = MODE_NORMAL;
      }
      break;

    case MODE_ALARM:
      // Red stays only while abnormal; clear after 2s continuously normal
      if (normalNow) {
        if (alarmClearStartMs == 0) alarmClearStartMs = now;
        if (now - alarmClearStartMs >= ALARM_CLEAR_MS) {
          mode = MODE_NORMAL;
          alarmClearStartMs = 0;
        }
      } else {
        alarmClearStartMs = 0;
      }
      break;
  }

  // Redraw only when mode changes (reduces flicker)
  if (mode != lastDrawnMode) {
    if (mode == MODE_NORMAL)          drawNormalScreen();
    else if (mode == MODE_GREEN_HOLD) drawGreenScreen();
    else if (mode == MODE_ALARM)      drawRedScreen();
    else                              drawNormalScreen(); // MODE_CHECKING

    lastDrawnMode = mode;
  }

  delay(150);
}
