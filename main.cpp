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

int producerThread(const int howMany, const int production_factor)
{
    for (int i = 0; i < howMany; ++i)
    {
        int image;
        std::this_thread::sleep_for(std::chrono::milliseconds(15));
        
        {   // limit time presence inside the mutex region
            // also obey tha RAII principle (on the lock_guard object)
            std::lock_guard<std::mutex> lock(acquisitions.m);
            image = acquisitions.images.emplace(randomize(production_factor));
        }

        acquisitions.cv.notify_one();
        std::cout << "producer at loop " << i << " producing value: " << image << std::endl;
    }
    return howMany;
}

int consumerThread(const int howMany)
{
    int tot_consumed = 0;
    for (int i = 0; i < howMany; ++i)
    {
        int image;

        {   // limit time presence inside the mutex region
            // also obey tha RAII principle (on the lock_guard object
            std::unique_lock lock(acquisitions.m);
            acquisitions.cv.wait(lock, []{ return(acquisitions.images.size() > 0); });
            image = acquisitions.images.front();
            acquisitions.images.pop();
        }

        if(image != 0)
        {
            tot_consumed++;
            std::cout << "consumer at loop " << i << " consuming value: " << image << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(randomize(40)));
        }
        else
        {
            std::cout << "consumer at loop " << i << " skipping consuming of value: " << image << std::endl;
        }
    }
    return tot_consumed;
}

int main(int argc, char* argv[])
{
    const int FACTOR = 10;
    int TEST_SIZE = 100;
    
    if(argc > 1)
    {
        TEST_SIZE = std::stoi(argv[1]);
    }

    std::srand(std::time(nullptr));
    auto prod = std::async(std::launch::async, producerThread, TEST_SIZE, FACTOR);
    auto cons = std::async(std::launch::async, consumerThread, TEST_SIZE);
    int created = prod.get();
    int consumed = cons.get();
    std::cout << std::endl << "SUMMARY:" << std::endl << 
        "producer created: " << created << std::endl << 
        "consumer consumed: " << consumed << std::endl;
}
