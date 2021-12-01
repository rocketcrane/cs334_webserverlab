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

void put(int value) {
    buffer[fill].conn_fd = value;
    //buffer[fill].file_size = size of file
    fill = (fill + 1) % buffers;
    count++;
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
    while(1) {
        pthread_mutex_lock(&mutex); //lock section

        while (count == 0) {
            pthread_cond_wait(&full, &mutex);
        }
        pthread_mutex_unlock(&mutex); //unlock section
        pthread_cond_signal(&empty); //signal producer (master) thread

        request_handle(get());
    }
}

//create consumer (worker) threads
void create_threads(int threads) {
    pthread_t id[threads]; //create pthread handles

    for (int i = 0; i < threads; i++) {
        pthread_create(&id[i], NULL, worker, NULL);
    }
}