int fill = 0;
int use = 0;
int count = 0;

//create lock and condition variables
pthread_cond_t empty, full;
pthread_mutex_t mutex;

struct request {
    int conn_fd;
    int file_size;
};

void put(int value, struct request buffer[], int buffers) {
    buffer[fill].conn_fd = value;
    //buffer[fill].file_size = size of file
    fill = (fill + 1) % buffers;
    count++;
}

int get(struct request buffer[], int buffers) {
    int val = buffer[use].conn_fd;
    use = (use + 1) % buffers;
    count--;
    return val;
}

//create consumer (worker) threads
void create_threads(int threads) {
    pthread_t id[threads]; //create pthread handles

    for (int i; i = 0; i < threads) {
        pthread_create(&id[i], NULL, worker, NULL);
    }
}

//worker thread function
void worker(struct request buffer[], int buffers) {
    while(1) {
        pthread_mutex_lock(&mutex); //lock section

        while (count == 0) {
            pthread_cond_wait(&full, &mutex);
        }

        int conn_fd = get(buffer, buffers);
        pthread_mutex_unlock(&mutex); //unlock section
        pthread_cond_signal(&empty); //signal producer (master) thread

        request_handle(conn_fd);
    }
}