
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <unistd.h>
#include <mqueue.h>
#include <time.h>
#include "messages.h"
#include "service2.h"

static void rsleep(int t);

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <S2_queue_name> <Rsp_queue_name>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *s2_queue_name = argv[1];
    const char *rsp_queue_name = argv[2];

    mqd_t s2_queue, rsp_queue;
    Message req_msg;
    Message rsp_msg;

    // Open the S2 queue for reading
    s2_queue = mq_open(s2_queue_name, O_RDONLY);
    if (s2_queue == (mqd_t)-1) {
        perror("mq_open S2_queue");
        exit(EXIT_FAILURE);
    }

    // Open the Rsp queue for writing
    rsp_queue = mq_open(rsp_queue_name, O_WRONLY);
    if (rsp_queue == (mqd_t)-1) {
        perror("mq_open Rsp_queue");
        mq_close(s2_queue);
        exit(EXIT_FAILURE);
    }

    printf("Service 2 Worker: Started processing jobs...\n");

    while (true) {
        ssize_t bytes_read = mq_receive(s2_queue, (char *)&req_msg, sizeof(req_msg), NULL);
        if (bytes_read == -1) {
            perror("mq_receive");
            break;
        }

        printf("Service 2 Worker: Received job ID %d with data %d\n", req_msg.RequestID, req_msg.data);

        // Process the request
        int result = service(req_msg.data);
        rsp_msg.ResponseID = req_msg.RequestID;
        rsp_msg.result = result;

        rsleep(10000);  // Simulate processing time

        // Send the response
        if (mq_send(rsp_queue, (const char *)&rsp_msg, sizeof(rsp_msg), 0) == -1) {
            perror("mq_send");
            break;
        }

        printf("Service 2 Worker: Completed job ID %d with result %d\n", rsp_msg.ResponseID, rsp_msg.result);
    }

    mq_close(s2_queue);
    mq_close(rsp_queue);
    printf("Service 2 Worker: Exiting...\n");
    return 0;
}

static void rsleep(int t) {
    static bool first_call = true;

    if (first_call) {
        srandom(time(NULL) % getpid());
        first_call = false;
    }
    usleep(random() % t);
}
