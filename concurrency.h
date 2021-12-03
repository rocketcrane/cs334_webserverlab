#include <sys/stat.h>
#include "request.h"

int fill = 0;
int use = 0;
int count = 0;
int buffers;

//create lock and condition variables
pthread_cond_t empty, full;
pthread_mutex_t mutex;

struct request {
    int conn_fd;
    int file_size;
};

struct request* buffer; //create circular buffer of request objects to store conn_fd (FIFO only)

size_t get_file_size(const char* file_name) {
    struct stat file_stats;
    if(stat(file_name, &file_stats) != 0) {
        return 0;
    }
    return file_stats.st_size;
}

void put(int value) {
    char* fn[8192];//change to maxbuf?
    buffer[fill].conn_fd = value;
    //get_file_name(value, fn);
    //buffer[fill].file_size = get_file_size(fn);
    //printf("file size: %d\n", buffer[fill].file_size);
    fill = (fill + 1) % buffers;
    count++;
    printf("did put, count: %d\n", count);
}

int get() {
    int val = buffer[use].conn_fd;
    use = (use + 1) % buffers;
    count--;
    printf("new fd: %d\n", val);
    return val;
}

//worker thread function
void* worker(void *arg) {
    pthread_mutex_lock(&mutex); //lock section
    while(1) {
        while (count == 0) {
            printf ("before wait worker, count: %d\n", count);
            pthread_cond_wait(&full, &mutex);
        }
        printf("after wait\n");
        int fd = get();
        printf("FINAL FD: %d\n", fd);
        request_handle(fd);
        close_or_die(fd);
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