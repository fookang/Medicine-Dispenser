const int NEW_TX_PIN = 17;
const int NEW_RX_PIN = 18;
const int SENSOR_PIN = 2;   // ADC-capable pin on ESP32

void setup() {
  Serial.begin(9600);
  Serial1.begin(9600, SERIAL_8N1, NEW_RX_PIN, NEW_TX_PIN);
  delay(1000);
  Serial.println("UART DEMO");

  pinMode(SENSOR_PIN, INPUT);
}

void loop() {
  int sensorValue = analogRead(SENSOR_PIN);
  Serial.print("Analog value: ");
  Serial.println(sensorValue);

  Serial1.println(sensorValue);

  delay(10);
}
