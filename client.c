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
#include "request.h"

static void rsleep (int t);


int main (int argc, char * argv[])
{

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <request_queue>\n", argv[0]);
        return 1;
    }
    
    const char *req_name = argv[1]; 
    mqd_t mq_req = mq_open(req_name, O_WRONLY);
    if (mq_req == (mqd_t)-1) {
        perror("Opening request queue failed");
        return 1;
    }

    MQ_REQUEST_MESSAGE request_msg; 

    while(true) {
        int i = getNextRequest(&request_message.RequestID, request_message.data, request_message.ServiceID); 

        if (i == NO_REQ) {
            if (mq_close(mq_req) == -1) {
                perror("Failed to close request queue");
                return 1;
            }
            break; 
        }

        if (mq_send(mq_req, (const char *)&request_message, sizeof(request_message), 0) == -1) {
            perror("Failed to send message to request queue");
            break;
        }

    }
    
    return (0);
}
