#include <stdio.h>
#include <string.h> //for scheduler string comparison
#include <pthread.h> //pthreads for concurrency
#include "request.h"
#include "io_helper.h"
#include "concurrency.h" //producer & consumer functions

char default_root[] = ".";

//
// ./wserver [-d <basedir>] [-p <portnum>] 
// 
int main(int argc, char *argv[]) {
    int c;
    char *root_dir = default_root;
    int port = 10000;			//default to port 10000
	int threads = 1;			//default to single thread
	char *schedalg = "FIFO";	//default to FIFO
    
    while ((c = getopt(argc, argv, "d:p:t:b:s:")) != -1) {
	switch (c) {
	case 'd':
	    root_dir = optarg;
	    break;
	case 'p':
	    port = atoi(optarg);
	    break;
	case 't':
		threads = atoi(optarg);
		break;
	case 'b':
		buffers = atoi(optarg);
		break;
	case 's':
		schedalg = optarg;
		break;
	default:
	    fprintf(stderr, "usage: wserver [-d basedir] [-p port] [-t threads] [-b buffers] [-s scheduler FIFO or SFF]\n");
	    exit(1);
	}
	}

	//update scheduler variable for worker threads
	//FIFO
	if (!strcmp(schedalg, "FIFO")) { //strcmp returns 0 if strings are equal
		scheduler = 0;
	}
	//SFF 
	else if (!strcmp(schedalg, "SFF")) {
		scheduler = 1;
	}
	//otherwise error out 
	else {
		printf("requested %s scheduler not implemented\n", schedalg);
		exit(1);
	}

    // run out of this directory
    chdir_or_die(root_dir);

	create_threads(threads);
	
    // now, get to work (listen for port)
    int listen_fd = open_listen_fd_or_die(port);

	buffer = (struct request*)malloc(sizeof(struct request) * buffers);	//allocate space for struct of requests
	
	//master thread loop
	pthread_mutex_lock(&mutex); //lock section
    while (1) {
		//if buffer is full then wait for empty signal
		while (count == buffers) {
			pthread_cond_wait(&empty, &mutex);
		}

		//(original) code to get connection file descriptor
		struct sockaddr_in client_addr;
		int client_len = sizeof(client_addr);
		int conn_fd = accept_or_die(listen_fd, (sockaddr_t *) &client_addr, (socklen_t *) &client_len);
		put(conn_fd); //put file descriptor in buffer		
		pthread_cond_signal(&full); //signal a worker thread
    }
	pthread_mutex_unlock(&mutex); //unlock section

    return 0;
}


    


 
