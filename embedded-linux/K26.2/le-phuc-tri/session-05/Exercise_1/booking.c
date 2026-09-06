#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#define NUM_AGENTS 5
#define TOTAL_SEATS 10
typedef struct {
	int agent_id;
	char customer[50];
	int seats_wanted;
} BookingRequest;

int seats_available = TOTAL_SEATS;
int failed_bookings= 0;
pthread_mutex_t seat_lock;

/*
 * IMPORTANT:
 *
 * The check and deduct operations MUST be inside the same
 * lock/unlock critical section.
 *
 * If we split them into two separate lock acquisitions:
 *
 *     lock
 *     check seats_available
 *     unlock
 *
 *     ... other thread may change seats_available ...
 *
 *     lock
 *     deduct seats
 *     unlock
 *
 * two threads could both see enough available seats and
 * both proceed to deduct them. This creates a race condition
 * and may cause more seats to be sold than actually exist.
 *
 * Therefore, "check + deduct" must be one atomic operation
 * protected by the same mutex lock.
 */
void *book_ticket(void *arg) 
{
	BookingRequest *request = (BookingRequest *) arg;
     /*
     * Sleep to force real concurrency.
     * All threads get a chance to start before entering
     * the critical section.
     */
    sleep(1);
    pthread_mutex_lock(&seat_lock);
     /*
     * Check and deduct are intentionally kept together.
     */
    if (seats_available >= request->seats_wanted) {
        seats_available -= request->seats_wanted;

        printf("[Agent %d | TID %lu] CONFIRMED: %d seat%s for %s. "
               "Remaining: %d\n",
               request->agent_id,
               (unsigned long)pthread_self(),
               request->seats_wanted,
               request->seats_wanted == 1 ? "" : "s",
               request->customer,
               seats_available);
    } else {
        failed_bookings++;

        printf("[Agent %d | TID %lu] SOLD OUT: needs %d seat%s, "
               "only %d left — booking failed.\n",
               request->agent_id,
               (unsigned long)pthread_self(),
               request->seats_wanted,
               request->seats_wanted == 1 ? "" : "s",
               seats_available);
    }
 pthread_mutex_unlock(&seat_lock);

    return NULL;
   }
int main(void)
{
    pthread_t threads[NUM_AGENTS];

    BookingRequest requests[NUM_AGENTS] = {
        {1, "Nguyen Van An",  2},
        {2, "Tran Thi Bich",  1},
        {3, "Le Van Cuong",   3},
        {4, "Pham Thi Dung",  1},
        {5, "Hoang Van Em",   2}
    };

    printf("==============================================\n");
    printf("   TICKET BOOKING SYSTEM (5 agents, 10 seats)\n");
    printf("==============================================\n");

    if (pthread_mutex_init(&seat_lock, NULL) != 0) {
        perror("pthread_mutex_init");
        return EXIT_FAILURE;
    }

    /*
     * Create 5 agent threads.
     */
    for (int i = 0; i < NUM_AGENTS; i++) {
        if (pthread_create(&threads[i], NULL,
                           book_ticket, &requests[i]) != 0) {
            perror("pthread_create");
            pthread_mutex_destroy(&seat_lock);
            return EXIT_FAILURE;
        }
    }

    /*
     * Wait for all agents to finish.
     */
    for (int i = 0; i < NUM_AGENTS; i++) {
        if (pthread_join(threads[i], NULL) != 0) {
            perror("pthread_join");
            pthread_mutex_destroy(&seat_lock);
            return EXIT_FAILURE;
        }
    }

    int seats_sold = TOTAL_SEATS - seats_available;

    printf("\n================ SUMMARY ================\n");
    printf("  Total seats     : %d\n", TOTAL_SEATS);
    printf("  Seats sold      : %d\n", seats_sold);
    printf("  Seats remaining : %d\n", seats_available);
    printf("  Failed bookings : %d\n", failed_bookings);
    printf("=========================================\n");

    pthread_mutex_destroy(&seat_lock);

    return EXIT_SUCCESS;
}
