#include <stdio.h>
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
    int port = 10000;
	int threads = 1;
	char *schedalg = "FIFO";
    
    while ((c = getopt(argc, argv, "d:p:t:b:s:")) != -1)
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
	    fprintf(stderr, "usage: wserver [-d basedir] [-p port]\n");
	    exit(1);
	}

    // run out of this directory
    chdir_or_die(root_dir);

	create_threads(threads);

	char* scheduler = schedalg; //choose scheduler (not used)
	
    // now, get to work (listen for port)
    int listen_fd = open_listen_fd_or_die(port);

	buffer = (struct request*)malloc(sizeof(struct request) * buffers);
	
	//master thread loop
	//only FIFO (currently)
    while (1) {
		pthread_mutex_lock(&mutex); //lock section

		//if buffer is full then wait for empty signal
		while (count == buffers) {
			pthread_cond_wait(&empty, &mutex);
		}

		//(original) code to get connection file descriptor
		struct sockaddr_in client_addr;
		int client_len = sizeof(client_addr);
		int conn_fd = accept_or_die(listen_fd, (sockaddr_t *) &client_addr, (socklen_t *) &client_len);
		printf("original fd: %d\n", conn_fd);
		put(conn_fd); //put file descriptor in buffer		
		pthread_cond_signal(&full); //signal a worker thread
		pthread_mutex_unlock(&mutex); //unlock section

		close_or_die(conn_fd);
    }
    return 0;
}


    


 
