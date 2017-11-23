/*
 * main.cpp
 *
 *  Created on: 23 Nov 2017
 *      Author: betran
 */
#include <iostream>
#include <array>
#include <cassert>
#include <chrono>
#include <string>
#include "ThreadPool.h"

using namespace std;

//#define NO_FALSE_SHARING
const long num_of_threads = 25;
const long chunk_size = 100000;
const long total_elements = num_of_threads * chunk_size;


std::string time() {
    static auto start = std::chrono::steady_clock::now();
    std::chrono::duration<double> d = std::chrono::steady_clock::now() - start;
    return "[" + std::to_string(d.count()) + "s]";
}

int main()
{
    std::array<long, total_elements> data_array;
    std::array<long, num_of_threads> result_array;

    std::cout << time() << "Starting Program" << std::endl;

    // Create thread pool with 25 worker threads
    ThreadPool pool(num_of_threads);
    // Count the number of Odd elements to check if threads are counting correctly.
    long compare_count = 0;
    // Initialise array with random numbers
    for (long i=0; i<total_elements; i++){

        long random_value  = rand() % 1000;
        data_array[i] = random_value;

        if(random_value%2 != 0 ){
            compare_count++;
        }
    }
    std::cout << time() << "Array Initialised" << std::endl;

    // Enqueue threads to pool. Threads count number of odd elements from each chunk.
    std::vector< std::future<long> > results;
    for(long i = 0; i < num_of_threads; ++i) {
        results.emplace_back(
            pool.enqueue([&,i] {
            long count = 0;
                for(long j=0; j<chunk_size; j++) {
                    if(data_array[ (i*chunk_size) + j ]%2 != 0)
#ifdef NO_FALSE_SHARING
                        // Updating result in local stack memory.
                        count++;
#else
                        // Updating result in a shared memory array.
                        result_array[i]++;
#endif
                }
#ifdef NO_FALSE_SHARING
                result_array[i] = count; // Updating result in a shared memory array.
#endif
                return count;
            })
        );
    }

    std::cout << time() << "Threads added to Pool, Starting..." << std::endl;
    for(auto && result: results)
        result.get();
    std::cout << time() << "Threads ran to completion" << std::endl;


    int total_odd_count = 0;
    for(long i=0; i<num_of_threads; i++)
        total_odd_count += result_array[i];

    assert(compare_count == total_odd_count);
    cout <<"Total number of Odd elements in the array=" << total_odd_count << std::endl;

    return 0;

}
