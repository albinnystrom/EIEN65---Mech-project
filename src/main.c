#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>

int engine_rpm = 1000;
int target_rpm = 1500;
pthread_mutex_t lock;

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
        printf("Engine running at %4d rpm   \n", engine_rpm);
        printf("Target speed     %4d rpm   \n", target_rpm);

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
    while (1)
    {
        int new_speed;

        pthread_mutex_lock(&lock);
        // Move cursor to line 3, column 1 and print prompt
        printf("\033[3;1H"); // row 3, col 1¨
        printf("\033[2K");
        printf("> Set speed: ");
        fflush(stdout);
        pthread_mutex_unlock(&lock);

        if (scanf("%d", &new_speed) == 1)
        {
            pthread_mutex_lock(&lock);
            target_rpm = new_speed;
            pthread_mutex_unlock(&lock);
        }
        else
        {
            // Clear invalid input
            int c;
            while ((c = getchar()) != '\n' && c != EOF)
            {
            }
        }
    }
    return NULL;
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
