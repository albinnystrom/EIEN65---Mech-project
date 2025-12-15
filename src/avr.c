#define F_CPU 1000000
#include <avr/io.h>
#include <avr/interrupt.h> //We need these definitions to be able to use interrupts
#include <util/delay.h>

#define BAUD 2400
#define MYUBRR ((F_CPU / (16UL * BAUD)) - 1)

int init_PWM(void)
{
	DDRD |= ((1<<DDD5)|(1<<DDD6));	//Set PIND5 and PIND6 as outputs
	TCCR0A |= (1<<COM0A1)|(1<<COM0B1)|(1<<COM0B0)|(1<<WGM01)|(1<<WGM00); //0b10110011 - COM0A non-inverting, COM0B inverting, Mode Fast-PWM
	TCCR0B |= (1<<CS00); //0b00000001 - Clock select: internal clock no prescaler
	return 1;
}

int updatePWM(int value)
{
	OCR0A = value;
	OCR0B = value;
	//OCR0B = 127;  //If we uncomment this line, the output COM0B is fixed at about 50% duty-cycle
	return value;
}

int init_LEDs(void)
{
	DDRB |= 0b11000000;	// Corresponding pins set as outputs 0b 0000 1111
	DDRD |= 0b00100000;	// Corresponding pins set as outputs 0b 0000 1111
	PORTB &= ~(0b11000000);	// Initially all LED pins set to 0
	PORTD &= ~(0b00100000);	// Initially all LED pins set to 0
	return 1;
}

int set_LED(int position, int value)
{
	switch(position)
	{
		case 1:
		if (value == 0)
		{
			PORTB &= ~(1 << 6);
		}
		else
		{
			PORTB |= (1 << 6);
		}
		break;
		case 2:
		if (value == 0)
		{
			PORTB &= ~(1 << 7);
		}
		else
		{
			PORTB |= (1 << 7);
		}
		break;
		case 3:
		if (value == 0)
		{
			PORTD &= ~(1 << 5);
		}
		else
		{
			PORTD |= (1 << 5);
		}
		break;
		
	}
	return 1;
}

void init_USART (unsigned int ubrr) {
	/*Set baud rate*/
	UBRR0H = (unsigned char) (ubrr>>8);
	UBRR0L = (unsigned char)ubrr;
	
	/*Enable receiver and transmitter*/
	UCSR0B = (1<<RXEN0) | (1<<TXEN0);
	
	/*Set frame format: 8data, 1stop bit*/
	UCSR0C = (1<<UCSZ01)|(1<<UCSZ00);

}

void USART_Transmit(unsigned char data){
	/* Wait for empty transmit buffer */
	while ( !(UCSR0A & (1<<UDRE0)));
	UDR0 = data;
}

unsigned char USART_Receive(void){
	/* Wait for empty transmit buffer */
	while ( !(UCSR0A & (1<<RXC0)));
	return UDR0;
}

int P;
int I, I_SUM, I_MAX;
int D;
int target_speed;
int last_err;

void init_PID(int p, int i, int d) {
	P = p;
	I = i;
	D = d;
}

int calc_PID(int value) {
	/* P */
	int err = target_speed - value;
	int u = 1/P * err;
	
	/* I */
	int new_sum = I_SUM + err;
	/* Anti wind-up*/
	new_sum = I_SUM > I_MAX ? I_MAX : new_sum;
	new_sum = I_SUM < -I_MAX ? -I_MAX : new_sum;
	
	I_SUM = new_sum;
	u += 1/I * new_sum;
	
	/* D */
	u += 1/D * (err-last_err);
	
	last_err = err;
	return u;
}

int main(void){
	init_LEDs();
	int onoff = 0;
	for(int i = 0; i < 5; i++) {
		set_LED(1,onoff);
		_delay_ms(100);
		set_LED(2,onoff);
		_delay_ms(100);
		set_LED(3,onoff);
		_delay_ms(250);
		onoff = !onoff;
	}
	init_USART(MYUBRR);
	//init_PWM();
	int sp;
	unsigned char serialin, serialout;

	while(1){
		serialin = USART_Receive();
		serialout = '8';
		switch (serialin) {
			case '1': set_LED(1,1); break;
			case '2':
			set_LED(2,1);
			break;
			case '3':
			set_LED(1,0);
			break;
			case '4':
			set_LED(2,0);
			break;
		}
		USART_Transmit(serialin);
	}
}
