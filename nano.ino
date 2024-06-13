
#include <Wire.h>
#include <ZMPT101B.h>
#include <DHT.h>

#define zmpt_Sensitivity 500.0f
#define DHTPIN 4
#define DHTTYPE DHT11

String dSend, dRec, df, suhu = "80.00", PLN, Batt1, Batt2;
String relay1, relay2;
float vac, Batt_A, Batt_B, Batt_C;
float logR2, R2s;

int Vo;
const int vAc = A0;

float reffVolt = 5.0;
const float R1 = 1000000.0;  // Nilai resistor R1 pada voltage divider (ohm)
const float R2 = 100000.0;

unsigned long prevMillisKerja = 0;
unsigned long intervalKerja = 900;


ZMPT101B voltageSensor1(vAc, 50.0);
DHT dht(DHTPIN, DHTTYPE);

void bacaSensor() {
  int val1 = analogRead(A1);
  float volt1 = (val1 * 5.0) / 1023.0;
  Batt_A = volt1 / (R2 / (R1 + R2));

  int val2 = analogRead(A2);
  float volt2 = (val2 * 5.0) / 1023.0;
  Batt_B = volt2 / (R2 / (R1 + R2));

  vac = voltageSensor1.getRmsVoltage();
  PLN = vac;

  float t = dht.readTemperature();  // Read temperature as Celsius
                                 

  suhu = String(t, 2);  // Convert float to String
  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.println(" *C");

  Batt1 = Batt_A;
  Batt2 = Batt_B;

  Batt1 = (Batt1.length() == 3) ? "00" + Batt1 : (Batt1.length() == 4) ? "0" + Batt1
                                                                       : Batt1;
  Batt2 = (Batt2.length() == 3) ? "00" + Batt2 : (Batt2.length() == 4) ? "0" + Batt2
                                                                       : Batt2;
  PLN = (PLN.length() == 4) ? "00" + PLN : (PLN.length() == 5) ? "0" + PLN
                                                               : PLN;


  Batt1 = Batt1.substring(0, 5);
  Batt2 = Batt2.substring(0, 5);
  suhu = suhu.substring(0, 5);

  dSend = PLN + "-" + Batt1 + "-" + Batt2 + "-" + suhu;
  Wire.beginTransmission(8);
  Wire.write(dSend.c_str());
  Wire.endTransmission();

  Serial.println(dSend);
}


// Terima dan bagi data status Saklar dari esp
void receiveEvent(int howMany) {
  df = "";
  while (0 < Wire.available()) {
    char c = Wire.read();  // receive byte as a character
    df = df + c;
  }
  df = df;

  Serial.print(F("data terima = "));
  Serial.println(df);
  relay1 = df.substring(1, 2);
  relay2 = df.substring(4, 5);

  Serial.println(relay1);
  Serial.println(relay2);
  if (relay1 == "1") {
    digitalWrite(5, HIGH);
  } else {
    digitalWrite(5, LOW);
  }

  if (relay2 == "1") {
    digitalWrite(6, HIGH);
  } else {
    digitalWrite(6, LOW);
  }
}


void requestEvent() {
  Wire.write(dSend.c_str());
  Serial.println(dSend.length());
}

void setup() {
  Wire.begin(0x09);             /* join i2c bus with address 8 */
  Wire.onReceive(receiveEvent); /* register receive event */
  Wire.onRequest(requestEvent); /* register request event */
  Serial.begin(115200);

  voltageSensor1.setSensitivity(zmpt_Sensitivity);
  dht.begin();
  pinMode(A0, INPUT);  //
  pinMode(A1, INPUT);  //
  pinMode(A2, INPUT);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
}

void loop() {
  unsigned long currMillisKerja = millis();
  if (currMillisKerja - prevMillisKerja >= intervalKerja) {
    bacaSensor();
    prevMillisKerja = currMillisKerja;
  }
}
