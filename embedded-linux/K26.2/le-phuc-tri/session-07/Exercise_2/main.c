#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

void handle_sigusr1(int sig)
{
    (void)sig;

    printf("[GATEWAY] Worker reported READY signal received\n");
    fflush(stdout);
}

int main(void)
{
    pid_t pid;
    int status;

    /* Register SIGUSR1 handler */
    if (signal(SIGUSR1, handle_sigusr1) == SIG_ERR) {
        perror("signal");
        return EXIT_FAILURE;
    }

    /*
     * Create child process
     */
    pid = fork();

    if (pid < 0) {
        perror("fork");
        return EXIT_FAILURE;
    }

    /*
     * Parent process: Gateway
     */
    if (pid > 0) {
        sigset_t block_set;

        printf("[GATEWAY] Worker PID = %d\n", pid);
        fflush(stdout);

        /*
         * Create signal set containing SIGUSR1
         */
        sigemptyset(&block_set);
        sigaddset(&block_set, SIGUSR1);

        /*
         * Block SIGUSR1
         */
        if (sigprocmask(SIG_BLOCK, &block_set, NULL) < 0) {
            perror("sigprocmask block");
            return EXIT_FAILURE;
        }

        /*
         * Simulate gateway initialization
         */
        sleep(5);

        /*
         * Unblock SIGUSR1
         */
        if (sigprocmask(SIG_UNBLOCK, &block_set, NULL) < 0) {
            perror("sigprocmask unblock");
            return EXIT_FAILURE;
        }

        /*
         * Wait for worker to terminate
         */
        if (wait(&status) < 0) {
            perror("wait");
            return EXIT_FAILURE;
        }

        /*
         * Check whether child exited normally
         */
        if (WIFEXITED(status)) {
            printf("[GATEWAY] Worker exited with code %d\n",
                   WEXITSTATUS(status));
            fflush(stdout);
        }
    }

    /*
     * Child process: Worker
     */
    else {
        sleep(2);

        /*
         * Tell parent that worker is ready
         */
        if (kill(getppid(), SIGUSR1) < 0) {
            perror("kill");
            exit(EXIT_FAILURE);
        }

        printf("[WORKER] Sent READY signal to gateway\n");
        fflush(stdout);

        exit(7);
    }

    return EXIT_SUCCESS;
}
