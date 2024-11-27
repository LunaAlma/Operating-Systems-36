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
#include <string.h>     // for strcpy
#include "messages.h"
#include "request.h"

static void rsleep(int t);

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <queue_name>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *queue_name = argv[1];
    mqd_t mqd; // Message queue descriptor
    struct mq_attr attr; // Queue attributes

    // Define message structure (from messages.h)
    typedef struct {
        int jobID;
        int serviceID;
        int data;
    } Message;

    // Open the message queue
    mqd = mq_open(queue_name, O_WRONLY);
    if (mqd == (mqd_t)-1) {
        perror("mq_open");
        exit(EXIT_FAILURE);
    }

    printf("Client: Opened message queue %s\n", queue_name);

    int jobID, serviceID, data;
    while (getNextRequest(&jobID, &data, &serviceID) != NO_REQ) {
        // Construct the message
        Message msg;
        msg.jobID = jobID;
        msg.serviceID = serviceID;
        msg.data = data;

        // Send the message to the queue
        if (mq_send(mqd, (const char *)&msg, sizeof(msg), 0) == -1) {
            perror("mq_send");
            mq_close(mqd);
            exit(EXIT_FAILURE);
        }

        printf("Client: Sent job ID %d, service %d, data %d\n", jobID, serviceID, data);

        // Simulate processing time
        rsleep(1);
    }

    // Close the message queue
    if (mq_close(mqd) == -1) {
        perror("mq_close");
        exit(EXIT_FAILURE);
    }

    printf("Client: Closed message queue %s\n", queue_name);
    return 0;
}

static void rsleep(int t) {
    usleep((rand() % t + 1) * 1000);
}

