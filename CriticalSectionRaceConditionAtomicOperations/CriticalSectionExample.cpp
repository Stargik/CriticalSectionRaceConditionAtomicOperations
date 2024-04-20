#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <chrono>
#include <barrier>
#include <unistd.h>

using namespace std;
mutex locker;
condition_variable cv;
std::barrier b(2);
int iterationt_count = 1e9;
int threadCount = 2;

void inc(int &t, int count){
    for (int i = 0; i < count; i++) {
        t = t + 1;
    }
}

void inc_with_mutex(int &t, int count){
    for (int i = 0; i < count; i++) {
        locker.lock();
        t = t + 1;
        locker.unlock();
    }
}

void inc_with_atomic(atomic<int> &t, int count){
    for (int i = 0; i < count; i++) {
        t++;
    }
}
  
void parallel_inc(atomic<int> &t, int count){
    for (int i = 0; i < count; i++) {
        int x = t.load();
        ++x;
        b.arrive_and_wait();
        t.store(x);
    }
}

int main(int argc, char *argv[]) {
    if (argc == 2) {
        iterationt_count = stoi(argv[1]);
    }
    int t1 = 0;
    vector<thread> threads1;
    for (int i = 0; i < threadCount; ++i) {
        threads1.push_back(thread(inc, ref(t1), iterationt_count));
    }
    auto start = chrono::high_resolution_clock::now();
    for (auto& thread : threads1) {
        thread.join();
    }
    cout << "t1 (race condition): " << t1 << endl;
    auto stop = chrono::high_resolution_clock::now();
    cout << "Execution time (race condition): ";
    chrono::duration<double, std::milli> duration = stop - start;
    cout << duration.count() << endl;
    
    cout << endl;
    
    atomic<int> t2(0);
    vector<thread> threads2;
    for (int i = 0; i < threadCount; ++i) {
        threads2.push_back(thread(inc_with_atomic, ref(t2), iterationt_count));
    }
    start = chrono::high_resolution_clock::now();
    for (auto& thread : threads2) {
        thread.join();
    }
    stop = chrono::high_resolution_clock::now();
    cout << "t2 (with atomic): " << t2 << endl;
    cout << "Execution time (with atomic): ";
    duration = stop - start;
    cout << duration.count() << endl;
    
    cout << endl;
    int t3 = 0;
    vector<thread> threads3;
    for (int i = 0; i < threadCount; ++i) {
        threads3.push_back(thread(inc_with_mutex, ref(t3), iterationt_count));
    }
    start = chrono::high_resolution_clock::now();
    for (auto& thread : threads3) {
        thread.join();
    }
    stop = chrono::high_resolution_clock::now();
    cout << "t3 (with mutex): " << t3 << endl;
    cout << "Execution time (with mutex): ";
    duration = stop - start;
    cout << duration.count() << endl;
    
    cout << endl;
    
    atomic<int> t0(0);
    vector<thread> threads0;
    for (int i = 0; i < threadCount; ++i) {
        threads0.push_back(thread(parallel_inc, ref(t0), iterationt_count));
    }
    start = chrono::high_resolution_clock::now();
    for (auto& thread : threads0) {
        thread.join();
    }
    cout << "t0 (parallel): " << t0 << endl;
    stop = chrono::high_resolution_clock::now();
    cout << "Execution time (parallel): ";
    duration = stop - start;
    cout << duration.count() << endl;
    
}
