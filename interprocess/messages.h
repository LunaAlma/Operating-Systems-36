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

#ifndef MESSAGES_H
#define MESSAGES_H

typedef struct
{
    int                     RequestID;
    int                     ServiceID;
    int                     data;
} MQ_REQUEST_MESSAGE;

typedef struct
{
    int                     ResponseID;
    int                     result;
} MQ_RESPONSE_MESSAGE;

#endif
