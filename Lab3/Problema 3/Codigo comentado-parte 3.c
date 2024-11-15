#define F_CPU 16000000UL
#define BAUD 9600
#define MYUBRR ((F_CPU / (16UL * BAUD)) - 1)
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>

#define DO 262
#define RE 294
#define MI 330
#define FA 349
#define SOL 392
#define LA 440
#define SI 494
#define DO_ 523

int control = 0;

void tx(unsigned char data) {
	while (!(UCSR0A & (1 << UDRE0)));
	UDR0 = data;
}

void print(const char* str) {
	while (*str) {
		tx(*str++);
	}
}

void setup() {
	UBRR0H = (unsigned char)(MYUBRR >> 8);
	UBRR0L = (unsigned char)MYUBRR;
    UCSR0B = (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0);
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
	DDRD &= ~(0b11110000);
	PORTD |= 0b11110000;
	DDRB &= ~(0b00111100);
	PORTB |= 0b00111100;
	DDRB |= (1 << PB1);
	TCCR1A |= (1 << COM1A1) | (1 << WGM11);
	TCCR1B |= (1 << WGM13) | (1 << WGM12) | (1 << CS10);
	ICR1 = 10000;
	OCR1A = 0;
    sei();
}

void pwmF(uint16_t freq) {
	ICR1 = F_CPU / freq;
	OCR1A = ICR1 / 5;
}

void nota(uint16_t frec) {
	switch (frec) {
		case DO:
		pwmF(frec);
		break;
		case RE:
		pwmF(frec);
		break;
		case MI:
		pwmF(frec);
		break;
		case FA:
		pwmF(frec);
		break;
		case SOL:
		pwmF(frec);
		break;
		case LA:
		pwmF(frec);
		break;
		case SI:
		pwmF(frec);
		break;
		case DO_:
		pwmF(frec);
		break;
		case 0:
		OCR1A=0;
		break;
		default:
		break;
	}
}

void pulso(uint16_t note){
		nota(note);
		_delay_ms(200);
}

void pausa(){
	nota(0);
	_delay_ms(200);
}

void cancion1(){
print("Reproduciendo cancion 1\r");
for(uint8_t estr=0;estr<=2;estr++){
	pausa();
	pulso(SOL);
	pausa();
	pausa();
	pulso(SOL);
	pausa();
	pausa();
	pulso(MI);
	pausa();
	pulso(FA);
	pausa();
	pulso(SOL);
	pausa();
	pulso(LA);
	pausa();
	pulso(SOL);
	pausa();

	pausa();
	pulso(FA);
	pausa();
	pulso(FA);
	pausa();
	pulso(MI);
	pausa();
	pausa();
	pulso(RE);
	pausa();
	pausa();
	
	pausa();
	pulso(MI);
	pausa();
	pausa();
	pulso(MI);
	pausa();
	pausa();
	pulso(DO);
	pausa();
	pulso(RE);
	pausa();
	pulso(MI);
	pausa();
	pulso(FA);
	pausa();
	pulso(MI);
	pausa(); 

	pausa();
	pulso(MI);
	pausa();
	pulso(RE);
	pausa();
	pulso(DO);
	pausa();
	pausa();
	pulso(DO);
	pausa();
	pausa();

	pausa();
	pulso(MI);
	pausa();
	pulso(MI);
	pausa();
	pulso(FA);
	pausa();
	pausa();
	pulso(LA);
	pausa();
	pausa();
	pulso(SOL);
	pausa();
	pulso(SOL);
	pausa();
	pulso(DO);
	pausa();
	
	pausa();
	pulso(RE);
	pausa();
	pulso(MI);
	pausa();
	pulso(FA);
	pausa();
	pausa();
	pulso(MI);
	pausa();
	pulso(MI);
	pausa();
	pulso(RE);
	pausa();
	pausa();
	pulso(DO);
	pausa();
	pausa();
	pulso(RE);
	pausa();
	pausa();
	pausa();

	pausa();
	pulso(RE);
	pausa();
	pausa();
	pulso(DO);
	pausa();
	pausa();
	pulso(DO);
	pausa();
	pausa();
	pausa();
	}
}

void cancion2(){
	print("Reproduciendo cancion 2\r");
	for(uint8_t estr=0;estr<=2;estr++){
		pausa();
		pulso(DO);
		pausa();
		pulso(DO);
		pausa();
		pulso(FA);
		pausa();
		pausa();

		pausa();
		pulso(DO_);
		pausa();
		pulso(SI);
		pausa();
		pulso(LA);
		pausa();
		pulso(SI);
		pausa();
		pausa();

		pausa();
		pulso(FA);
		pausa();
		pulso(SOL);
		pausa();
		pulso(LA);
		pausa();
		
		pausa();
		pulso(LA);
		pausa();
		pulso(LA);
		pausa();

		
		pausa();
		pulso(SI);
		pausa();
		pulso(LA);
		pausa();
		pulso(SOL);
		pausa();
		pausa();
	}
}

int main(void) {
	setup();
	print(" \r");
	print("\rModo piano\r");

	while (1) {
		if (!(PIND & (1 << PD4))) {
			print("Nota: do\r");
			nota(DO);
			while (!(PIND & (1 << PD4))) {
			}
		}
		else if (!(PIND & (1 << PD5))) {
			print("Nota: re\r");
			nota(RE);
			while (!(PIND & (1 << PD5))) {
			}
		}
		else if (!(PIND & (1 << PD6))) {
			print("Nota: mi\r");
			nota(MI);
			while (!(PIND & (1 << PD6))) {
			}
		}
		else if (!(PIND & (1 << PD7))) {
			print("Nota: fa\r");
			nota(FA);
			while (!(PIND & (1 << PD7))) {
			}
		}
		else if (!(PINB & (1 << PB2))) {
			print("Nota: sol\r");
			nota(SOL);
			while (!(PINB & (1 << PB2))) {
			}
		}
		else if (!(PINB & (1 << PB3))) {
			print("Nota: la\r");
			nota(LA);
			while (!(PINB & (1 << PB3))) {
			}
		}
		else if (!(PINB & (1 << PB4))) {
			print("Nota: si\r");
			nota(SI);
			while (!(PINB & (1 << PB4))) {
			}
		}
		else if (!(PINB & (1 << PB5))) {
			print("Nota: DO\r");
			nota(DO_);
			while (!(PINB & (1 << PB5))) {
			}
		}

		nota(0);
		_delay_ms(10);
		
		if (control==1){
			cancion1();
			control=0;
		}
		else if (control==2){
			cancion2();
			control=0;
		}
	}
}

ISR(USART_RX_vect) {
	char receivedChar = UDR0;
	if (receivedChar == '1') {
		control=1;
	}
	else if (receivedChar == '2') {
		control=2;
	}
	else if (receivedChar == 's') {
		control=0;
		asm volatile("jmp main");
	}
}