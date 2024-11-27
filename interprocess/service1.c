/* 
 * Operating Systems (2INCO) Practical Assignment
 * Interprocess Communication
 *
 * Service 1 implementation
 *
 */

#include "service1.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>      // for perror()
#include <unistd.h>     // for getpid()
#include <mqueue.h>     // for mq functions
#include <string.h>     // for memset
#include "messages.h"


int service(int data) {
	if (data <= 1)
		return data;

	int n1 = 0, n2 = 1, n3;
	for (int i = 1; i < data; ++i) {
		n3 = n1 + n2;
		n1 = n2;
		n2 = n3;
	}

	return n3;
}

