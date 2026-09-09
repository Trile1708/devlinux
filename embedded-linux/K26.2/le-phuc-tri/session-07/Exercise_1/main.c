#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>

volatile sig_atomic_t reading_count = 0;

void handle_sigint(int sig)
{
    (void)sig;

    printf("[WARN] Received SIGINT, ignoring...\n");
    fflush(stdout);
}

void handle_sigterm(int sig)
{
    (void)sig;

    printf("[INFO] Received SIGTERM, shutting down gracefully...\n");
    fflush(stdout);

    exit(0);
}

void handle_sigusr1(int sig)
{
    (void)sig;

    printf("[REPORT] Total readings so far: %d\n", reading_count);
    fflush(stdout);
}

int main(void)
{
    /* Register signal handlers */
    if (signal(SIGINT, handle_sigint) == SIG_ERR) {
        perror("signal(SIGINT)");
        return EXIT_FAILURE;
    }

    if (signal(SIGTERM, handle_sigterm) == SIG_ERR) {
        perror("signal(SIGTERM)");
        return EXIT_FAILURE;
    }

    if (signal(SIGUSR1, handle_sigusr1) == SIG_ERR) {
        perror("signal(SIGUSR1)");
        return EXIT_FAILURE;
    }

    srand((unsigned int)time(NULL));

    printf("[INFO] Sensor daemon started. PID=%d\n", getpid());
    fflush(stdout);

    while (1) {
        int temperature = 20 + rand() % 21;  /* 20 - 40 */

        reading_count++;

        printf("[INFO] Sensor reading #%d: temperature=%d\n",
               reading_count,
               temperature);
        fflush(stdout);

        sleep(1);
    }

    return 0;
}
