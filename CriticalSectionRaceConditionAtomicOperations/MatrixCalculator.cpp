#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <iomanip>
#include <chrono>

using namespace std;
mutex locker;

vector<vector<double>> fill_matrix(int rows, int cols){
    vector<vector<double>> m(rows, vector<double>(cols));
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            m[i][j] = rand() % 100;
        }
    }
    return m;
}

void print_matrix(vector<vector<double>> m){
    for (const auto& row : m) {
        for (int elem : row) {
            cout << setw(8) << elem << " ";
        }
        cout << endl;
    }
}

void calc_element(double& res, vector<vector<double>> A, vector<vector<double>> B, int rows, int cols, int i, int j, int printMode){
    res = 0;
    for (int k = 0; k < B.size(); ++k) {
        res += A[i][k] * B[k][j];
    }
    if (printMode) {
        locker.lock();
        cout << "[" << i << ", " << j << "] = " << res << endl;
        locker.unlock();
    }
}

void calc_vectors(vector<vector<double>> &resM, vector<vector<double>> A, vector<vector<double>> B, int rows, int cols, int start_raw, int vectors_count, int printMode){
    for (int j = 0; j < vectors_count; ++j) {
        for (int i = 0; i < cols; ++i) {
            calc_element(resM[start_raw + j][i], A, B, rows, cols, start_raw + j, i, printMode);
        }
    }
    
}

vector<vector<double>> transpose_matrix(vector<vector<double>> matrix, int cols){
    vector<vector<double>> transposeM(cols, vector<double>(matrix.size()));
    for (int i = 0; i < cols; ++i) {
        for (int j = 0; j < matrix.size(); ++j) {
            transposeM[i][j] = matrix[j][i];
        }
    }
    return transposeM;
}

vector<vector<double>> multiple_matrix(vector<vector<double>> A, vector<vector<double>> B, int rows, int cols, int printMode){
    vector<vector<double>> resM(rows, vector<double>(cols));
    vector<thread> threads;
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            calc_element(resM[i][j], A, B, rows, cols, i, j, printMode);
        }
    }
    for (auto& thread : threads) {
        thread.join();
    }
    
    return resM;
}

vector<vector<double>> multiple_matrix_parallel_by_element(vector<vector<double>> A, vector<vector<double>> B, int rows, int cols, int printMode){
    vector<vector<double>> resM(rows, vector<double>(cols));
    vector<thread> threads;
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            threads.push_back(thread(calc_element, ref(resM[i][j]), A, B, rows, cols, i, j, printMode));
        }
    }
    for (auto& thread : threads) {
        thread.join();
    }
    
    return resM;
}

vector<vector<double>> multiple_matrix_parallel_by_vector(vector<vector<double>> A, vector<vector<double>> B, int rows, int cols, int threadCount, int printMode){
    int vectorsCount = rows / threadCount;
    int vectorsCountReminder = rows % threadCount;
    vector<vector<double>> resM(rows, vector<double>(cols));
    vector<thread> threads;
    for (int i = 0; i < threadCount; ++i) {
        threads.push_back(thread(calc_vectors, ref(resM), A, B, rows, cols, i*vectorsCount, vectorsCount, printMode));
    }
    for (auto& thread : threads) {
        thread.join();
    }
    for (int i = 0; i < vectorsCountReminder; ++i) {
        calc_vectors(resM, A, B, rows, cols, threadCount*vectorsCount + i, 1, printMode);
    }
    
    return resM;
}



int main(int argc, char *argv[]) {
    int n = 4;
    int m = 4;
    int k = 4;
    int threadCount = 4;
    int printMode = 1;
    if (argc == 6) {
        threadCount = stoi(argv[1]);
        n = stoi(argv[2]);
        m = stoi(argv[3]);
        k = stoi(argv[4]);
        printMode = stoi(argv[5]);
    }
    
    srand((unsigned)time(0));
    vector<vector<double>> A = fill_matrix(n, m);
    vector<vector<double>> B = fill_matrix(m, k);
    if (printMode) {
        cout << "Matrix A:" << endl;
        print_matrix(A);
        cout << endl;
        cout << "Matrix B:" << endl;
        print_matrix(B);
        cout << endl;
        cout << "Calculation:" << endl;
    }
    auto start = chrono::high_resolution_clock::now();
    vector<vector<double>> R = multiple_matrix_parallel_by_element(A, B, n, k, printMode);
    auto stop = chrono::high_resolution_clock::now();
    if (printMode) {
        cout << endl;
        cout << "Result:" << endl;
        print_matrix(R);
        cout << endl;
    }
    cout << "Execution time (thread per element): ";
    chrono::duration<double, std::milli> duration = stop - start;
    cout << duration.count() << endl;
    for (int i = 1; i <= threadCount; i += 1) {
        if (printMode) {
            cout << endl;
            cout << "Calculation:" << endl;
        }
        start = chrono::high_resolution_clock::now();
        R = multiple_matrix_parallel_by_vector(A, B, n, k, i, printMode);
        stop = chrono::high_resolution_clock::now();
        if (printMode) {
            cout << endl;
            cout << "Result:" << endl;
            print_matrix(R);
            cout << endl;
        }
        cout << "Execution time with " << setw(3) << i << " threads: ";
        duration = stop - start;
        cout << duration.count() << endl;
    }
    if (printMode) {
        cout << endl;
        cout << "Calculation:" << endl;
    }
    start = chrono::high_resolution_clock::now();
    R = multiple_matrix(A, B, n, k, printMode);
    stop = chrono::high_resolution_clock::now();
    if (printMode) {
        cout << endl;
        cout << "Result:" << endl;
        print_matrix(R);
        cout << endl;
    }
    cout << "Execution time (synchronously): ";
    duration = stop - start;
    cout << duration.count() << endl;
}
