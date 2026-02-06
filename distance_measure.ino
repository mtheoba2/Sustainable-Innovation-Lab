#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>

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

  long duration = pulseIn(echoPin, HIGH, 30000);
  if (duration == 0) return -1;
  return duration * 0.0343 / 2;
}

void setup() {
  // --- Serial (optional) ---
  Serial.begin(9600);

  // --- Ultrasonic ---
  pinMode(TRIG1, OUTPUT);
  pinMode(ECHO1, INPUT);
  pinMode(TRIG2, OUTPUT);
  pinMode(ECHO2, INPUT);
  digitalWrite(TRIG1, LOW);
  digitalWrite(TRIG2, LOW);

  // --- TFT ---
  tft.begin();
  tft.setRotation(1);
  tft.fillScreen(ILI9341_BLACK);

  tft.setTextSize(2);
  tft.setTextColor(ILI9341_WHITE);
}

void loop() {
  float d1 = readDistanceCM(TRIG1, ECHO1);
  delay(50);   // prevent crosstalk
  float d2 = readDistanceCM(TRIG2, ECHO2);

  // --- Display ---
  tft.fillRect(0, 0, tft.width(), 80, ILI9341_BLACK);

  tft.setCursor(10, 10);
  tft.print("Sensor 1: ");
  if (d1 < 0) tft.print("----");
  else {
    tft.print(d1, 1);
    tft.print(" cm");
  }

  tft.setCursor(10, 40);
  tft.print("Sensor 2: ");
  if (d2 < 0) tft.print("----");
  else {
    tft.print(d2, 1);
    tft.print(" cm");
  }

  delay(300);
}
