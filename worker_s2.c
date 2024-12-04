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
#include "service2.h"

static void rsleep (int t);

char* name = "NO_NAME_DEFINED";
mqd_t dealer2worker;
mqd_t worker2dealer;


int main (int argc, char * argv[])
{
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <request_queue> <response_queue>\n", argv[0]);
        return 1;
    }

    name = argv[1]; 
    dealer2worker = mq_open(name, O_RDONLY);
    if (dealer2worker == (mqd_t)-1) {
        perror("Opening request queue failed");
        return 1;
    }

    name = argv[2]; 
    mqd_t worker2dealer = mq_open(name, O_WRONLY);
    if (worker2dealer == (mqd_t)-1) {
        perror("Opening response queue failed");
        return 1;
    } 

    MQ_REQUEST_MESSAGE request_msg; 
    MQ_RESPONSE_MESSAGE response_msg; 

    while (true) {
        ssize_t msg = mq_receive(dealer2worker, (char *)&request_msg, sizeof(request_msg), NULL);
        if (msg == -1 && errno != EAGAIN) {
            break;
        }

        if (request_msg.RequestID == -1) {
            break;
        }

        rsleep(10000);

        response_msg.ResponseID = request_msg.RequestID;
        response_msg.result = service(request_msg.data);

        if (mq_send(worker2dealer, (const char *)&response_msg, sizeof(response_msg), 0) == -1) {
            perror("Failed to send message to response queue");
            break;
        }

    }

    // Close the message queues
    if (mq_close(dealer2worker) == -1) {
        perror("Failed to close request queue");
    }

    if (mq_close(worker2dealer) == -1) {
        perror("Failed to close response queue");
    }

    // TODO:
    // (see message_queue_test() in interprocess_basic.c)
    //  * open the two message queues (whose names are provided in the
    //    arguments)
    //  * repeatedly:
    //      - read from the S2 message queue the new job to do
    //      - wait a random amount of time (e.g. rsleep(10000);)
    //      - do the job 
    //      - write the results to the Rsp message queue
    //    until there are no more tasks to do
    //  * close the message queues

    return(0);
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
