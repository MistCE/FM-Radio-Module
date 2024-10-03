#include <TEA5767N.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

TEA5767N radio = TEA5767N();

int analogPin = 0;
int val = 0;
int frequencyInt = 0;
float frequency = 0;
float previousFrequency = 0;
int signalStrength = 0;

void setup() {
  radio.setMonoReception();
  radio.setStereoNoiseCancellingOn();

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  display.display();
  delay(2000);
  display.clearDisplay();
}

void loop() {
  val = 0;
  for (int i = 0; i < 30; i++) {
    val += analogRead(analogPin);
    delay(1);
  }

  val /= 30;
  frequencyInt = map(val, 0, 1023, 8750, 10800); //Analog value to frequency from 87.5 MHz to 108.0 MHz
  frequency = frequencyInt / 100.0f;

  if (abs(frequency - previousFrequency) >= 0.1f) {
    display.clearDisplay();
    radio.selectFrequency(frequency);
    printSignalStrength();
    printStereo();
    printFrequency(frequency);
    previousFrequency = frequency;
    display.display();
  }

  delay(50);
}

void printFrequency(float frequency) {
  display.setTextSize(2); // Size for the big numbers
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(14, 12); // Position for the frequency
  display.println(frequency, 1); // Print frequency with one decimal place
}

void printStereo() {
  boolean isStereo = radio.isStereo();
  if (isStereo) {
    display.setTextSize(1); // Size for the small text
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(55, 2); // Position for the stereo indicator
    display.println("STEREO");
  }
}

void printSignalStrength() {
  signalStrength = radio.getSignalLevel();
  if (signalStrength >= 15) {
    drawExternalBitmap("signal5.bmp", 1, 1);
  } else if (signalStrength >= 11) {
    drawExternalBitmap("signal4.bmp", 1, 1);
  } else if (signalStrength >= 9) {
    drawExternalBitmap("signal3.bmp", 1, 1);
  } else if (signalStrength >= 7) {
    drawExternalBitmap("signal2.bmp", 1, 1);
  } else {
    drawExternalBitmap("signal1.bmp", 1, 1);
  }
}

void drawExternalBitmap(const char* filename, int16_t x, int16_t y) {
  File bmpFile;
  
  bmpFile = SPIFFS.open(filename, "r");
  if (!bmpFile) {
    Serial.println("File not found");
    return;
  }

  int bmpWidth = bmpFile.read();
  int bmpHeight = bmpFile.read();

  uint8_t bmpBuffer[bmpWidth * bmpHeight / 8];

  bmpFile.read(bmpBuffer, sizeof(bmpBuffer));

  bmpFile.close();

  display.drawBitmap(x, y, bmpBuffer, bmpWidth, bmpHeight, SSD1306_WHITE);
}
