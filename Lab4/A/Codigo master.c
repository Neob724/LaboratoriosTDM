#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>

#define SS 2
#define MOSI 3
#define MISO 4
#define SCLK 5

uint8_t dato1 = 0x00;
uint8_t dato2 = 0x00;
uint8_t dato3 = 0x00;

//inicializa la uart
void SPI_Master() {
    DDRB |= (1 << MOSI) | (1 << MISO) | (1 << SCLK) | (1 << SS);
    SPCR = (1 << SPIE) | (1 << SPE) | (1 << MSTR) | (1 << SPR0);
}

//transmite datos vía SPI
void SPI_tx(char data, char slave) {
    PORTB &= ~(1 << slave);
    _delay_ms(10);
    SPDR = data;
    while (!(SPSR & (1 << SPIF)));
    _delay_ms(10);
    PORTB |= (1 << slave);
}

//lee un valor analógico del ADC
uint8_t analogRead(uint8_t canal) {
    ADMUX = (ADMUX & 0xF0) | (canal & 0x0F); //selecciona el canal del ADC
    ADCSRA |= (1 << ADSC); //inicia la conversión
    while (ADCSRA & (1 << ADSC)); //espera a que termine la conversión
    return ADC / 4; //retorna el valor del ADC
}

//inicializa el ADC
void adc() {
    ADMUX = (1 << REFS0); //usa AVcc como referencia de voltaje
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1);
}

int main() {
    DDRC |= 0; //configura los pines de entrada
    SPI_Master(); //inicializa SPI
    adc(); //inicializa ADC
    
    while (1) {
        //lee el estado del pin de entrada y asigna valores
        if (PINC & 2) {
            dato2 = 123;
        } else {
            dato2 = 0;
        }
        
        //verifica si el bit 2 de PINC está en alto
        if (PINC & 4) {
            _delay_ms(100); //espera para evitar rebotes
            while (PINC & 4); //espera hasta que el bit vuelva a bajo
            _delay_ms(100); //espera adicional para estabilidad
            dato3++; //incrementa el valor de dato3
            if (dato3 >= 2) { //reinicia a 0 si alcanza 2
                dato3 = 0;
            }
        }
        
        dato1 = analogRead(0); //lee el valor del ADC
        
        //transmite los datos vía SPI
        SPI_tx(dato1, SS);
        SPI_tx(dato2, SS);
        SPI_tx(dato3, SS);
        _delay_ms(10);
    }
}
