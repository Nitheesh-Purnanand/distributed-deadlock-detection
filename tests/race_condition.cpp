#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
using namespace std;
//adding atomic to fix the race condition
// #include <atomic>
// atomic<int> counter = 0;
int counter = 0;
mutex counter_mutex;

void incrementCounter(int iterations) {
    for (int i = 0; i < iterations; ++i) {
        lock_guard<mutex> lock(counter_mutex);
        counter++; 
    }
}

int main() {
    const int NUM_THREADS = 3;
    const int ITERATIONS_PER_THREAD = 100000;

    vector<thread> threads;
    for (int i = 0; i < NUM_THREADS; ++i) {
        threads.emplace_back(incrementCounter, ITERATIONS_PER_THREAD);
    }

    // Wait for all threads to finish
    for (auto& t : threads) {
        t.join();
    }

    int expected = NUM_THREADS * ITERATIONS_PER_THREAD;
    cout << "Expected value: " << expected << endl;
    cout << "Actual value:   " << counter << endl;

    if (counter != expected) {
        cout << "Race condition detected! Lost "
                   << (expected - counter) << " increments." << endl;
    } else {
        cout << "No race condition observed this run (got lucky — "
                      "try running again)." << endl;
    }

    return 0;
}