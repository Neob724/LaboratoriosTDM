#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define SS 2 //pin conectado a SS habilitador de esclavo
#define MOSI 3 //pin conectado a MOSI (master output, slave input)
#define MISO 4 //pin conectado a MISO (master input, slave output)
#define SCLK 5 //pin conectado a SCLK (serial clock)

uint8_t datos[3]; //buffer para almacenar los tres bytes
uint8_t cont = 0;

void SPI_Slave() {
    DDRB |= (1 << MISO); //configura MISO como salida
    DDRB &= ~((1 << MOSI) | (1 << SCLK) | (1 << SS)); //configura MOSI, SCLK y SS como entrada
    SPCR = (1 << SPIE) | (1 << SPE); //habilita interrupción SPI y módulo SPI
    sei(); //habilita interrupciones globales
}

void pwm(){
    TCCR2A |= (1 << WGM21) | (1 << WGM20);
    TCCR2A |= (1 << COM2B1);
    TCCR2B |= (1 << CS22);

    OCR2B = 0;
}

ISR(SPI_STC_vect) {
    datos[cont] = SPDR; //almacena el byte recibido
    cont++; //incrementa el índice
    if (cont >= 3) { //si se recibieron los tres bytes
        cont = 0; //reinicia el índice
    }
}

int main() {
    DDRD |= (1 << PD3); //configura PD3 como salida (PWM)
    DDRC |= 0xFF; //configura todos los pines del puerto C como salida
    SPI_Slave(); //inicializa SPI en modo esclavo
    pwm(); //inicializa PWM
    
    _delay_ms(1000); //retardo inicial de 1 segundo
    
    while (1) {
        //si el segundo dato recibido es 123, activa PC1
        if (datos[1] == 123) {
            PORTC |= (1 << PC1); //enciende PC1
        } else {
            PORTC &= ~(1 << PC1); //apaga PC1
        }
        
        //actualiza el ciclo de trabajo del PWM con el primer dato recibido
        OCR2B = datos[0];
        
        //si el tercer dato recibido es 1, activa PC2, de lo contrario apágalo
        if (datos[2] == 1) {
            PORTC |= (1 << PC2); //enciende PC2
        } else {
            PORTC &= ~(1 << PC2); //apaga PC2
        }
    }
}
