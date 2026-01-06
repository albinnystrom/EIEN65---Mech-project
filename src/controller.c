#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include "serialport.h"

uint8_t engine_rpm = 1000;
int target_rpm = 1500;
int P = 10;
int I = 1;
int D = 1;
int LED = -1;
int last_recieved = 2;
unsigned char sent_msg = 0;

typedef enum Message
{
    NEW_SPEED,
    NEW_P,
    NEW_I,
    NEW_D,
    SET_LED,
} Message;

pthread_mutex_t lock;

void send_message(unsigned char msg, int value);

uint8_t get_speed();

void *status_thread(void *arg)
{
    while (1)
    {
        pthread_mutex_lock(&lock);

        // Simple “physics”: move engine rpm slowly towards target
        engine_rpm = get_speed();

        // Save cursor position
        printf("\033[s");

        // Go to row 1, col 1
        printf("\033[1;1H");
        printf("Engine running at %4d rpm | ", engine_rpm);
        printf("Target speed %4d rpm  | ", target_rpm);
        printf("P=%d I=%d D=%d, msg=%d", P, I, D, last_recieved);

        // Restore previous cursor position (where user is typing)
        printf("\033[u");

        fflush(stdout);
        pthread_mutex_unlock(&lock);

        usleep(1000000); // 100 ms
    }
    return NULL;
}

void *input_thread(void *arg)
{
    char line[128];

    while (1)
    {
        // Show prompt on line 3
        pthread_mutex_lock(&lock);
        printf("\033[2;1H");                // go to row 3, col 1
        printf("\033[2K");                  // clear the line
        printf(">Command (Speed/P/I/D/L): "); // prompt (user can type e.g. "Speed 1500")
        fflush(stdout);
        pthread_mutex_unlock(&lock);

        // Block for a full line of input (no lock while blocking)
        if (fgets(line, sizeof(line), stdin) == NULL)
        {
            // EOF or error
            break;
        }

        // Try to parse the different commands:
        int value;
        int matched = 0;

        // Speed <INT>
        if (sscanf(line, "Speed %d", &value) == 1)
        {
            pthread_mutex_lock(&lock);
            target_rpm = value;
            pthread_mutex_unlock(&lock);
            matched = 1;
        }
        // P <INT>
        else if (sscanf(line, "P %d", &value) == 1)
        {
            pthread_mutex_lock(&lock);
            P = value;
            pthread_mutex_unlock(&lock);
            matched = 1;
        }
        // I <INT>
        else if (sscanf(line, "I %d", &value) == 1)
        {
            pthread_mutex_lock(&lock);
            I = value;
            pthread_mutex_unlock(&lock);
            matched = 1;
        }
        // D <INT>
        else if (sscanf(line, "D %d", &value) == 1)
        {
            pthread_mutex_lock(&lock);
            D = value;
            pthread_mutex_unlock(&lock);
            matched = 1;
        }
        // L <INT>
        else if (sscanf(line, "L %d", &value) == 1)
        {
            pthread_mutex_lock(&lock);
            LED = value;
            pthread_mutex_unlock(&lock);
            matched = 1;
        }

        // Optional: show a tiny feedback / error message on the same line
        pthread_mutex_lock(&lock);
        printf("\033[2;1H");
        printf("\033[2K"); // clear line again
        if (matched)
        {
            printf("> OK, LED=%d\n",LED);
            usleep(500000);
        }
        else
        {
            printf("> Invalid command. Use: \"Speed <int>\", \"P <int>\", \"I <int>\", \"D <int>\"\n");
            usleep(2000000);
        }
        fflush(stdout);
        pthread_mutex_unlock(&lock);

        // Small pause so the user can see the message,
        // then the loop will redraw a fresh prompt anyway.
        usleep(200000); // 200 ms
    }

    return NULL;
}

void *controller_thread(void *arg)
{
    int old_target_rpm = -1;
    int old_P = -1;
    int old_I = -1;
    int old_D = -1;
    int old_LED = -1;
    while (1)
    {
        pthread_mutex_lock(&lock);

        if (old_target_rpm != target_rpm)
        {
            Message msg = NEW_SPEED;
            send_message('S',target_rpm);
            old_target_rpm = target_rpm;
        }
        
        if (old_P != P)
        {
            Message msg = NEW_P;
            send_message('P',P);
            old_P = P;
        }
        
        if (old_I != I)
        {
            Message msg = NEW_I;
            send_message('I',I);
            old_I = I;
        }
        
        if (old_D != D)
        {
            Message msg = NEW_D;
            send_message('D',D);
            old_D = D;
        }
        if (old_LED != LED)
        {
            Message msg = SET_LED;
            send_message('L',LED);
            old_LED = LED;
        }

        pthread_mutex_unlock(&lock);
        usleep(200000); // 200ms
    }
}

int sp;
int8_t cout;
void send_message(unsigned char msg, int value)

{
    /*
    PROTOCOL:
    PROGRAM <--> AVR

    1) Handshake (check AVR is ready to receive a command)

    -> READY?          (PC asks if AVR can accept a new message)
    <- READY           (AVR confirms it's ready)

    2) Send message (fixed layout)

    -> STX             (start-of-message marker, e.g. 0x02)
    -> MSG TYPE        (fixed length, e.g. 1 byte)
    -> MSG VALUE       (fixed length, e.g. N bytes)
    -> CHECKSUM        (checksum of [MSG TYPE + MSG VALUE])

    3) ACK / ERROR

    <- OK              (checksum valid, command accepted & executed)
    <- ERR             (checksum invalid or command rejected)
    */
    sent_msg = 1;
    char val = value + '0';

    write(sp,&msg,1);
    write(sp,&val,1);

    /* Busy-wait until a response is received */
    int inChar;
    while(!inChar) {
        read(sp, &inChar, 1);
    }
    last_recieved = inChar;
}

uint8_t get_speed(){
    char cmd = 'G';
    char val = '0';

    write(sp,&cmd,1);
    write(sp,&val,1);

    /* Busy-wait until a response is received */
    unsigned char inChar = 0;
    while(!inChar) {
        read(sp, &inChar, 1);
    }
    return (uint8_t)inChar;
}

int main(void)
{
    pthread_t t1, t2, t3;

    sp = serial_init("/dev/ttyS0", 0);
    if (sp <= 0) {
        perror("serial_init");
        return 1;
    }

    // Clear screen and move cursor home
    printf("\033[2J\033[H");
    fflush(stdout);

    pthread_mutex_init(&lock, NULL);

    pthread_create(&t1, NULL, status_thread, NULL);
    pthread_create(&t2, NULL, input_thread, NULL);
    pthread_create(&t3, NULL, controller_thread, NULL);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    pthread_join(t3, NULL);

    return 0;
}