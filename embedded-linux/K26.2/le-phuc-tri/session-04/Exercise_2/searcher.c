#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void print_grade(float gpa)
{
    if (gpa >= 8.5f) {
        printf("  Grade   : Excellent\n");
    }
    else if (gpa >= 7.0f) {
        printf("  Grade   : Good\n");
    }
    else if (gpa >= 5.0f) {
        printf("  Grade   : Average\n");
    }
    else {
        printf("  Grade   : Poor\n");
    }
}

int main(int argc, char *argv[])
{
    if (argc < 3) {
        fprintf(stderr,
                "Usage: %s <student_id> <data_file>\n",
                argv[0]);
        exit(2);
    }

    const char *student_id = argv[1];
    const char *filename = argv[2];

    printf("[SEARCHER] PID: %d | PPID: %d\n",
           getpid(), getppid());

    printf("[SEARCHER] Searching for \"%s\" in %s...\n\n",
           student_id, filename);

    FILE *file = fopen(filename, "r");

    if (file == NULL) {
        perror("fopen");
        exit(2);
    }

    char line[256];

    while (fgets(line, sizeof(line), file) != NULL) {

        /*
         * Remove newline character.
         */
        line[strcspn(line, "\n")] = '\0';

        /*
         * Split:
         * ID | Name | Class | GPA
         */
        char *id = strtok(line, "|");
        char *name = strtok(NULL, "|");
        char *class_name = strtok(NULL, "|");
        char *gpa_string = strtok(NULL, "|");

        /*
         * Check malformed line.
         */
        if (id == NULL ||
            name == NULL ||
            class_name == NULL ||
            gpa_string == NULL) {
            continue;
        }

        /*
         * Compare student ID.
         */
        if (strcmp(id, student_id) == 0) {

            float gpa = atof(gpa_string);

            printf("========== SEARCH RESULT ==========\n");
            printf("  ID      : %s\n", id);
            printf("  Name    : %s\n", name);
            printf("  Class   : %s\n", class_name);
            printf("  GPA     : %.1f\n", gpa);

            print_grade(gpa);

            printf("====================================\n");

            fclose(file);

            /*
             * Student found.
             */
            exit(0);
        }
    }

    fclose(file);

    printf("[SEARCHER] No student found with ID: %s\n",
           student_id);

    /*
     * Student not found.
     */
    exit(1);
}
