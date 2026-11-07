#include <mega8535.h>
#include <delay.h>

#define TRIG_PIN 6    // PB6
#define ECHO_PIN 7    // PB7
#define LED1 0        // PA0 (Far distance)
#define LED2 1        // PA1 (Medium distance)
#define LED3 2        // PA2 (Near distance)
#define BUZZER 3      // PA3 (Warning buzzer)

void sendTriggerPulse() {
    PORTB |= (1 << TRIG_PIN);      // Set TRIG pin HIGH
    delay_us(10);                  // Pulse width 10 microseconds
    PORTB &= ~(1 << TRIG_PIN);     // Set TRIG pin LOW
}

unsigned int readDistance() {
    unsigned long duration;
    unsigned int distance;
    
    sendTriggerPulse();
    
    // Wait for ECHO pin to go HIGH (start of echo)
    while (!(PINB & (1 << ECHO_PIN)));  // Wait for HIGH
    
    // Start Timer1
    TCNT1 = 0;                       // Reset Timer1
    TCCR1B = (1 << CS11);            // Prescaler 8
    
    // Wait for ECHO pin to go LOW (end of echo) with timeout
    while ((PINB & (1 << ECHO_PIN)) && (TCNT1 < 65535));
    
    duration = TCNT1;                // Get timer value
    
    // Stop Timer1
    TCCR1B = 0;                      // Stop the timer
    
    // Convert duration to distance (in cm)
    distance = (duration * 8) / 58;  // Adjust for prescaler 8
    
    return distance;
}

void updateIndicators(unsigned int distance) {
    PORTA &= ~((1 << LED1) | (1 << LED2) | (1 << LED3) | (1 << BUZZER)); // Clear all indicators
    
    if (distance > 30) {
        PORTA |= (1 << LED1);  // Far distance (>30 cm)
    } else if (distance > 20 && distance <= 30) {
        PORTA |= (1 << LED2);  // Medium distance (20 - 30 cm)
    } else {
        PORTA |= (1 << LED3);  // Near distance (<=20 cm)
        PORTA |= (1 << BUZZER); // Activate buzzer
    }
}

void main(void) {
    DDRB |= (1 << TRIG_PIN);                // TRIG as output
    DDRB &= ~(1 << ECHO_PIN);               // ECHO as input
    DDRA |= (1 << LED1) | (1 << LED2) | (1 << LED3) | (1 << BUZZER); // LEDs and buzzer as outputs
    
    while (1) {
        unsigned int distance = readDistance();
        updateIndicators(distance);
        delay_ms(100);  // Small delay between readings
    }
}
