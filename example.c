
// Global variable
static int global_counter = 0;

// Function to get the current value of the global counter
int get_counter() {
    return global_counter;
}

// Function to increment the global counter
void increment_counter(int value) {
    global_counter += value;
}

// Function to reset the global counter
void reset_counter() {
    global_counter = 0;
}