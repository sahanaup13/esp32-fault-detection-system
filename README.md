const int V_PIN = 34, I_PIN = 35, T_PIN = 32;
const int BUZZ = 25, LED = 26, RELAY = 27;

const float V_MAX = 12.0, V_MIN = 4.5; // volts
const float I_MAX = 3.5;               // amps
const float T_MAX = 60.0;              // deg C
const float BETA = 3950;

bool faultLatched = false;

float readTemp() {
  int a = analogRead(T_PIN);
  return 1 / (log(1 / (4095. / a - 1)) / BETA + 1.0 / 298.15) - 273.15;
}

void setup() {
  Serial.begin(115200);
  pinMode(BUZZ, OUTPUT); pinMode(LED, OUTPUT); pinMode(RELAY, OUTPUT);
  digitalWrite(RELAY, HIGH); // load ON
}

void loop() {
  float v = analogRead(V_PIN) * 15.0 / 4095.0; // scale to 0-15 V
  float i = analogRead(I_PIN) * 5.0 / 4095.0;  // scale to 0-5 A
  float t = readTemp();

  const char* fault = "NONE";
  if (v > V_MAX) fault = "OVERVOLTAGE";
  else if (v < V_MIN) fault = "UNDERVOLTAGE";
  else if (i > I_MAX) fault = "OVERCURRENT";
  else if (t > T_MAX) fault = "OVERHEAT";

  if (fault[0] != 'N' && !faultLatched) {
    faultLatched = true;
    digitalWrite(RELAY, LOW);   // cut load
    digitalWrite(LED, HIGH);
    digitalWrite(BUZZ, HIGH);
  }

  // CSV log: time_ms,V,I,T,fault
  Serial.printf("%lu,%.2f,%.2f,%.1f,%s\n", millis(), v, i, t, fault);
  delay(100);
}
