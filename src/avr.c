#define FOSC 1843200
#define BAUD 9600
#define MYUBRR FOSC/16/BAUD-1

int main(void){
    init_PWM();
    init_LEDs();
    init_USART(MYUBRR);
    
    for(int i = 0; i < 5; i++) {
        set_LED(1,1);
        _delay_ms(200);
        set_led(1,0);
        _delay_ms(200);
    }
    int sp;
    int8_t serialin, serialout;

    while(true){
        serialin = USART_Recieve();
        switch (serialin) {
            case 1:
                set_LED(1,1);
                break;
            case 2:
                set_LED(2,1);
                break;
            case 3:
                set_LED(1,0);
                break;
            case 4:
                set_LED(2,0);
                break;
        }
    }
}

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
	DDRC |= 0x0F;	// Corresponding pins set as outputs 0b 0000 1111 
	PORTC &= ~(0x0F);	// Initially all LED pins set to 0
	return 1;
}

int set_LED(int position, int value)
{
	switch(position)
	{
		case 1:
		if (value == 0)
		{
			PORTC &= ~(1 << 1);
		}
		else
		{
			PORTC |= (1 << 1);
		}
		break;
		case 2:
		if (value == 0)
		{
			PORTC &= ~(1 << 2);
		}
		else
		{
			PORTC |= (1 << 2);
		}
		break;
		case 3:
		if (value == 0)
		{
			PORTC &= ~(1 << 3);
		}
		else
		{
			PORTC |= (1 << 3);
		}
		break;
		
	}
	return 1;
}

void init_USART (unsigned int ubrr) {
    /*Set baud rate*/
    UBRR0H = (unsigned char) (ubrr>>8);
    UBRR0L = (unsigned char)ubrr;
    
    /*Enable reciever and transmitter*/
    USCR0B = (1<<RXEN0) | (1<<TXEN0);
    
    /*Set frame format: 8data, 2stop bit*/
    USCR0C = (1<<USBS0) | (3<<USCZ00);
}

void USART_Transmit(unsigned char data){
    /* Wait for empty trasmit buffer */
    while ( !(USCRnA & (1<<UDREn)));
    UDRn = data;
}

void USART_Recieve(void){
    /* Wait for empty trasmit buffer */
    while ( !(USCRnA & (1<<RXCn)));
    return UDRn;
}