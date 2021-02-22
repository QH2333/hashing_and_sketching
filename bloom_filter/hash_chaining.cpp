#include <iostream>
#include <cstdlib>
#include <cstring>
#include <random>
#include <unordered_map>
#include <chrono>

#include "lookup3.h"
#include "hash_map.h"

void make_data(int* keys, int size)
{
    std::random_device rd;  //Will be used to obtain a seed for the random number engine
    std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
    std::uniform_int_distribution<> key_distrib(1, 1000);
 
    for (int n=0; n<size; ++n)
    {
        keys[n] = key_distrib(gen);
    }
}


int main()
{
    constexpr int data_size = 400000;
    int keys[data_size];
    hash_map<int, int> recorder_v1(64);
    std::unordered_map<int, int> recorder_v2;

    make_data(keys, data_size);

    auto start = std::chrono::steady_clock::now();
    for (int i = 0; i < data_size; i++)
    {
        if (!recorder_v1.insert(keys[i], 1)) (*recorder_v1.find(keys[i]))++;
    }
    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsed_seconds = end-start;
    std::cout << "Version 1: " << elapsed_seconds.count() << "s\n";
    
    start = std::chrono::steady_clock::now();
    for (int i = 0; i < data_size; i++)
    {
        if (!recorder_v2.insert({keys[i], 1}).second) recorder_v2[keys[i]]++;
    }
    end = std::chrono::steady_clock::now();
    elapsed_seconds = end-start;
    std::cout << "Version 2: " << elapsed_seconds.count() << "s\n";

    // for (int i = 0; i < data_size; i++)
    // {
    //     if (*recorder_v1.find(keys[i]) != recorder_v2[keys[i]])
    //         cout << keys[i] << " - " << *recorder_v1.find(keys[i]) << " - " << recorder_v2[keys[i]] << endl;
    //     else if (recorder_v2[keys[i]] > 465)
    //         cout << keys[i] << " - " << *recorder_v1.find(keys[i]) << " - " << recorder_v2[keys[i]] << endl;
    // }

    return 0;
}