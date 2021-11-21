int fill = 0;
int use = 0;
int count = 0;

void put(int value, int buffer[], int buffers) {
    buffer[fill] = value;
    fill = (fill + 1) % buffers;
    count++;
}

int get(int buffer[], int buffers) {
    int val = buffer[use];
    use = (use + 1) % buffers;
    count--;
    return val;
}