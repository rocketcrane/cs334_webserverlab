#include <stdio.h>
#include <pthread.h> //pthreads for concurrency
#include "request.h"
#include "io_helper.h"

char default_root[] = ".";

//
// ./wserver [-d <basedir>] [-p <portnum>] 
// 
int main(int argc, char *argv[]) {
    int c;
    char *root_dir = default_root;
    int port = 10000;
	int threads = 1;
	int buffers = 1;
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

	//create_threads(threads);
	
    // now, get to work (listen for port)
    int listen_fd = open_listen_fd_or_die(port);
	
	//create lock and condition variables
	pthread_cond_t empty, fill;
	pthread_mutex_t mutex;
	
	int fd_buffer[buffers]; //create circular buffer to store conn_fd (FIFO only)
	
	//variables for producer loop
	int fill = 0;
	int use = 0;
	int count = 0;
	
    while (1) {
		//producer loop
		for (int i = 0; i < buffers; i++) {

			//if buffer is full then wait for empty signal
			while (count == buffers) {
				pthread_cond_wait(&empty, &mutex);
			}
			//put:
			struct sockaddr_in client_addr;
			int client_len = sizeof(client_addr);
			int conn_fd = accept_or_die(listen_fd, (sockaddr_t *) &client_addr, (socklen_t *) &client_len);
			fd_buffer[i] = conn_fd;
			fill = (fill + 1) % buffers;
			
		}

		
		//request_handle(conn_fd);
		buffer[fill] = conn_fd;


		close_or_die(conn_fd);
    }
    return 0;
}


    


 
