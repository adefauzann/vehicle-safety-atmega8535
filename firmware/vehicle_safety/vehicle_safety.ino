// Program untuk ATmega8535 menggunakan MightyCore
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Inisialisasi LCD I2C
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Pin definisi
const int trigPin = 10;        // Pin Trig untuk sensor ultrasonik
const int echoPin = 11;        // Pin Echo untuk sensor ultrasonik
const int lm35Pin = A1;        // Pin untuk sensor LM35
const int buzzerPin = 8;       // Pin untuk buzzer
const int ledMerah = 3;        // LED merah
const int ledKuning = 4;       // LED kuning
const int ledHijau = 5;        // LED hijau

// Ambang batas
const float ambangSuhu = 50.0; // Ambang suhu dalam derajat Celsius
const int jarakDekat = 10;     // Ambang jarak dekat dalam cm
const int jarakSedang = 20;    // Ambang jarak sedang dalam cm

void setup() {
  // Inisialisasi LCD
  lcd.begin();
  lcd.backlight();

  // Inisialisasi pin
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(lm35Pin, INPUT);
  pinMode(buzzerPin, OUTPUT);
  pinMode(ledMerah, OUTPUT);
  pinMode(ledKuning, OUTPUT);
  pinMode(ledHijau, OUTPUT);

  // Buzzer dan LED off saat awal
  digitalWrite(buzzerPin, LOW);
  digitalWrite(ledMerah, LOW);
  digitalWrite(ledKuning, LOW);
  digitalWrite(ledHijau, LOW);

  // Tampilkan pesan awal di LCD
  lcd.setCursor(0, 0);
  lcd.print("Monitoring J&S");
  lcd.setCursor(0, 1);
  lcd.print("  ADE FAUZAN N   ");
  delay(2000);
}

void loop() {
  // Mengukur jarak dengan sensor ultrasonik
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long durasi = pulseIn(echoPin, HIGH);
  int jarak = durasi * 0.034 / 2; // Konversi ke cm

  // Baca data dari LM35
  int nilaiLm35 = analogRead(lm35Pin);
  float tegangan = (nilaiLm35 / 1024.0) * 5.0; // Tegangan dalam volt
  float suhu = tegangan * 100.0;              // Suhu dalam derajat Celsius

  // Tampilkan data di LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Jarak: ");
  lcd.print(jarak);
  lcd.print(" cm");
  lcd.setCursor(0, 1);
  lcd.print("Suhu: ");
  lcd.print(suhu, 1);
  lcd.print(" C");

  // Cek kondisi jarak dan suhu
  
  if (jarak <= jarakDekat ) {
    // Aktifkan buzzer
    digitalWrite(buzzerPin, HIGH);
    digitalWrite(ledMerah, HIGH);
    digitalWrite(ledKuning, LOW);
    digitalWrite(ledHijau, LOW);
  } 
 else if (suhu > ambangSuhu) {
    // Aktifkan buzzer
    digitalWrite(buzzerPin, HIGH);
    digitalWrite(ledMerah, HIGH);
    digitalWrite(ledKuning, HIGH);
    digitalWrite(ledHijau, HIGH);
  }
  
  else if (jarak <= jarakSedang) {
    // Aktifkan LED kuning
    digitalWrite(buzzerPin, LOW);
    digitalWrite(ledMerah, LOW);
    digitalWrite(ledKuning, HIGH);
    digitalWrite(ledHijau, LOW);
  } else {
    // Aktifkan LED hijau
    digitalWrite(buzzerPin, LOW);
    digitalWrite(ledMerah, LOW);
    digitalWrite(ledKuning, LOW);
    digitalWrite(ledHijau, HIGH);
  }

  delay(500); // Delay untuk pembacaan data berikutnya
}
