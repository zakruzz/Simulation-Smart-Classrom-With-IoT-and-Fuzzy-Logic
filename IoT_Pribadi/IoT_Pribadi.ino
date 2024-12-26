#include "secret.h"
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "WiFi.h"
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

#define AWS_IOT_PUBLISH_TOPIC "esp32zaki/pub"
#define AWS_IOT_SUBSCRIBE_TOPIC "esp32zaki/sub"

DHT dht(DHTPIN, DHTTYPE);
Servo myServo;
WiFiClientSecure net;
PubSubClient client(net);
// Sensor MQ135 untuk deteksi gas CO2
MQ135 gasSensor(MQ135_PIN);  // Pastikan pin yang benar untuk MQ135

// NTP client setup
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 0, 3600000);  // UTC timezone, update every hour

float dhtTemp = 0, humidity = 0;
float co2 = 0;  // Sensor CO2 (dalam PPM)
int flameDetected = 0;
int ldrValue = 0;
float hcsrDistance = 0;

// Fuzzy Logic
String fuzzyStatus = "";

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
  } 
  else if (dhtTemp > 20 && dhtTemp <= 25) {
    // Jika suhu antara 20°C dan 25°C
    if (hcsrDistance > 15) {
      // Jika jarak lebih dari 15cm, semuanya dimatikan
      digitalWrite(SOLENOID_PIN, LOW);
      digitalWrite(EXHAUST_PIN, LOW);
      digitalWrite(COOLING_PIN, LOW);
      myServo.write(0);
      digitalWrite(LAMP_PIN, LOW);
      digitalWrite(BUZZER_PIN, LOW);
      fuzzyStatus = "Kelas Belum Terbuka (Jarak Terlalu Jauh)";
    } else {
      // Jika jarak antara 5 dan 7 cm
      digitalWrite(SOLENOID_PIN, HIGH);
      digitalWrite(EXHAUST_PIN, LOW);
      digitalWrite(COOLING_PIN, LOW);
      myServo.write(0);
      digitalWrite(LAMP_PIN, HIGH);
      digitalWrite(BUZZER_PIN, LOW);
      fuzzyStatus = "Kelas Akan Terbuka, Lampu Nyala";
    }
  } 
  else if (dhtTemp > 25 && dhtTemp <= 30) {
    // Jika suhu antara 25°C dan 30°C
    if (hcsrDistance > 15) {
      // Jika jarak lebih dari 15cm, semuanya dimatikan
      digitalWrite(SOLENOID_PIN, LOW);
      digitalWrite(EXHAUST_PIN, LOW);
      digitalWrite(COOLING_PIN, LOW);
      myServo.write(0);
      digitalWrite(LAMP_PIN, LOW);
      digitalWrite(BUZZER_PIN, LOW);
      fuzzyStatus = "Kelas Belum Terbuka (Jarak Terlalu Jauh)";
    } else {
      // Jika jarak antara 5 dan 7 cm dan tidak ada api
      if (flameDetected == 0) {
        digitalWrite(SOLENOID_PIN, HIGH);
        digitalWrite(EXHAUST_PIN, LOW);
        digitalWrite(COOLING_PIN, HIGH);
        myServo.write(180);
        digitalWrite(LAMP_PIN, HIGH);
        digitalWrite(BUZZER_PIN, LOW);
        fuzzyStatus = "Kelas Terbuka, Pendinginan Dimulai";
      } else {
        // Jika ada api, aktifkan semua untuk memadamkan kebakaran
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
  else if (dhtTemp > 30) {
    // Jika suhu > 30°C, kondisi lebih ekstrem
    if (hcsrDistance > 15) {
      // Jika jarak lebih dari 15cm, semuanya dimatikan
      digitalWrite(SOLENOID_PIN, LOW);
      digitalWrite(EXHAUST_PIN, LOW);
      digitalWrite(COOLING_PIN, LOW);
      myServo.write(0);
      digitalWrite(LAMP_PIN, LOW);
      digitalWrite(BUZZER_PIN, LOW);
      fuzzyStatus = "Kelas Belum Terbuka (Jarak Terlalu Jauh)";
    } else {
      // Jika jarak antara 5 dan 7 cm dan tidak ada api
      if (flameDetected == 0) {
        digitalWrite(SOLENOID_PIN, HIGH);
        digitalWrite(EXHAUST_PIN, HIGH);
        digitalWrite(COOLING_PIN, HIGH);
        myServo.write(180);
        digitalWrite(LAMP_PIN, HIGH);
        digitalWrite(BUZZER_PIN, LOW);
        fuzzyStatus = "Kelas Terbuka, Pendinginan Diperlukan";
      } else {
        // Jika ada api, aktifkan semua untuk memadamkan kebakaran
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
  
  // Dapatkan waktu UTC dalam detik
  unsigned long epochTime = timeClient.getEpochTime();

  // Konversi waktu UTC ke waktu Indonesia (WIB, UTC +7)
  epochTime += 7 * 3600;  // Menambahkan 7 jam ke UTC untuk WIB

  // Format waktu Indonesia (YYYY-MM-DD HH:MM:SS)
  String timestamp = "";
  int year = (epochTime / 31536000) + 1970;  // Tahun
  epochTime %= 31536000;
  int month = (epochTime / 2592000) + 1;  // Bulan
  epochTime %= 2592000;
  int day = (epochTime / 86400);  // Hari
  epochTime %= 86400;
  int hour = (epochTime / 3600);  // Jam
  epochTime %= 3600;
  int minute = (epochTime / 60);  // Menit
  int second = epochTime % 60;    // Detik

  // Format menjadi string
  timestamp += String(year) + "-";
  if (month < 10) timestamp += "0";
  timestamp += String(month) + "-";
  if (day < 10) timestamp += "0";
  timestamp += String(day) + " ";
  if (hour < 10) timestamp += "0";
  timestamp += String(hour) + ":";
  if (minute < 10) timestamp += "0";
  timestamp += String(minute) + ":";
  if (second < 10) timestamp += "0";
  timestamp += String(second);

  return timestamp;
}

void publishMessage() {
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

  if (client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer)) {
    Serial.println("Data published to AWS IoT");
  } else {
    Serial.println("Failed to publish data to AWS IoT");
    Serial.println("Error: " + String(client.state())); // Menampilkan status error
  }
}

void connectAWS() {
  // Connecting to Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print("...");
  }

  Serial.println("Connected to Wi-Fi!");

  // Setting up secure connection to AWS IoT
  net.setCACert(AWS_CERT_CA);
  net.setCertificate(AWS_CERT_CRT);
  net.setPrivateKey(AWS_CERT_PRIVATE);

  // Setting up MQTT Client
  client.setServer(AWS_IOT_ENDPOINT, 8883);

  // Connect to AWS IoT
  while (!client.connect(THINGNAME)) {
    delay(1000);
    Serial.print("...");
  }
  Serial.println("AWS IoT connected!");
  
  // Subscribe to topic (optional, for receiving messages)
  client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC);
}

void setup() {
  Serial.begin(115200);
  connectAWS();

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

  myServo.write(0);

  timeClient.begin();  // Mulai NTP client
}

void loop() {
  dhtTemp = dht.readTemperature();
  humidity = dht.readHumidity();
  co2 = gasSensor.getPPM();  // Nilai CO2 dalam PPM
  ldrValue = digitalRead(LDR_PIN);  // Baca nilai LDR (logika terbalik)
  flameDetected = digitalRead(FLAME_PIN);  // Baca nilai flame (logika terbalik)

  // Menghitung jarak dari sensor ultrasonic
  long duration;
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  duration = pulseIn(ECHO_PIN, HIGH);
  hcsrDistance = duration * 0.034 / 2;

  applyFuzzyLogic();
  publishMessage();
  delay(3000);
}
