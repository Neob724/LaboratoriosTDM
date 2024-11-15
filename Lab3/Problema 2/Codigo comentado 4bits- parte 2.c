#define F_CPU 16000000UL
#define BAUD 9600
#define MYUBRR ((F_CPU / (16UL * BAUD)) - 1)
#define F_SCL 100000UL       // Frecuencia de SCL (100 kHz)
#define TWBR_VALUE ((F_CPU / F_SCL - 16) / 2)  // Valor del registro TWBR
#define SLA_W 0x27           // Dirección del esclavo con escritura (<< 1 | 0 para escritura)
#define LCD_RS 4
#define LCD_E  5
#define PCF8574_ADDRESS 0x20  // Dirección base (ajustar según hardware)
#include <stdbool.h>
#include <avr/io.h>
#include <util/delay.h>
#include <string.h>

char data[8];
uint8_t posicion = 0;

char keypad[4][4] = {
	{'7', '8', '9', '%'},
	{'4', '5', '6', '*'},
	{'1', '2', '3', '-'},
	{'C', '0', '=', '+'}
};

char read_keypad(void) {
	for (uint8_t row = 0; row <= 3; row++) {
		PORTD = ~(1 << row);
		for (uint8_t col = 4; col <= 7; col++) {
			if (!(PIND & (1 << col))) {
				
				_delay_ms(100);
				while (!(PIND & (1 << col)));
				_delay_ms(100);
				
				char tecla = keypad[row][col - 4];
				
				return tecla;
			}
		}
	}
	return 0;
}

void lcd_send(uint8_t HB) {
	PORTC = (PORTC & 0xF0) | (HB & 0x0F);  // Envía los 4 bits bajos a PC0-PC3
	PORTC |= (1 << PC5);  // E = 1
	_delay_us(1);         // Pequeño retardo
	PORTC &= ~(1 << PC5); // E = 0
	_delay_ms(2);         // Espera para que el LCD procese
}

void lcd_sendCommand(uint8_t command) {
	PORTC &= ~(1 << PC4); // RS = 0
	lcd_send(command >> 4);   // Parte alta
	lcd_send(command);        // Parte baja
}

void lcd_sendChar(uint8_t chart) {
	PORTC |= (1 << PC4);  // RS = 1
	lcd_send(chart >> 4);     // Parte alta
	lcd_send(chart);          // Parte baja
}

void lcd_sendString(const char* str) {
	while (*str) {
		lcd_sendChar(*str++);
	}
}

void lcd_clear(void) {
	lcd_sendCommand(0x01);
	_delay_ms(2);  // Espera para que se complete el comando de limpieza
}

void lcd_nextLine(void) {
	lcd_sendCommand(0xC0); // Comando para mover el cursor a la segunda línea
}


void lcd_init(void) {
	DDRC |= (1 << PC4) | (1 << PC5) | 0x0F;  // PC4 y PC5 como salida para RS y E, PC0-PC3 para datos

	_delay_ms(50);           // Espera inicial para la configuración
	lcd_sendCommand(0x03);
	lcd_sendCommand(0x03);
	lcd_sendCommand(0x02);
	lcd_sendCommand(0x28);       // Configura LCD: 4 bits, 2 líneas
	lcd_sendCommand(0x0F);       // Enciende display y cursor
	lcd_clear();                 // Limpia pantalla
	lcd_sendCommand(0x06);
}

void EEPROM_writeByte(uint16_t addres, uint8_t data) {
	while (EECR & (1 << EEPE));  // Espera hasta que la EEPROM esté libre
	EEAR = addres;               // Configura la dirección
	EEDR = data;                  // Carga el dato
	EECR |= (1 << EEMPE);         // Habilita el modo de escritura
	EECR |= (1 << EEPE);          // Inicia la escritura
}

void EEPROM_writeString(uint16_t start_address, const char* data) {
	while (*data) {
		EEPROM_writeByte(start_address++, *data++);  // Escribe cada byte usando EEPROM_writeByte
	}
}

char EEPROM_readByte(uint16_t addres) {
	while (EECR & (1 << EEPE));
	EEAR = addres;
	EECR |= (1 << EERE);
	return EEDR;
}

uint8_t EEPROM_verify(uint16_t addres) {
	while (EECR & (1 << EEPE));  // Espera si la EEPROM está ocupada
	EEAR = addres;               // Configura la dirección
	EECR |= (1 << EERE);          // Inicia la lectura
	return EEDR;                  // Devuelve el valor leído
}

void ERASE_password(uint16_t addres) {
	for(uint16_t i=addres;i<(addres+8);i++) {
		EEPROM_writeByte(addres,0xff);
	}
}

void save_key(char key) {
	if (key && posicion < 8) {
		data[posicion++] = key;
		data[posicion] = '\0';
	}
}

bool verify_password(char* data, uint16_t addres) {
	for (uint8_t i = 0; data[i] != '\0'; i++) {
		if (data[i] != EEPROM_readByte(addres++)) {
			return false;
		}
	}
	return EEPROM_readByte(addres) == 0xFF;
}

void clave_incorrecta(){
	while(1){
		lcd_clear();
		lcd_sendString("Bloqueado.");
		_delay_ms(100);
	}
}

void new_pass(void){
	while(1){
		lcd_clear();
		lcd_sendString("Nueva clave:");
		lcd_nextLine();
		lcd_sendString(data);
		char nkey = read_keypad();
		if (nkey >= '0' && nkey <= '9') save_key(nkey);
		else if (nkey == 'C') {
			if (posicion > 0) {
				posicion--;
				data[posicion] = '\0';
			}
		}
		
		size_t len = strlen(data);
		if(nkey=='=' && len>=6){
			ERASE_password(0x20);
			EEPROM_writeString(0x20,data);
			break;
		}
		_delay_ms(100);
	}
}

int main(void) {
	DDRC = 0xff;
	DDRB = 0x03;
	DDRD = 0x0F;
	DDRD &= ~((1 << PD4) | (1 << PD5) | (1 << PD6) | (1 << PD7));
	PORTD |= (1 << PD4) | (1 << PD5) | (1 << PD6) | (1 << PD7);
	
	lcd_init();
	char key;
	bool validacion=false;
	uint8_t intentos = 3;
	uint8_t verif=EEPROM_verify(0x20);
	if(verif==0xff) EEPROM_writeString(0x20, "12345678");
	
	while (1) {
		
		lcd_clear();
		lcd_sendString("Clave:");
		lcd_nextLine();
		lcd_sendString("Intentos:");
		lcd_sendCommand(0x80 + 7);
		lcd_sendString(data);
		lcd_sendCommand(0xC0 + 11);
		lcd_sendChar(intentos+'0');
		
		key=read_keypad();
		
		if (key >= '0' && key <= '9') save_key(key);
		else if (key == 'C') {
			if (posicion > 0) {
				posicion--;
				data[posicion] = '\0';
			}
		}
		
		validacion = verify_password(data,0x20);
		
		if (key == '=' && validacion==true)break;
		else if(key == '=' && validacion==false)intentos--;
		if(intentos==0){
			PORTB=2;
			clave_incorrecta();
		}
		_delay_ms(200);
	}
	
	lcd_clear();
	lcd_sendString("Acceso concedido");
	_delay_ms(300);
	lcd_clear();
	_delay_ms(300);
	lcd_clear();
	lcd_sendString("Acceso concedido");
	_delay_ms(300);
	lcd_clear();
	_delay_ms(300);
	lcd_clear();
	lcd_sendString("Acceso concedido");
	_delay_ms(1500);
	
	while (1){
		PORTB = 1;
		data[0] = '\0';
		posicion = 0;
		
		lcd_clear();
		lcd_sendString("Cambiar clave?");
		
		key=read_keypad();
		if(key=='=') new_pass();
		
		_delay_ms(200);
	}
}