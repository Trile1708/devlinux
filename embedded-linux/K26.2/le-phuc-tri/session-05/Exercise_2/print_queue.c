#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#define QUEUE_SIZE 5
#define NUM_PRODUCERS 3
#define DOCS_PER_PRODUCER 3
#define TOTAL_DOCUMENTS 9

typedef struct {
    int doc_id;
    char filename[60];
    int pages;
} Document;

/* Shared queue */
Document queue[QUEUE_SIZE];

int head = 0;
int tail = 0;
int count = 0;

int all_sent = 0;

/* Statistics */
int documents_submitted = 0;
int documents_printed = 0;
int total_pages_printed = 0;

/* Synchronization */
pthread_mutex_t q_lock;
pthread_cond_t not_full;
pthread_cond_t not_empty;

/*
 * IMPORTANT:
 *
 * pthread_cond_wait() must normally be used inside a WHILE
 * loop, not an IF statement.
 *
 * Example:
 *
 *     while (count == QUEUE_SIZE)
 *         pthread_cond_wait(&not_full, &q_lock);
 *
 * Why?
 *
 * 1. A condition variable does not remember that a condition
 *    became true. A thread must always re-check the actual
 *    condition after waking up.
 *
 * 2. Spurious wakeup can happen. This means a thread waiting
 *    on a condition variable may wake up even though no thread
 *    actually signaled the condition, or the condition it was
 *    waiting for is no longer true.
 *
 * 3. Multiple threads may be waiting for the same condition.
 *    When one thread changes the shared state and wakes a
 *    waiting thread, another thread may acquire the mutex first
 *    and change the state again.
 *
 * Therefore, after pthread_cond_wait() returns, the thread must
 * check the condition again.
 *
 * Using IF would be unsafe:
 *
 *     if (count == QUEUE_SIZE)
 *         pthread_cond_wait(&not_full, &q_lock);
 *
 * After waking up, the producer would continue without checking
 * whether the queue is actually still not full.
 *
 * The WHILE loop guarantees that the condition is valid before
 * the thread continues.
 */

/*
 * Add a document to the queue.
 *
 * q_lock must already be locked.
 */
void enqueue(Document doc)
{
    queue[tail] = doc;

    tail = (tail + 1) % QUEUE_SIZE;
    count++;
}

/*
 * Remove a document from the queue.
 *
 * q_lock must already be locked.
 */
Document dequeue(void)
{
    Document doc = queue[head];

    head = (head + 1) % QUEUE_SIZE;
    count--;

    return doc;
}

void *producer(void *arg)
{
    int producer_id = *(int *)arg;

    /*
     * Documents assigned to each producer.
     *
     * Producer 1:
     *   report_Q1.pdf
     *   slides.pdf
     *   summary.pdf
     *
     * Producer 2:
     *   contract.pdf
     *   memo.pdf
     *   budget.pdf
     *
     * Producer 3:
     *   invoice.pdf
     *   proposal.pdf
     *   ...
     */
    Document documents[NUM_PRODUCERS][DOCS_PER_PRODUCER] = {
        {
            {1, "report_Q1.pdf", 12},
            {2, "slides.pdf",    20},
            {3, "summary.pdf",    4}
        },
        {
            {4, "contract.pdf",  5},
            {5, "memo.pdf",      2},
            {6, "budget.pdf",    7}
        },
        {
            {7, "invoice.pdf",  3},
            {8, "proposal.pdf", 8},
            {9, "presentation.pdf", 5}
        }
    };

    for (int i = 0; i < DOCS_PER_PRODUCER; i++) {

        Document doc = documents[producer_id - 1][i];

        pthread_mutex_lock(&q_lock);

        /*
         * Wait while the queue is full.
         */
        while (count == QUEUE_SIZE) {
            printf("[Producer %d] Queue full — waiting...\n",
                   producer_id);

            pthread_cond_wait(&not_full, &q_lock);
        }

        /*
         * Add document to queue.
         */
        enqueue(doc);
        documents_submitted++;

        printf("[Producer %d] Submitting: %-18s (%d pages) — queue: %d/5\n",
               producer_id,
               doc.filename,
               doc.pages,
               count);

        /*
         * A new document is available.
         * Wake one waiting printer.
         */
        pthread_cond_signal(&not_empty);

        pthread_mutex_unlock(&q_lock);

        /*
         * Small delay so that producer/printer interleaving
         * is easier to observe.
         */
        usleep(100000);
    }

    return NULL;
}

void *printer(void *arg)
{
    (void)arg;

    while (1) {

        pthread_mutex_lock(&q_lock);

        /*
         * Wait while:
         *
         *   1. queue is empty
         *   2. producers have not all finished
         *
         * If all producers have finished and queue is empty,
         * there will never be another document.
         */
        while (count == 0 && !all_sent) {
            pthread_cond_wait(&not_empty, &q_lock);
        }

        /*
         * No documents remain and all producers have finished.
         * The printer can terminate.
         */
        if (count == 0 && all_sent) {
            pthread_mutex_unlock(&q_lock);
            break;
        }

        /*
         * Remove one document.
         */
        Document doc = dequeue();

        printf("[Printer]    Printing: %-18s (%d pages) — queue: %d/5\n",
               doc.filename,
               doc.pages,
               count);

        documents_printed++;
        total_pages_printed += doc.pages;

        /*
         * A slot is now available.
         * Wake one producer waiting because the queue was full.
         */
        pthread_cond_signal(&not_full);

        pthread_mutex_unlock(&q_lock);

        /*
         * Simulate printing time.
         *
         * IMPORTANT:
         * We sleep OUTSIDE the mutex.
         *
         * The printer should not hold q_lock while printing,
         * otherwise producers would unnecessarily be blocked.
         */
        sleep(1);
    }

    printf("[Printer]    All documents printed. Exiting.\n");

    return NULL;
}

int main(void)
{
    pthread_t producers[NUM_PRODUCERS];
    pthread_t printer_thread;

    int producer_ids[NUM_PRODUCERS] = {1, 2, 3};

    printf("==============================================\n");
    printf("   OFFICE PRINT QUEUE (3 producers, 1 printer)\n");
    printf("   Queue capacity: 5 documents\n");
    printf("==============================================\n\n");

    /*
     * Initialize mutex and condition variables.
     */
    if (pthread_mutex_init(&q_lock, NULL) != 0) {
        perror("pthread_mutex_init");
        return EXIT_FAILURE;
    }

    if (pthread_cond_init(&not_full, NULL) != 0) {
        perror("pthread_cond_init");
        pthread_mutex_destroy(&q_lock);
        return EXIT_FAILURE;
    }

    if (pthread_cond_init(&not_empty, NULL) != 0) {
        perror("pthread_cond_init");
        pthread_cond_destroy(&not_full);
        pthread_mutex_destroy(&q_lock);
        return EXIT_FAILURE;
    }

    /*
     * Create printer thread.
     */
    if (pthread_create(&printer_thread, NULL,
                       printer, NULL) != 0) {
        perror("pthread_create");

        pthread_cond_destroy(&not_empty);
        pthread_cond_destroy(&not_full);
        pthread_mutex_destroy(&q_lock);

        return EXIT_FAILURE;
    }

    /*
     * Create 3 producer threads.
     */
    for (int i = 0; i < NUM_PRODUCERS; i++) {

        if (pthread_create(&producers[i], NULL,
                           producer, &producer_ids[i]) != 0) {
            perror("pthread_create");

            return EXIT_FAILURE;
        }
    }

    /*
     * Wait for all producers to finish.
     */
    for (int i = 0; i < NUM_PRODUCERS; i++) {
        pthread_join(producers[i], NULL);
    }

    /*
     * All 9 documents have now been submitted.
     *
     * The printer may still have documents in the queue.
     * Set all_sent = 1 and wake the printer so it can
     * determine whether it should continue or exit.
     */
    pthread_mutex_lock(&q_lock);

    all_sent = 1;

    pthread_cond_broadcast(&not_empty);

    pthread_mutex_unlock(&q_lock);

    /*
     * Wait for printer to print all remaining documents.
     */
    pthread_join(printer_thread, NULL);

    printf("\n");
    printf("================ SUMMARY ================\n");
    printf("  Documents submitted : %d\n", documents_submitted);
    printf("  Documents printed   : %d\n", documents_printed);
    printf("  Total pages printed : %d\n", total_pages_printed);
    printf("=========================================\n");

    /*
     * Destroy synchronization objects.
     */
    pthread_cond_destroy(&not_empty);
    pthread_cond_destroy(&not_full);
    pthread_mutex_destroy(&q_lock);

    return EXIT_SUCCESS;
}
