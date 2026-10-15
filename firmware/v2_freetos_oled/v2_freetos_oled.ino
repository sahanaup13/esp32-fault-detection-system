#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

const int V_PIN = 34, I_PIN = 35, T_PIN = 32;
const int BUZZ = 25, LED = 26, RELAY = 27, BTN = 14;

const float V_MAX = 12.0, V_MIN = 4.5, I_MAX = 3.5, T_MAX = 60.0;
const float BETA = 3950;

Adafruit_SSD1306 oled(128, 64, &Wire, -1);

struct Data { float v, i, t; const char* fault; bool latched; };
Data d = {0, 0, 25, "NONE", false};
SemaphoreHandle_t mtx;

float readTemp() {
  int a = analogRead(T_PIN);
  return 1 / (log(1 / (4095. / a - 1)) / BETA + 1.0 / 298.15) - 273.15;
}

// Task 1: read sensors
void sensorTask(void*) {
  for (;;) {
    float v = analogRead(V_PIN) * 15.0 / 4095.0;
    float i = analogRead(I_PIN) * 5.0 / 4095.0;
    float t = readTemp();
    xSemaphoreTake(mtx, portMAX_DELAY);
    d.v = v; d.i = i; d.t = t;
    xSemaphoreGive(mtx);
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

// Task 2: protection logic + reset button
void protectionTask(void*) {
  for (;;) {
    if (millis() > 2000) {
      xSemaphoreTake(mtx, portMAX_DELAY);
      if (digitalRead(BTN) == LOW && d.latched) {
        d.latched = false;
        digitalWrite(RELAY, HIGH);
        digitalWrite(LED, LOW);
        digitalWrite(BUZZ, LOW);
      }
      const char* f = "NONE";
      if (d.v > V_MAX) f = "OVERVOLTAGE";
      else if (d.v < V_MIN) f = "UNDERVOLTAGE";
      else if (d.i > I_MAX) f = "OVERCURRENT";
      else if (d.t > T_MAX) f = "OVERHEAT";
      d.fault = f;
      if (f[0] != 'N' && !d.latched) {
        d.latched = true;
        digitalWrite(RELAY, LOW);
        digitalWrite(LED, HIGH);
        digitalWrite(BUZZ, HIGH);
      }
      xSemaphoreGive(mtx);
    }
    vTaskDelay(pdMS_TO_TICKS(50));
  }
}

// Task 3: OLED display
void displayTask(void*) {
  for (;;) {
    xSemaphoreTake(mtx, portMAX_DELAY);
    Data c = d;
    xSemaphoreGive(mtx);
    oled.clearDisplay();
    oled.setTextSize(1);
    oled.setTextColor(SSD1306_WHITE);
    oled.setCursor(0, 0);  oled.printf("V: %.2f V", c.v);
    oled.setCursor(0, 12); oled.printf("I: %.2f A", c.i);
    oled.setCursor(0, 24); oled.printf("T: %.1f C", c.t);
    oled.setCursor(0, 40); oled.printf("Fault: %s", c.fault);
    oled.setCursor(0, 52); oled.print(c.latched ? "LOAD CUT OFF" : "LOAD ON");
    oled.display();
    vTaskDelay(pdMS_TO_TICKS(250));
  }
}

// Task 4: serial CSV log (on change + every 2 s)
void logTask(void*) {
  const char* last = "";
  unsigned long lastPrint = 0;
  for (;;) {
    xSemaphoreTake(mtx, portMAX_DELAY);
    Data c = d;
    xSemaphoreGive(mtx);
    if (strcmp(c.fault, last) != 0 || millis() - lastPrint > 2000) {
      Serial.printf("%lu,%.2f,%.2f,%.1f,%s\n", millis(), c.v, c.i, c.t, c.fault);
      last = c.fault;
      lastPrint = millis();
    }
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(BUZZ, OUTPUT); pinMode(LED, OUTPUT); pinMode(RELAY, OUTPUT);
  pinMode(BTN, INPUT_PULLUP);
  digitalWrite(RELAY, HIGH);
  oled.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  mtx = xSemaphoreCreateMutex();
  xTaskCreate(sensorTask, "sensor", 4096, NULL, 2, NULL);
  xTaskCreate(protectionTask, "protect", 4096, NULL, 3, NULL);
  xTaskCreate(displayTask, "display", 4096, NULL, 1, NULL);
  xTaskCreate(logTask, "log", 4096, NULL, 1, NULL);
}

void loop() { vTaskDelay(portMAX_DELAY); }
