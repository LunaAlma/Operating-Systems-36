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
  struct mq_attr                attr;

  sprintf(client2dealer_name, "/Request_queue_36%d", getpid());
  sprintf(worker2dealer_name, "/Response_queue_36%d", getpid());
  sprintf(dealer2worker1_name, "/S1_queue_36%d", getpid());
  sprintf(dealer2worker2_name, "/S2_queue_36%d", getpid());

  attr.mq_maxmsg  = MQ_MAX_MESSAGES;
  attr.mq_msgsize = sizeof (MQ_REQUEST_MESSAGE);

  mq_fd_request = mq_open (client2dealer_name, O_WRONLY | O_CREAT | O_EXCL, 0600, &attr);
  if (mq_fd_request == (mqd_t)-1) {
        perror("Making the request queue failed");
        exit(1);
  }
  printf("Created Request queue: %s\n", client2dealer_name);

  mq_fd_s1 = mq_open (dealer2worker1_name, O_WRONLY | O_CREAT | O_EXCL, 0600, &attr);
  if (mq_fd_s1 == (mqd_t)-1) {
      perror("Making the s1 queue failed");
      exit(1);
  }
  printf("Created s1 queue: %s\n", dealer2worker1_name);

  mq_fd_s2 = mq_open (dealer2worker2_name, O_WRONLY | O_CREAT | O_EXCL, 0600, &attr);
  if (mq_fd_s2 == (mqd_t)-1) {
      perror("Making the s2 queue failed");
      exit(1);
  }
  printf("Created s2 queue: %s\n", dealer2worker2_name);

  attr.mq_msgsize = sizeof (MQ_RESPONSE_MESSAGE);

  mq_fd_response = mq_open (worker2dealer_name, O_RDONLY | O_CREAT | O_EXCL, 0600, &attr);
  if (mq_fd_response == (mqd_t)-1) {
      perror("Making the response queue failed");
      exit(1);
  }
  printf("Created Response queue: %s\n", worker2dealer_name);

}

pid_t create_client() {
  printf("Creating client process...\n");
  pid_t pid = fork();
  if (pid == 0) {
      execlp("./client", "client", client2dealer_name, NULL);
      perror("Failed to start client process");
      exit(1);
  }
  return pid;
}

void create_workers(int num_workers, const char *worker_program, const char *request_queue, const char *response_queue) {
  for (int i = 0; i < num_workers; i++) {
    printf("Creating worker %d for program %s...", i+1, worker_program);
    pid_t pid = fork();
    if (pid == 0) {
        execlp(worker_program, worker_program, request_queue, response_queue, NULL);
        perror("Failed to start worker process");
        exit(1);
    }
    else if (pid < 0) {
        perror("Failed to fork worker process");
        exit(1);
    }
    // Parent process continues to create other workers
  }
}

int main (int argc, char * argv[])
{
  if (argc != 1)
  {
    fprintf (stderr, "%s: invalid arguments\n", argv[0]);
  }
  
  // TODO:
    //  * create the message queues (see message_queue_test() in
    //    interprocess_basic.c)
  MQ_REQUEST_MESSAGE            req;
  MQ_RESPONSE_MESSAGE           rsp;
  create_message_queues();

    //  * create the child processes (see process_test() and
    //    message_queue_test())
  pid_t client_pid = create_client();
  create_workers(N_SERV1, "./worker_s1", dealer2worker1_name, worker2dealer_name);
  create_workers(N_SERV2, "./worker_s2", dealer2worker2_name, worker2dealer_name);

  
    //  * read requests from the Req queue and transfer them to the workers
    //    with the Sx queues
    //  * read answers from workers in the Rep queue and print them
    //  * wait until the client has been stopped (see process_test())
    //  * clean up the message queues (see message_queue_test())

    // Important notice: make sure that the names of the message queues
    // contain your goup number (to ensure uniqueness during testing)
  
  return (0);
}
