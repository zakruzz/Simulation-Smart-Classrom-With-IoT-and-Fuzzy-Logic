#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <DHT.h>
#include <MQ135.h>  // Menambahkan pustaka MQ135
#include <ESP32Servo.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

#define DHTPIN 17
#define DHTTYPE DHT22
#define MQ135_PIN 7  // Pin untuk sensor MQ135
#define FLAME_PIN 18
#define LDR_PIN 46
#define TRIG_PIN 3
#define ECHO_PIN 8
#define EXHAUST_PIN 4
#define COOLING_PIN 15
#define SERVO_PIN 9
#define LAMP_PIN 6
#define BUZZER_PIN 16
#define SOLENOID_PIN 5

#define THINGSBOARD_SERVER "demo.thingsboard.io"
#define THINGSBOARD_PORT 1883  // Port MQTT default untuk ThingsBoard

// Ganti dengan kredensial dari gambar
#define MQTT_CLIENT_ID "zaki123"          // Client ID
#define MQTT_USER_NAME "zaki123"         // Username
#define MQTT_PASSWORD "zaki"          // Password dari ThingsBoard

DHT dht(DHTPIN, DHTTYPE);
Servo myServo;
WiFiClient netThingsBoard;
PubSubClient clientThingsBoard(netThingsBoard);  // MQTT client untuk ThingsBoard
MQ135 gasSensor(MQ135_PIN);  // Sensor MQ135 untuk deteksi gas CO2

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 0, 3600000);  // UTC timezone, update setiap jam

float dhtTemp = 0, humidity = 0;
float co2 = 0;  // Sensor CO2 (dalam PPM)
int flameDetected = 0;
int ldrValue = 0;
float hcsrDistance = 0;

// Fuzzy Logic
String fuzzyStatus = "";

// Fungsi Fuzzy Logic untuk mengambil keputusan
void applyFuzzyLogic() {
  // Rule Base Logic menggunakan rentang suhu
  if (dhtTemp <= 20) {
    // Jika suhu <= 20°C, sistem dianggap belum siap
    digitalWrite(SOLENOID_PIN, LOW);
    digitalWrite(EXHAUST_PIN, LOW);
    digitalWrite(COOLING_PIN, LOW);
    myServo.write(0);
    digitalWrite(LAMP_PIN, LOW);
    digitalWrite(BUZZER_PIN, LOW);
    fuzzyStatus = "Kelas Belum Terbuka (Suhu Terlalu Dingin)";
  } else if (dhtTemp > 20 && dhtTemp <= 25) {
    // Jika suhu antara 20°C dan 25°C
    if (hcsrDistance > 15) {
      digitalWrite(SOLENOID_PIN, LOW);
      digitalWrite(EXHAUST_PIN, LOW);
      digitalWrite(COOLING_PIN, LOW);
      myServo.write(0);
      digitalWrite(LAMP_PIN, LOW);
      digitalWrite(BUZZER_PIN, LOW);
      fuzzyStatus = "Kelas Belum Terbuka (Jarak Terlalu Jauh)";
    } else {
      digitalWrite(SOLENOID_PIN, HIGH);
      digitalWrite(EXHAUST_PIN, LOW);
      digitalWrite(COOLING_PIN, LOW);
      myServo.write(0);
      digitalWrite(LAMP_PIN, LOW);
      digitalWrite(BUZZER_PIN, LOW);
      fuzzyStatus = "Kelas Akan Terbuka, Lampu Nyala";
    }
  } else if (dhtTemp > 25 && dhtTemp <= 30) {
    if (hcsrDistance > 15) {
      digitalWrite(SOLENOID_PIN, LOW);
      digitalWrite(EXHAUST_PIN, LOW);
      digitalWrite(COOLING_PIN, LOW);
      myServo.write(0);
      digitalWrite(LAMP_PIN, LOW);
      digitalWrite(BUZZER_PIN, LOW);
      fuzzyStatus = "Kelas Belum Terbuka (Jarak Terlalu Jauh)";
    } else {
      if (flameDetected == 0) {
        digitalWrite(SOLENOID_PIN, HIGH);
        digitalWrite(EXHAUST_PIN, LOW);
        digitalWrite(COOLING_PIN, HIGH);
        myServo.write(180);
        digitalWrite(LAMP_PIN, HIGH);
        digitalWrite(BUZZER_PIN, LOW);
        fuzzyStatus = "Kelas Terbuka, Pendinginan Dimulai";
      } else {
        digitalWrite(SOLENOID_PIN, HIGH);
        digitalWrite(EXHAUST_PIN, HIGH);
        digitalWrite(COOLING_PIN, HIGH);
        myServo.write(180);
        digitalWrite(LAMP_PIN, HIGH);
        digitalWrite(BUZZER_PIN, HIGH);
        fuzzyStatus = "KEBAKARAN!!";
      }
    }
  } else if (dhtTemp > 30) {
    if (hcsrDistance > 15) {
      digitalWrite(SOLENOID_PIN, LOW);
      digitalWrite(EXHAUST_PIN, LOW);
      digitalWrite(COOLING_PIN, LOW);
      myServo.write(0);
      digitalWrite(LAMP_PIN, LOW);
      digitalWrite(BUZZER_PIN, LOW);
      fuzzyStatus = "Kelas Belum Terbuka (Jarak Terlalu Jauh)";
    } else {
      if (flameDetected == 0) {
        digitalWrite(SOLENOID_PIN, HIGH);
        digitalWrite(EXHAUST_PIN, HIGH);
        digitalWrite(COOLING_PIN, HIGH);
        myServo.write(180);
        digitalWrite(LAMP_PIN, HIGH);
        digitalWrite(BUZZER_PIN, LOW);
        fuzzyStatus = "Kelas Terbuka, Pendinginan Diperlukan";
      } else {
        digitalWrite(SOLENOID_PIN, HIGH);
        digitalWrite(EXHAUST_PIN, HIGH);
        digitalWrite(COOLING_PIN, HIGH);
        myServo.write(180);
        digitalWrite(LAMP_PIN, HIGH);
        digitalWrite(BUZZER_PIN, HIGH);
        fuzzyStatus = "KEBAKARAN!!";
      }
    }
  }
}

// Fungsi untuk mendapatkan timestamp waktu Indonesia (WIB)
String getTimestamp() {
  timeClient.update();
  unsigned long epochTime = timeClient.getEpochTime();
  epochTime += 7 * 3600;  // Tambah 7 jam untuk WIB
  return String(epochTime);
}

// Fungsi untuk mengirim data ke ThingsBoard
void publishToThingsBoard() {
  StaticJsonDocument<1024> doc;
  doc["status"] = fuzzyStatus;
  doc["dhtTemp"] = dhtTemp;
  doc["co2"] = co2;  // Kirim nilai CO2 dalam PPM
  doc["ldr"] = (ldrValue == 0) ? 1 : 0;  // Terbalikkan logika LDR
  doc["hcsr"] = hcsrDistance;
  doc["flame"] = (flameDetected == 0) ? 1 : 0;  // Terbalikkan logika Flame sensor
  doc["timestamp"] = getTimestamp();  // Tambahkan timestamp ke payload

  char jsonBuffer[1024];
  serializeJson(doc, jsonBuffer);

  if (clientThingsBoard.publish("v1/devices/me/telemetry", jsonBuffer)) {
    Serial.println("Data published to ThingsBoard");
  } else {
    Serial.println("Failed to publish data to ThingsBoard");
  }
}

// Fungsi untuk menghubungkan ke ThingsBoard
void connectThingsBoard() {
  while (!clientThingsBoard.connected()) {
    Serial.print("Connecting to ThingsBoard...");
    if (clientThingsBoard.connect(MQTT_CLIENT_ID, MQTT_USER_NAME, MQTT_PASSWORD)) {
      Serial.println("Connected to ThingsBoard!");
    } else {
      Serial.print("Failed, rc=");
      Serial.print(clientThingsBoard.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  WiFi.begin("zaki", "ojokdibagi");  // Ganti dengan nama Wi-Fi dan kata sandi Anda
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Wi-Fi connected!");

  clientThingsBoard.setServer(THINGSBOARD_SERVER, THINGSBOARD_PORT);
  connectThingsBoard();

  dht.begin();
  myServo.attach(SERVO_PIN);
  pinMode(LAMP_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(SOLENOID_PIN, OUTPUT);
  pinMode(COOLING_PIN, OUTPUT);
  pinMode(EXHAUST_PIN, OUTPUT);
  pinMode(FLAME_PIN, INPUT);
  pinMode(LDR_PIN, INPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  timeClient.begin();
}

void loop() {
  dhtTemp = dht.readTemperature();
  humidity = dht.readHumidity();
  co2 = gasSensor.getPPM();
  ldrValue = digitalRead(LDR_PIN);
  flameDetected = digitalRead(FLAME_PIN);

  long duration;
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  duration = pulseIn(ECHO_PIN, HIGH);
  hcsrDistance = duration * 0.034 / 2;

  applyFuzzyLogic();
  publishToThingsBoard();
  delay(3000);
}
