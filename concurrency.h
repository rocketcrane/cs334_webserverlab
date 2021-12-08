#include <sys/stat.h>
#include "request.h"

#define MAXBUF (8192) //same as request.c

//concurrency variables
int fill = 0;
int use = 0;
int count = 0;
int buffers;

int scheduler = 0; //scheduler chosen: FIFO = 0, SFF = 1;

//create lock and condition variables
pthread_cond_t empty, full;
pthread_mutex_t mutex;

//holds everything required to process request
struct request {
    int conn_fd;
    int file_size;
    int is_static;

    //for request handling, abstracted away from request_handle
    char buf[MAXBUF];
    char method[MAXBUF];
    char uri[MAXBUF];
    char version[MAXBUF];
    char filename[MAXBUF];
    char cgiargs[MAXBUF];
};

struct request* buffer; //create circular buffer of request objects to store conn_fd (FIFO takes items in order, SFF takes items by file size)

//get filename
int get_file_name(int fd) { 
    //use FILL because this is called from PUT, before FILL is updated
    readline_or_die(fd, buffer[fill].buf, MAXBUF);
    sscanf(buffer[fill].buf, "%s %s %s", buffer[fill].method, buffer[fill].uri, buffer[fill].version);
    //printf("uri[0-1]: %s\n", buffer[fill].uri[3]);
    //run out of this directory
    if (strstr(buffer[fill].uri, "..") != NULL) {
        printf("NO ACCESS TO THIS DIRECTORY. FBI: YOU HAVE VIOLATED TERMS & CONDITIONS. FINE OF UP TO $250,000.\n");
        exit(1);
    }
    buffer[fill].is_static = request_parse_uri(buffer[fill].uri, buffer[fill].filename, buffer[fill].cgiargs);
    //sprintf(buffer[fill].filename, ".%s", buffer[fill].uri);
    return 1;
}

//get file size from filename
size_t get_file_size(const char* file_name) {
    struct stat file_stats;
    if(stat(file_name, &file_stats) != 0) {
        return 0;
    }
    return file_stats.st_size;
}

void put(int value) {
    buffer[fill].conn_fd = value;

    //code to get file size
    get_file_name(value);
    buffer[fill].file_size = get_file_size(buffer[fill].filename);
    printf("file size: %d\n", buffer[fill].file_size);
    
    //update fill and count
    fill = (fill + 1) % buffers;
    count++;
    //printf("did put, count: %d\n", count);
}

struct request get() {
    //FIFO
    struct request local_request = buffer[use];
    //SFF
    if(scheduler == 1) {
        //walk through array to see which file size is smallest but non zero and save that request to local_request
        //then zero that item's file size to prevent it from being used by another thread
        int index = -1; //initialize index to -1 (no non-zero size files)
        for (int i = 0; i < buffers; i++) {
            
            //if this item is empty don't bother
            if(buffer[i].file_size == 0) {
                continue;
            }
            //else if index is still -1 then save the first file
            else if(index == -1) {
                index = i;
            }
            //else if the file size is smaller than the previous one, use this one instead
            else if(buffer[i].file_size < buffer[index].file_size) {
                index = i;
            }
        }
        printf("SFF index is %d\n", index);
        local_request = buffer[index]; //save smallest file request to local_request
        buffer[index].file_size = 0; //zero out the file size of that request
    }

    use = (use + 1) % buffers;
    count--;
    //printf("new fd: %d\n", local_request.conn_fd);
    return local_request;
}

//worker thread function
void* worker(void *arg) {
    pthread_mutex_lock(&mutex); //lock section
    while(1) {
        while (count == 0) {
            //printf ("before wait worker, count: %d\n", count);
            pthread_cond_wait(&full, &mutex);
        }
        //printf("after wait\n");
        struct request local_request = get();
        printf("FINAL FD: %d\n", local_request.conn_fd);
        request_handle(local_request.conn_fd, local_request.buf, local_request.method, local_request.uri, local_request.version, local_request.filename, local_request.cgiargs, local_request.is_static); //actually handles the request
        close_or_die(local_request.conn_fd);
        pthread_cond_signal(&empty); //signal producer (master) thread
    }
    pthread_mutex_unlock(&mutex); //unlock section
}

//create consumer (worker) threads
void create_threads(int threads) {
    pthread_t id[threads]; //create pthread handles

    for (int i = 0; i < threads; i++) {
        pthread_create(&id[i], NULL, worker, NULL);
        printf("created thread %d\n", i);
    }
}