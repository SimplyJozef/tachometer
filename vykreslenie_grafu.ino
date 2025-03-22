#include <TFT_eSPI.h>

TFT_eSPI tft = TFT_eSPI();

const int graphWidth = 160;
const int graphHeight = 33;
const int barWidth = 1;

int barHeights1[graphWidth];
int barHeights2[graphWidth];
int barHeights3[graphWidth];

uint16_t buffer1[graphWidth * graphHeight];
uint16_t buffer2[graphWidth * graphHeight];
uint16_t buffer3[graphWidth * graphHeight];

void setup() {
  Serial.begin(115200);
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_WHITE);

  for (int i = 0; i < graphWidth; i++) {
    barHeights1[i] = 0;
    barHeights2[i] = 0;
    barHeights3[i] = 0;
  }

  for (int i = 0; i < graphWidth * graphHeight; i++) {
    buffer1[i] = TFT_WHITE;
    buffer2[i] = TFT_WHITE;
    buffer3[i] = TFT_WHITE;
  }
}

void loop() {
  for (int value = 0; value <= 1023; value += 10) {
    updateAndDrawGraph(value, 0, barHeights1, buffer1);
    updateAndDrawGraph(value, 40, barHeights2, buffer2);
    updateAndDrawGraph(value, 80, barHeights3, buffer3);
    delay(20);
  }
}

void updateAndDrawGraph(int value, int yOffset, int barHeights[], uint16_t buffer[]) {
  for (int i = 0; i < graphWidth - 1; i++) {
    barHeights[i] = barHeights[i + 1];
  }

  int newBarHeight = map(value, 0, 1023, 0, graphHeight);
  barHeights[graphWidth - 1] = newBarHeight;

  for (int i = 0; i < graphWidth * graphHeight; i++) {
    buffer[i] = TFT_WHITE;
  }

  for (int x = 0; x < graphWidth; x++) {
    int barHeight = barHeights[x];
    if (barHeight > 0) {
      for (int y = graphHeight - barHeight; y < graphHeight; y++) {
        buffer[y * graphWidth + x] = TFT_BLACK;
      }
    }
  }

  tft.pushImage(0, yOffset, graphWidth, graphHeight, buffer);
}
