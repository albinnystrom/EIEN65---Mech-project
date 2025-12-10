#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include "serialport.h"

int engine_rpm = 1000;
int target_rpm = 1500;
int P = 10;
int I = 1;
int D = 1;

typedef enum Message
{
    NEW_SPEED,
    NEW_P,
    NEW_I,
    NEW_D,
} Message;

pthread_mutex_t lock;

void *send_message(Message msg);

void *status_thread(void *arg)
{
    while (1)
    {
        pthread_mutex_lock(&lock);

        // Simple “physics”: move engine rpm slowly towards target
        if (engine_rpm < target_rpm)
            engine_rpm++;
        else if (engine_rpm > target_rpm)
            engine_rpm--;

        // Save cursor position
        printf("\033[s");

        // Go to row 1, col 1
        printf("\033[1;1H");
        printf("Engine running at %4d rpm | ", engine_rpm);
        printf("Target speed %4d rpm  | ", target_rpm);
        printf("P=%d I=%d D=%d", P, I, D);

        // Restore previous cursor position (where user is typing)
        printf("\033[u");

        fflush(stdout);
        pthread_mutex_unlock(&lock);

        usleep(100000); // 100 ms
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
        printf(">Command (Speed/P/I/D): "); // prompt (user can type e.g. "Speed 1500")
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

        // Optional: show a tiny feedback / error message on the same line
        pthread_mutex_lock(&lock);
        printf("\033[2;1H");
        printf("\033[2K"); // clear line again
        if (matched)
        {
            usleep(500000);
            printf("> OK\n");
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
    while (1)
    {
        pthread_mutex_lock(&lock);

        if (old_target_rpm != target_rpm)
        {
            Message msg = NEW_SPEED;
            send_message(msg);
            old_target_rpm = target_rpm;
        }
        
        if (old_P != P)
        {
            Message msg = NEW_P;
            send_message(msg);
            old_P = P;
        }
        
        if (old_I != I)
        {
            Message msg = NEW_I;
            send_message(msg);
            old_I = I;
        }
        
        if (old_D != D)
        {
            Message msg = NEW_D;
            send_message(msg);
            old_D = D;
        }

        pthread_mutex_unlock(&lock);
        usleep(200000); // 200ms
    }
}

int sp;
int8_t cout;

void *send_message(Message msg)
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

    sp = serial_init("/dev/ttyS0",0);
    write(sp,(int8_t)msg,1);
}

int main(void)
{
    pthread_t t1, t2;

    // Clear screen and move cursor home
    printf("\033[2J\033[H");
    fflush(stdout);

    pthread_mutex_init(&lock, NULL);

    pthread_create(&t1, NULL, status_thread, NULL);
    pthread_create(&t2, NULL, input_thread, NULL);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    return 0;
}