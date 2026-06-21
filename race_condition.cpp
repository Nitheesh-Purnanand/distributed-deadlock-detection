#include <iostream>
#include <thread>
#include <vector>

int counter = 0;

void incrementCounter(int iterations) {
    for (int i = 0; i < iterations; ++i) {
        counter++;  // NOT atomic — read-modify-write race condition here
    }
}

int main() {
    const int NUM_THREADS = 3;
    const int ITERATIONS_PER_THREAD = 100000;

    std::vector<std::thread> threads;
    for (int i = 0; i < NUM_THREADS; ++i) {
        threads.emplace_back(incrementCounter, ITERATIONS_PER_THREAD);
    }

    // Wait for all threads to finish
    for (auto& t : threads) {
        t.join();
    }

    int expected = NUM_THREADS * ITERATIONS_PER_THREAD;
    std::cout << "Expected value: " << expected << std::endl;
    std::cout << "Actual value:   " << counter << std::endl;

    if (counter != expected) {
        std::cout << "Race condition detected! Lost "
                   << (expected - counter) << " increments." << std::endl;
    } else {
        std::cout << "No race condition observed this run (got lucky — "
                      "try running again)." << std::endl;
    }

    return 0;
}