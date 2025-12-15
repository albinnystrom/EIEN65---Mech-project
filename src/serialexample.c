#include "serialport.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

/* Function to print a byte in binary */
void print_byte_binary(unsigned char byte) {
    for(int i = 7; i >= 0; i--) {
        printf("%c", (byte & (1 << i)) ? '1' : '0');
    }
    printf("\n");
}

int main(void)
{
    int sp;
    char outChar;
    char inChar = 0;

    /* Initialize serial port */
    sp = serial_init("/dev/ttyS0", 0);
    if(sp == 0) {
        printf("Error! Serial port could not be opened.\n");
        return 1;
    }
    printf("Serial port open with identifier %d \n", sp);

    /* Ask user for a single character */
    printf("Enter a single character to send: ");
    scanf(" %c", &outChar); // skip whitespace

    /* Send the character */
    write(sp, &outChar, 1);

    /* Busy-wait until a response is received */
    while(inChar == 0) {
        read(sp, &inChar, 1);
    }

    /* Print the received byte in binary */
	printf("Char: %c\n", (unsigned  char)inChar);
    printf("Received byte in binary: ");
    print_byte_binary((unsigned char)inChar);

    /* Close the serial port */
    serial_cleanup(sp);

    return 0;
}
