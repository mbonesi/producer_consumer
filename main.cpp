#include <iostream>
#include <queue>
#include <chrono>
#include <mutex>
#include <thread>
#include <future>

using namespace std;

inline int randomize(const int max)
{
    // NOTE: add 1 if you do not want 0 to apppear in the generated sequence
    return (/* SEE NOTE ABOVE => 1 + */ std::rand() / ((RAND_MAX + 1u) / max));
}

struct Queue
{
    std::mutex m;
    std::queue<int> images;

    std::condition_variable cv;
} acquisitions;

int producerThread(const int howMany)
{
    for (int i = 0; i < howMany; ++i)
    {
        int instance = randomize(5);
        std::cout << "producer at loop " << i << " producing: " << instance << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(15));
        std::lock_guard<std::mutex> lock(acquisitions.m);
        acquisitions.images.emplace(instance);
        acquisitions.cv.notify_one();
    }
    return howMany;
}

int consumerThread(const int howMany)
{
    int tot_consumed = 0;
    for (int i = 0; i < howMany; ++i)
    {
        int consumed;

        {   // limit time presence inside the mutex region
            std::unique_lock lock(acquisitions.m);
            acquisitions.cv.wait(lock, []{ return(acquisitions.images.size() > 0); });
            consumed = acquisitions.images.front();
            acquisitions.images.pop();
        }

        if(consumed != 0)
        {
            tot_consumed++;
            std::cout << "consumer at loop " << i << " consuming: " << consumed << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(randomize(40)));
        }
        else
        {
            std::cout << "consumer at loop " << i << " skipping consuming of value: " << consumed << std::endl;
        }
    }
    return tot_consumed;
}


int main()
{
    std::srand(std::time(nullptr));
    auto prod = std::async(std::launch::async, producerThread, 100);
    auto cons = std::async(std::launch::async, consumerThread, 100);
    int created = prod.get();
    int consumed = cons.get();
    std::cout << std::endl << "SUMMARY:" << std::endl << 
        "producer created: " << created << std::endl << 
        "consumer consumed: " << consumed << std::endl;
}