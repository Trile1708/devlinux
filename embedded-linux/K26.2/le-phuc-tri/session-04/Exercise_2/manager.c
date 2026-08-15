#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

extern char **environ;

int main(void)
{
    char student_id[50];

    printf("=============================================\n");
    printf("   STUDENT LOOKUP SYSTEM — MANAGER\n");
    printf("   (fork + execve | file: students.txt)\n");
    printf("=============================================\n");

    printf("[MANAGER] PID: %d\n", getpid());
    printf("Enter student ID ('quit' to exit).\n");

    while (1) {
        printf("\n---------------------------------------------\n");
        printf("Student ID: ");
        fflush(stdout);

        if (fgets(student_id, sizeof(student_id), stdin) == NULL) {
            break;
        }

        /* Remove trailing newline */
        student_id[strcspn(student_id, "\n")] = '\0';

        if (strcmp(student_id, "quit") == 0) {
            printf("[MANAGER] Exiting. Goodbye!\n");
            break;
        }

        if (strlen(student_id) == 0) {
            printf("[MANAGER] Student ID cannot be empty.\n");
            continue;
        }

        fflush(stdout);

        pid_t pid = fork();

        if (pid < 0) {
            perror("fork");
            continue;
        }

        if (pid == 0) {
            /*
             * Child process:
             * Replace the child process image with ./searcher.
             */
            char *args[] = {
                "./searcher",
                student_id,
                "students.txt",
                NULL
            };

            execve("./searcher", args, environ);

            /*
             * execve() only returns if an error occurs.
             * Normally this line is never reached because successful
             * execve() completely replaces the current process image.
             */
            perror("execve failed");
            exit(2);
        }

        /*
         * Parent process
         */
        printf("[MANAGER] fork() → child PID: %d\n", pid);
        printf("[MANAGER] Waiting for child (waitpid)...\n\n");

        int status;

        pid_t result = waitpid(pid, &status, 0);

        if (result == -1) {
            perror("waitpid");
            continue;
        }

        if (WIFEXITED(status)) {
            int exit_code = WEXITSTATUS(status);

            if (exit_code == 0) {
                printf("[MANAGER] Child (PID %d) exited. "
                       "code=0 → Found\n", pid);
            }
            else if (exit_code == 1) {
                printf("[MANAGER] Child (PID %d) exited. "
                       "code=1 → Not found\n", pid);
            }
            else if (exit_code == 2) {
                printf("[MANAGER] Child (PID %d) exited. "
                       "code=2 → Error\n", pid);
            }
            else {
                printf("[MANAGER] Child (PID %d) exited. "
                       "code=%d → Unknown result\n",
                       pid, exit_code);
            }
        }
        else {
            printf("[MANAGER] Child (PID %d) did not exit normally.\n",
                   pid);
        }
    }

    return 0;
}
