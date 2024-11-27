/* 
 * Operating Systems  (2INCO)  Practical Assignment
 * Interprocess Communication
 *
 * STUDENT_NAME_1 (STUDENT_NR_1)
 * STUDENT_NAME_2 (STUDENT_NR_2)
 *
 * Grading:
 * Your work will be evaluated based on the following criteria:
 * - Satisfaction of all the specifications
 * - Correctness of the program
 * - Coding style
 * - Report quality
 * - Deadlock analysis
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>      // for perror()
#include <unistd.h>     // for getpid()
#include <mqueue.h>     // for mq-stuff
#include <time.h>       // for time()

#include "messages.h"
#include "service1.h"

static void rsleep (int t);

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <S1_queue_name> <Rsp_queue_name>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *s1_queue_name = argv[1];
    const char *rsp_queue_name = argv[2];

    mqd_t s1_queue, rsp_queue; // Message queue descriptors
    struct mq_attr attr;       // Queue attributes
    Message msg;               // Message structure

    // Open the S1 message queue for reading
    s1_queue = mq_open(s1_queue_name, O_RDONLY);
    if (s1_queue == (mqd_t)-1) {
        perror("mq_open S1_queue");
        exit(EXIT_FAILURE);
    }

    // Open the Rsp message queue for writing
    rsp_queue = mq_open(rsp_queue_name, O_WRONLY);
    if (rsp_queue == (mqd_t)-1) {
        perror("mq_open Rsp_queue");
        mq_close(s1_queue);
        exit(EXIT_FAILURE);
    }

    printf("Service 1 Worker: Listening for jobs...\n");

    while (true) {
        // Receive a job from the S1 queue
        ssize_t bytes_read = mq_receive(s1_queue, (char *)&msg, sizeof(msg), NULL);
        if (bytes_read == -1) {
            perror("mq_receive");
            break;
        }

        printf("Service 1 Worker: Received job %d with data %d\n", msg.jobID, msg.data);

        // Process the job
        int result = service(msg.data);
        printf("Service 1 Worker: Processed job %d, result = %d\n", msg.jobID, result);

        // Send the result back to the Rsp queue
        msg.data = result; // Replace input data with the result
        if (mq_send(rsp_queue, (const char *)&msg, sizeof(msg), 0) == -1) {
            perror("mq_send");
            break;
        }

        printf("Service 1 Worker: Sent result for job %d\n", msg.jobID);
    }

    // Cleanup
    mq_close(s1_queue);
    mq_close(rsp_queue);

    return 0;
}


/*
 * rsleep(int t)
 *
 * The calling thread will be suspended for a random amount of time
 * between 0 and t microseconds
 * At the first call, the random generator is seeded with the current time
 */
static void rsleep (int t)
{
    static bool first_call = true;
    
    if (first_call == true)
    {
        srandom (time (NULL) % getpid ());
        first_call = false;
    }
    usleep (random() % t);
}
