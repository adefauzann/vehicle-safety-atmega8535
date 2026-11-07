#include <mega32a.h>
#include <alcd.h>
#include <delay.h>
#include <stdio.h>

// Define pin untuk ultrasonic sensor
#define TRIG PORTB.6
#define ECHO PINB.7

// Define pin untuk DS18B20
#define DS18B20_PIN PORTB.5

// Define pin untuk LED indikator
#define LED1 PORTD.0 // Jarak > 30 cm
#define LED2 PORTD.1 // Jarak 20-30 cm
#define LED3 PORTD.2 // Jarak < 20 cm

// Buffer untuk menyimpan data jarak dan suhu dalam bentuk string
char buffer[16];

// Konfigurasi ulang pin LCD
#define LCD_RW PORTC.1
#define LCD_RS PORTC.0
#define LCD_E  PORTC.2
#define LCD_D4 PORTC.4
#define LCD_D5 PORTC.5
#define LCD_D6 PORTC.6
#define LCD_D7 PORTC.7

// Fungsi untuk mengukur jarak menggunakan HC-SR04
unsigned int get_distance() {
    unsigned int count;
    unsigned int distance;

    // Mengirimkan pulse trigger
    TRIG = 1;
    delay_us(10);
    TRIG = 0;

    // Tunggu sampai ECHO naik
    while (!ECHO);

    // Hitung durasi HIGH pada ECHO
    count = 0;
    while (ECHO && count < 30000) {
        delay_us(1);
        count++;
    }

    // Konversi durasi ke jarak (dalam cm)
    distance = count / 58; // 1 us ~ 0.0343 cm, jadi 58 us ~ 1 cm

    // Batasi jarak maksimal
    if (distance > 400) distance = 400;

    return distance;
}

// Fungsi untuk reset 1-Wire
void ow_reset() {
    DS18B20_PIN = 0;  // Tarik pin ke LOW
    delay_us(480);    // Tunggu selama 480us
    DS18B20_PIN = 1;  // Lepaskan pin ke HIGH
    delay_us(60);     // Tunggu selama 60us
    if (DS18B20_PIN == 0) {  // Cek apakah ada respon
        delay_us(420);    // Tunggu selama 420us
    }
}

// Fungsi untuk menulis data 1 byte ke DS18B20
void ow_write(unsigned char data) {
    unsigned char i;

    for (i = 0; i < 8; i++) {
        DS18B20_PIN = 0;          // Tarik pin ke LOW
        delay_us(1);              // Tunggu selama 1us
        if (data & (1 << i)) {
            DS18B20_PIN = 1;      // Kirim 1
        }
        delay_us(60);             // Tunggu 60us untuk memastikan data terkirim
        DS18B20_PIN = 1;          // Pulihkan pin ke HIGH
        delay_us(1);              // Tunggu sebelum bit berikutnya
    }
}

// Fungsi untuk membaca data 1 byte dari DS18B20
unsigned char ow_read() {
    unsigned char i, value = 0;

    for (i = 0; i < 8; i++) {
        DS18B20_PIN = 0;          // Tarik pin ke LOW
        delay_us(2);              // Tunggu selama 2us
        DS18B20_PIN = 1;          // Lepaskan pin ke HIGH
        delay_us(10);             // Tunggu selama 10us untuk baca data
        if (DS18B20_PIN) {
            value |= (1 << i);    // Jika pin HIGH, set bit ke-`i`
        }
        delay_us(50);             // Tunggu sebelum bit berikutnya
    }

    return value;
}

// Fungsi untuk membaca suhu dari DS18B20
float get_temperature() {
    unsigned char temp_LSB, temp_MSB;
    int raw_temperature;
    float temperature;

    // Reset 1-Wire dan mulai konversi suhu
    ow_reset();
    ow_write(0xCC); // Skip ROM
    ow_write(0x44); // Start temperature conversion
    delay_ms(750);  // Tunggu konversi selesai

    // Baca suhu dari scratchpad
    ow_reset();
    ow_write(0xCC); // Skip ROM
    ow_write(0xBE); // Read scratchpad
    temp_LSB = ow_read();
    temp_MSB = ow_read();

    // Hitung suhu
    raw_temperature = (temp_MSB << 8) | temp_LSB;
    temperature = raw_temperature * 0.0625; // Resolusi DS18B20 adalah 0.0625°C

    return temperature;
}

// Inisialisasi ulang fungsi LCD agar sesuai dengan konfigurasi pin
void lcd_init_custom() {
    // Konfigurasi port LCD sebagai output
    DDRC |= 0b11100111; // PC0 (RS), PC1 (RW), PC2 (E) sebagai output
    PORTC &= ~(1 << 1); // Pastikan RW diatur ke LOW (Write Mode)

    // Inisialisasi LCD
    lcd_init(16);
}

void main() {
    unsigned int distance;
    float temperature;

    // Inisialisasi port
    DDRB = 0b01000100;  // PB6 sebagai output (TRIG), PB7 sebagai input (ECHO), PB5 sebagai output untuk DS18B20
    PORTB = 0b00000000; // Default LOW
    DDRD |= 0b00000111; // PD0-PD2 sebagai output untuk LED

    // Inisialisasi LCD dengan konfigurasi ulang
    lcd_init_custom();
    lcd_clear();

    while (1) {
        // Baca jarak
        distance = get_distance();

        // Baca suhu
        temperature = get_temperature();

        // Konversi data jarak ke string
        sprintf(buffer, "Dist: %ucm", distance);
        lcd_clear();
        lcd_gotoxy(0, 0);
        lcd_puts(buffer);

        // Konversi data suhu ke string
        sprintf(buffer, "Temp: %.1fC", temperature);
        lcd_gotoxy(0, 1);
        lcd_puts(buffer);

        // Logika untuk mengatur LED berdasarkan jarak
        if (distance > 30) {
            LED1 = 1; // LED untuk jarak > 30 cm menyala
            LED2 = 0;
            LED3 = 0;
        } else if (distance >= 20 && distance <= 30) {
            LED1 = 0;
            LED2 = 1; // LED untuk jarak 20-30 cm menyala
            LED3 = 0;
        } else {
            LED1 = 0;
            LED2 = 0;
            LED3 = 1; // LED untuk jarak < 20 cm menyala
        }

        // Logika untuk LED berkedip jika suhu >= 70°C
        if (temperature >= 70.0) {
            LED1 = 1;
            LED2 = 1;
            LED3 = 1;
            delay_ms(500);
            LED1 = 0;
            LED2 = 0;
            LED3 = 0;
            delay_ms(500);
        }

        // Tunggu 500ms sebelum pembacaan berikutnya
        delay_ms(500);
    }
}
