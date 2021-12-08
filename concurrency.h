#include <sys/stat.h>
#include "request.h"

#define MAXBUF (8192) //same as request.c

int fill = 0;       //buffer index of request, for adding to buffer and getting request info for struct
int use = 0;        //buffer index of request, for use() when call request
int count = 0;      //number of requests in buffer
int buffers;        //number of slots in buffer

int scheduler = 0; //scheduler chosen: FIFO = 0, SFF = 1;

//create lock and condition variables
pthread_cond_t empty, full;
pthread_mutex_t mutex;

//holds everything required to process request
struct request {
    int conn_fd; //file descriptor
    int file_size;
    int is_static; //whether file is static or not
    int not_in_directory; //whether file is in directory or not

    //for request handling, abstracted away from request_handle
    char buf[MAXBUF];
    char method[MAXBUF];
    char uri[MAXBUF];
    char version[MAXBUF];
    char filename[MAXBUF];
    char cgiargs[MAXBUF];
};

struct request* buffer; //create circular buffer of request objects to store conn_fd (FIFO takes items in order, SFF takes items by file size)

//get filename-- if file is not in working directory return 1, otherwise return 0
int get_file_name(int fd) { 
    //use FILL because this is called from PUT, before FILL is updated
    readline_or_die(fd, buffer[fill].buf, MAXBUF);
    sscanf(buffer[fill].buf, "%s %s %s", buffer[fill].method, buffer[fill].uri, buffer[fill].version);
    //errors out if requesting a file not in directory 
    if (strstr(buffer[fill].uri, "..") != NULL) {
        request_error(buffer[fill].conn_fd, buffer[fill].filename, "403", "Forbidden", "NO ACCESS TO THIS DIRECTORY. FBI: YOU HAVE VIOLATED TERMS & CONDITIONS. FINE OF UP TO $250,000");
        return 1;
    }
    buffer[fill].is_static = request_parse_uri(buffer[fill].uri, buffer[fill].filename, buffer[fill].cgiargs);
    return 0;
}

//get file size from filename
size_t get_file_size(const char* file_name) {
    struct stat file_stats;
    if(stat(file_name, &file_stats) != 0) {
        return 0;
    }
    return file_stats.st_size;
}

void put(int value) {           //given file descriptor
    buffer[fill].conn_fd = value;

    //------code to get file size------
    buffer[fill].not_in_directory = get_file_name(value);
    //only check file size if file is in directory
    if(!buffer[fill].not_in_directory) {
        buffer[fill].file_size = get_file_size(buffer[fill].filename);
    } else { //otherwise put a minimal file size so SFF scheduler doesn't trip up
        buffer[fill].file_size = 1;
    }
    printf("file size: %d\n", buffer[fill].file_size);
    
    //update fill and count
    fill = (fill + 1) % buffers;
    count++;
}

struct request get() {
    //-----------------------------FIFO-----------------------------
    //take from the buffer in order
    struct request local_request = buffer[use];
    //-----------------------------SFF-----------------------------
    if(scheduler == 1) {
        //walk through array to see which file size is smallest but non zero and save that request to local_request
        //then zero that item's file size to prevent it from being used by another thread
        int index = -1; //initialize index to -1 (no files found yet)
        for (int i = 0; i < buffers; i++) {
            
            //item is empty, skip to next loop
            if(buffer[i].file_size == 0) {
                continue;
            }
            //save the first file found
            else if(index == -1) {
                index = i;
            }
            //file size is smaller than the previous found file, use this one instead
            else if(buffer[i].file_size < buffer[index].file_size) {
                index = i;
            }
        }
        local_request = buffer[index]; //save request with smallest file size
        buffer[index].file_size = 0; //zero out the file size of that request (prevent it being used by another thread)
    }
    use = (use + 1) % buffers;      //move use to next slot in buffer
    count--;                        //decrement count of requests in buffer
    return local_request;
}

//-----------------------------worker thread-----------------------------
void* worker(void *arg) {
    pthread_mutex_lock(&mutex); //lock section
    while(1) {
        while (count == 0) {
            pthread_cond_wait(&full, &mutex); //waits for signal when buffer empty
        }
        struct request local_request = get();
        request_handle(local_request.conn_fd, local_request.buf, local_request.method, 
                        local_request.uri, local_request.version, local_request.filename, 
                        local_request.cgiargs, local_request.is_static, local_request.not_in_directory); //actually handles the request
        close_or_die(local_request.conn_fd);
        pthread_cond_signal(&empty); //signal master thread
    }
    pthread_mutex_unlock(&mutex); //unlock section
}

//create worker threads
void create_threads(int threads) {
    pthread_t id[threads]; //create pthread handles

    for (int i = 0; i < threads; i++) {
        pthread_create(&id[i], NULL, worker, NULL);
        printf("created thread %d\n", i);
    }
}