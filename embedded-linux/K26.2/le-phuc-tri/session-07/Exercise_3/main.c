#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

int main(void)
{
    sigset_t block_set;
    sigset_t old_set;

    /* Create signal set containing SIGINT */
    sigemptyset(&block_set);
    sigaddset(&block_set, SIGINT);

    for (int i = 1; i <= 5; i++) {

        /* Block SIGINT and save the previous signal mask */
        if (sigprocmask(SIG_BLOCK, &block_set, &old_set) < 0) {
            perror("sigprocmask SIG_BLOCK");
            return EXIT_FAILURE;
        }

        printf("[SAFE] Writing transaction #%d ...\n", i);
        fflush(stdout);

        sleep(3);

        printf("[SAFE] Transaction #%d committed.\n", i);
        fflush(stdout);

        /*
         * Restore the signal mask that existed before
         * entering the critical section.
         */
        if (sigprocmask(SIG_SETMASK, &old_set, NULL) < 0) {
            perror("sigprocmask SIG_SETMASK");
            return EXIT_FAILURE;
        }

        printf("[IDLE] Waiting for next transaction...\n");
        fflush(stdout);

        sleep(3);
    }

    return EXIT_SUCCESS;
}
