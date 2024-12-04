#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>    
#include <unistd.h>    // for execlp
#include <mqueue.h>    // for mq

#include "settings.h"  
#include "messages.h"

char client2dealer_name[30];
char dealer2worker1_name[30];
char dealer2worker2_name[30];
char worker2dealer_name[30];

mqd_t mq_fd_request, mq_fd_response, mq_fd_s1,  mq_fd_s2; 

void create_message_queues() {
    struct mq_attr attr;

    sprintf(client2dealer_name, "/Request_queue_36%d", getpid());
    sprintf(worker2dealer_name, "/Response_queue_36%d", getpid());
    sprintf(dealer2worker1_name, "/S1_queue_36%d", getpid());
    sprintf(dealer2worker2_name, "/S2_queue_36%d", getpid());

    attr.mq_maxmsg = MQ_MAX_MESSAGES;
    attr.mq_msgsize = sizeof(MQ_REQUEST_MESSAGE);

    mq_fd_request = mq_open(client2dealer_name, O_RDONLY | O_CREAT | O_NONBLOCK | O_EXCL, 0600, &attr);
    if (mq_fd_request == (mqd_t)-1) {
        perror("Making the request queue failed");
        exit(1);
    }

    mq_fd_s1 = mq_open(dealer2worker1_name, O_WRONLY | O_CREAT | O_EXCL, 0600, &attr);
    if (mq_fd_s1 == (mqd_t)-1) {
        perror("Making the S1 queue failed");
        exit(1);
    }

    mq_fd_s2 = mq_open(dealer2worker2_name, O_WRONLY | O_CREAT | O_EXCL, 0600, &attr);
    if (mq_fd_s2 == (mqd_t)-1) {
        perror("Making the S2 queue failed");
        exit(1);
    }

    attr.mq_msgsize = sizeof(MQ_RESPONSE_MESSAGE);
    mq_fd_response = mq_open(worker2dealer_name, O_RDONLY | O_CREAT | O_NONBLOCK | O_EXCL, 0600, &attr);
    if (mq_fd_response == (mqd_t)-1) {
        perror("Making the response queue failed");
        exit(1);
    }
}

pid_t create_client() {
    pid_t pid = fork();
    if (pid == 0) {
        execlp("./client", "client", client2dealer_name, NULL);
        perror("Failed to start client process");
        exit(1);
    }
    return pid;
}

void create_workers(int num_workers, const char *worker_program, const char *request_queue, const char *response_queue, pid_t *worker_pids, int *worker_index) {
    for (int i = 0; i < num_workers; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            execlp(worker_program, worker_program, request_queue, response_queue, NULL);
            perror("Failed to start worker process");
            exit(1);
        } else if (pid < 0) {
            perror("Failed to fork worker process");
            exit(1);
        }
        worker_pids[*worker_index] = pid;
        (*worker_index)++;
    }
}

bool is_full(mqd_t queue) {
    struct mq_attr check;
    if (mq_getattr(queue, &check) == -1) {
        perror("Failed to get attributes of request queue");
    }
    if (check.mq_curmsgs == MQ_MAX_MESSAGES) {
        return true;
    }
    return false;
}

void terminate_workers(pid_t *worker_pids, int num_workers) {
    MQ_REQUEST_MESSAGE termination_msg = { .RequestID = -1, .ServiceID = 0, .data = 0 };

    for (int i = 0; i < num_workers; i++) {
        mqd_t target_queue = (i < N_SERV1) ? mq_fd_s1 : mq_fd_s2;
        if (mq_send(target_queue, (const char *)&termination_msg, sizeof(termination_msg), 0) == -1) {
            perror("Failed to send termination message");
        }
    }

    // Wait for all worker processes
    for (int i = 0; i < num_workers; i++) {
        waitpid(worker_pids[i], NULL, 0);
    }
}

void clean_message_queues() {
    mq_unlink(client2dealer_name);
    mq_unlink(worker2dealer_name);
    mq_unlink(dealer2worker1_name);
    mq_unlink(dealer2worker2_name);
}


int main(int argc, char *argv[]) {
    if (argc != 1) {
        fprintf(stderr, "%s: invalid arguments\n", argv[0]);
    }

    MQ_REQUEST_MESSAGE req;
    MQ_RESPONSE_MESSAGE rsp;

    create_message_queues();

    pid_t client_pid = create_client();
    pid_t worker_pids[N_SERV1 + N_SERV2];
    int worker_index = 0;
    
    create_workers(N_SERV1, "./worker_s1", dealer2worker1_name, worker2dealer_name, worker_pids, &worker_index);
    create_workers(N_SERV2, "./worker_s2", dealer2worker2_name, worker2dealer_name, worker_pids, &worker_index);

    bool client_done = false;
    int pending = 0;
    struct mq_attr attr1; 

    while (true) {
        if (!client_done && waitpid(client_pid, NULL, WNOHANG) > 0) {
            client_done = true;
        }
        
        if (!is_full(mq_fd_s1) && !is_full(mq_fd_s2)) {
            if (mq_receive(mq_fd_request, (char *)&req, sizeof(req), NULL) > 0) {
                mqd_t target_queue = (req.ServiceID == 1) ? mq_fd_s1 : mq_fd_s2;
                if (mq_send(target_queue, (const char *)&req, sizeof(req), 0) == -1) {
                    perror("Failed to forward request to worker queue");
                } else {
                    pending++;
                }
            } 
        } 

        if (mq_receive(mq_fd_response, (char *)&rsp, sizeof(rsp), NULL) > 0) {
            printf("%d -> %d\n", rsp.ResponseID, rsp.result);
            pending--;
        }

        if (mq_getattr(mq_fd_request, &attr1) == -1) {
            perror("Failed to get attributes of request queue");
            break;  
        }
        
        if (client_done && pending == 0 && attr1.mq_curmsgs == 0) {
            break; 
        }
        
    }

    terminate_workers(worker_pids, worker_index);
    clean_message_queues();
    
    return 0;
}
