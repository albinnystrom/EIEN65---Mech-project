/*Example serial port use in PC*/

/*Bridge the RXD and TXD pins in the serial port connector (or use your boards to perform a loop-back test!)*/
/*Compile the code, together with serialport.c ->  gcc -o myfile.exe serialexample.c serialport.c */
/*Execute your file!   ./myfile.exe */

#include "serialport.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
	/*Declaration of variables*/
	int sp,sl;
	char cin[8];
	char cout[8];
	
	/*Initialise serial port */
	sp = serial_init("/dev/ttyS0",0);
	if(sp == 0)
	{
		printf("Error! Serial port could not be opened.\n");
	}
	else
	{
		printf("Serial port open with identifier %d \n",sp);
	}
	
	/*Initialise both strings*/
	/*The max size of the strings is 8 characters (by declaration)*/
	/*Remember to leave space for the termination character!*/
	
	/*The output string is initialised through the keyboard...*/
	printf("Please enter your desired string:");
	scanf("%s", &cout);
	//OBS! scanf will stop reading if there are any space characters. In that case, try using the function fgets instead!

	/*The input string is initialised to xxxxxxxx*/
	strncpy(cin,"xxxxxxxx",8);
	
	/*Verify that the strings were successfully initialised... */	
	printf("Output: %s, Input: %s \n", cout, cin);

	/*Send the string out */
	write(sp,cout,8);
	/*NOTE: The string itself may be smaller than 9 bytes... 
	but we are still sending 9 bits. 
	When communicating with the AVR we must be more careful! 
	We may want to use fixed string length all the time,
	or have a for loop and send one character at a time*/
	
	
	/*Wait a little bit... (or until a signal comes!)*/
	sleep(1);
	
	/*Read the incoming string */
	read(sp,cin,8);

	/*Check that the input string is equal to the output one! */
	printf("Output: %s, Input: %s \n", cout, cin);
	
	/*Close the serial port */	
	serial_cleanup(sp);
	return 1;
}
