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

void TWI_init(void) {
	TWBR = (uint8_t)TWBR_VALUE;   // Configura el baud rate para I2C
	TWSR = 0x00;                  // Prescaler en 1
}

void TWI_start(void) {
	TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);  // Enviar condición de START
	while (!(TWCR & (1 << TWINT)));                     // Esperar a que termine
}

void TWI_stop(void) {
	TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN);  // Enviar condición de STOP
}

void TWI_write(uint8_t data) {
	TWDR = data;                                   // Cargar el dato en el registro de datos
	TWCR = (1 << TWINT) | (1 << TWEN);             // Iniciar transmisión
	while (!(TWCR & (1 << TWINT)));                // Esperar a que termine
}

uint8_t TWI_get_status(void) {
	uint8_t status;
	status = TWSR & 0xF8;
	return status;
}

void PCF8574_write(uint8_t data) {
	TWI_start();
	TWI_write(PCF8574_ADDRESS << 1);
	TWI_write(data);
	TWI_stop();
}

void lcd_send(uint8_t data, uint8_t rs) {
	uint8_t highNibble = (data & 0xF0) >> 4;
	uint8_t lowNibble = data & 0x0F;
	
	highNibble <<= 4;
	lowNibble <<= 4;
	
	highNibble = highNibble | rs;  // Configura RS en highNibble
	lowNibble = lowNibble | rs;    // Configura RS en lowNibble

	PCF8574_write(highNibble + 0b00000100); // ENABLE = 1
	_delay_ms(100);
	PCF8574_write(highNibble); // ENABLE = 0
	_delay_ms(100);
	PCF8574_write(lowNibble + 0b00000100); // ENABLE = 1
	_delay_ms(100);
	PCF8574_write(lowNibble); // ENABLE = 0

	_delay_ms(2); // Retardo para que el LCD procese
}

void lcd_sendCommand(uint8_t command) {
	lcd_send(command, 0); // RS = 0 para comando
}

void lcd_sendChar(uint8_t character) {
	lcd_send(character, 1); // RS = 1 para datos
}

void lcd_sendString(const char* str) {
	while (*str) {
		lcd_sendChar(*str++);
	}
}

void lcd_clear(void) {
	lcd_sendCommand(0x01); // Comando para limpiar pantalla
	_delay_ms(2);
}

void lcd_nextLine(void) {
	lcd_sendCommand(0xC0); // Comando para mover el cursor a la segunda línea
}

void lcd_back(void) {
	lcd_sendCommand(0x10);     // Mueve el cursor una posición a la izquierda
	lcd_sendChar(' ');         // Escribe un espacio para borrar el carácter
	lcd_sendCommand(0x10);     // Mueve el cursor de nuevo a la izquierda
}

void lcd_init(void) {
	TWI_init();
	_delay_ms(50);
	lcd_sendCommand(0x03);
	lcd_sendCommand(0x03);
	lcd_sendCommand(0x02);
	lcd_sendCommand(0x28);
	lcd_sendCommand(0x0F); // Display ON, Cursor OFF
	lcd_clear();
	lcd_sendCommand(0x06); // Modo de entrada: cursor a la derecha
}

void EEPROM_writeByte(uint16_t address, uint8_t data) {
	while (EECR & (1 << EEPE));  // Espera hasta que la EEPROM esté libre
	EEAR = address;               // Configura la dirección
	EEDR = data;                  // Carga el dato
	EECR |= (1 << EEMPE);         // Habilita el modo de escritura
	EECR |= (1 << EEPE);          // Inicia la escritura
}

void EEPROM_writeString(uint16_t start_address, const char* data) {
	while (*data) {
		EEPROM_writeByte(start_address++, *data++);  // Escribe cada byte usando EEPROM_writeByte
	}
}

char EEPROM_readByte(uint16_t address) {
    while (EECR & (1 << EEPE));
    EEAR = address;
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

bool verify_password(char* data, uint16_t address) {
	for (uint8_t i = 0; data[i] != '\0'; i++) {
		if (data[i] != EEPROM_readByte(address++)) {
			return false;
		}
	}
	return EEPROM_readByte(address) == 0xFF;
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
	DDRC = 0x03;
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
		PORTC=2;
		clave_incorrecta();
		}
		_delay_ms(100);	
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
		PORTC = 1;
		data[0] = '\0';
		posicion = 0;
		
		lcd_clear();
		lcd_sendString("Cambiar clave?");
		
		key=read_keypad();
		if(key=='=') new_pass();
		
		_delay_ms(100);
	}
}