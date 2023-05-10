#define CATCH_CONFIG_MAIN

#include "catch2/catch.hpp"

#include "include/ThreadSafeMemOp.hpp"
#include "include/Debug.hpp"

#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <list>


TEST_CASE( "Initially there's no blocks", "[all]" ) {
    
    auto blocks = tsmo::debug::getBlocksInfo();
    REQUIRE(blocks.empty());

}


TEST_CASE( "First malloc would request approx. 10*(<required size> + <header size>)", "[all]" ) {
    
    void* test = tsmo::malloc(30);

    auto actualBlocks1 = tsmo::debug::getBlocksInfo();
    std::vector<tsmo::debug::BlockInfo> expectedBlocks1 {
        {30, false},
        {530, true}
    };

    REQUIRE_THAT(actualBlocks1, Catch::Matchers::Equals(expectedBlocks1));


    tsmo::free(test);

    auto actualBlocks2 = tsmo::debug::getBlocksInfo();
    std::vector<tsmo::debug::BlockInfo> expectedBlocks2 {
        {592, true}
    };

    REQUIRE_THAT(actualBlocks2, Catch::Matchers::Equals(expectedBlocks2));
}


TEST_CASE( "Test malloc 2 blocks", "[all]" ) {
    
    void* test1 = tsmo::malloc(30);
    void* test2 = tsmo::malloc(10);

    auto actualBlocks1 = tsmo::debug::getBlocksInfo();
    std::vector<tsmo::debug::BlockInfo> expectedBlocks1 {
        {30, false},
        {10, false},
        {488, true}
    };

    REQUIRE_THAT(actualBlocks1, Catch::Matchers::Equals(expectedBlocks1));


    tsmo::free(test1);
    tsmo::free(test2);

    auto actualBlocks2 = tsmo::debug::getBlocksInfo();
    std::vector<tsmo::debug::BlockInfo> expectedBlocks2 {
        {592, true}
    };

    REQUIRE_THAT(actualBlocks2, Catch::Matchers::Equals(expectedBlocks2));
}


TEST_CASE( "Test malloc 5 blocks", "[all]" ) {
    
    void* test1 = tsmo::malloc(30);
    void* test2 = tsmo::malloc(10);
    void* test3 = tsmo::malloc(100);
    void* test4 = tsmo::malloc(10);
    void* test5 = tsmo::malloc(50);

    auto actualBlocks1 = tsmo::debug::getBlocksInfo();
    std::vector<tsmo::debug::BlockInfo> expectedBlocks1 {
        {30, false},
        {10, false},
        {100, false},
        {10, false},
        {50, false},
        {232, true}
    };

    REQUIRE_THAT(actualBlocks1, Catch::Matchers::Equals(expectedBlocks1));


    tsmo::free(test1);
    tsmo::free(test2);
    tsmo::free(test3);
    tsmo::free(test4);
    tsmo::free(test5);

    auto actualBlocks2 = tsmo::debug::getBlocksInfo();
    std::vector<tsmo::debug::BlockInfo> expectedBlocks2 {
        {592, true}
    };

    REQUIRE_THAT(actualBlocks2, Catch::Matchers::Equals(expectedBlocks2));
}


TEST_CASE( "Test merge left after free", "[all]" ) {
    
    void* test1 = tsmo::malloc(30);
    void* test2 = tsmo::malloc(10);
    void* test3 = tsmo::malloc(100);
    void* test4 = tsmo::malloc(10);
    void* test5 = tsmo::malloc(50);

    auto actualBlocks1 = tsmo::debug::getBlocksInfo();
    std::vector<tsmo::debug::BlockInfo> expectedBlocks1 {
        {30, false},
        {10, false},
        {100, false},
        {10, false},
        {50, false},
        {232, true}
    };

    REQUIRE_THAT(actualBlocks1, Catch::Matchers::Equals(expectedBlocks1));


    tsmo::free(test1);
    tsmo::free(test2);
    tsmo::free(test3);

    auto actualBlocks2 = tsmo::debug::getBlocksInfo();
    std::vector<tsmo::debug::BlockInfo> expectedBlocks2 {
        {204, true},
        {10, false},
        {50, false},
        {232, true}
    };

    REQUIRE_THAT(actualBlocks2, Catch::Matchers::Equals(expectedBlocks2));


    tsmo::free(test4);
    tsmo::free(test5);

    auto actualBlocks3 = tsmo::debug::getBlocksInfo();
    std::vector<tsmo::debug::BlockInfo> expectedBlocks3 {
        {592, true}
    };

    REQUIRE_THAT(actualBlocks3, Catch::Matchers::Equals(expectedBlocks3));
}


TEST_CASE( "Test merge right after free", "[all]" ) {
    
    void* test1 = tsmo::malloc(30);
    void* test2 = tsmo::malloc(10);
    void* test3 = tsmo::malloc(100);
    void* test4 = tsmo::malloc(10);
    void* test5 = tsmo::malloc(50);

    auto actualBlocks1 = tsmo::debug::getBlocksInfo();
    std::vector<tsmo::debug::BlockInfo> expectedBlocks1 {
        {30, false},
        {10, false},
        {100, false},
        {10, false},
        {50, false},
        {232, true}
    };

    REQUIRE_THAT(actualBlocks1, Catch::Matchers::Equals(expectedBlocks1));


    tsmo::free(test4);
    tsmo::free(test5);

    auto actualBlocks2 = tsmo::debug::getBlocksInfo();
    std::vector<tsmo::debug::BlockInfo> expectedBlocks2 {
        {30, false},
        {10, false},
        {100, false},
        {356, true}
    };

    REQUIRE_THAT(actualBlocks2, Catch::Matchers::Equals(expectedBlocks2));


    tsmo::free(test1);
    tsmo::free(test2);
    tsmo::free(test3);

    auto actualBlocks3 = tsmo::debug::getBlocksInfo();
    std::vector<tsmo::debug::BlockInfo> expectedBlocks3 {
        {592, true}
    };

    REQUIRE_THAT(actualBlocks3, Catch::Matchers::Equals(expectedBlocks3));
}


TEST_CASE( "Test merge 2 way after free", "[all]" ) {
    
    void* test1 = tsmo::malloc(30);
    void* test2 = tsmo::malloc(10);
    void* test3 = tsmo::malloc(100);
    void* test4 = tsmo::malloc(10);
    void* test5 = tsmo::malloc(50);

    auto actualBlocks1 = tsmo::debug::getBlocksInfo();
    std::vector<tsmo::debug::BlockInfo> expectedBlocks1 {
        {30, false},
        {10, false},
        {100, false},
        {10, false},
        {50, false},
        {232, true}
    };

    REQUIRE_THAT(actualBlocks1, Catch::Matchers::Equals(expectedBlocks1));


    tsmo::free(test1);
    tsmo::free(test2);

    tsmo::free(test4);
    tsmo::free(test5);

    tsmo::free(test3);

    auto actualBlocks2 = tsmo::debug::getBlocksInfo();
    std::vector<tsmo::debug::BlockInfo> expectedBlocks2 {
        {592, true}
    };

    REQUIRE_THAT(actualBlocks2, Catch::Matchers::Equals(expectedBlocks2));

}


TEST_CASE( "Test malloc 1000 blocks, write memory to them and check back", "[all]" ) {

    std::uniform_int_distribution<int> dist(1, 1000);
    std::mt19937 engine;
    auto diceRoller = std::bind(dist, engine);

    std::vector<int*> requestedBlocks;
    std::vector<int> values;
    
    for (int i=0; i<1000; ++i) {

        void* ptr = tsmo::malloc(sizeof(int));
        int value = diceRoller();

        int* ptrInt = static_cast<int*>(ptr);
        *ptrInt = value;

        requestedBlocks.push_back(ptrInt);
        values.push_back(value);
    }


    for (int i=0; i<values.size(); ++i) {

        int* ptrInt = static_cast<int*>(requestedBlocks[i]);

        REQUIRE(values[i] == *ptrInt);


        tsmo::free(requestedBlocks[i]);
    }

}


TEST_CASE( "Test multithread malloc and free", "[all]" ) {
    
    std::mutex m;
    std::condition_variable cv;
    bool ready = false;


    auto threadFuncs = [&m, &cv, &ready](int nBlocks) {

        // wait for signal to start
        std::unique_lock lk(m);
        cv.wait(lk, [&ready]{return ready;});
        lk.unlock();

        std::uniform_int_distribution<int> dist(1, 1000);
        std::mt19937 engine;
        auto diceRoller = std::bind(dist, engine);

        std::vector<int*> requestedBlocks;
        std::vector<int> values;
        
        for (int i=0; i<nBlocks; ++i) {

            void* ptr = tsmo::malloc(sizeof(int));
            int value = diceRoller();

            int* ptrInt = static_cast<int*>(ptr);
            *ptrInt = value;

            requestedBlocks.push_back(ptrInt);
            values.push_back(value);
        }


        for (int i=0; i<values.size(); ++i) {

            int* ptrInt = static_cast<int*>(requestedBlocks[i]);

            REQUIRE(values[i] == *ptrInt);

            tsmo::free(requestedBlocks[i]);
        }

    };


    // create threads
    std::list<std::thread> threads;
    const int nThreadsPerCase = 10;
    const int nBlocks = 100;

    for (int i=0; i<nThreadsPerCase; ++i) {
        threads.emplace_back( std::thread(threadFuncs, 2*nBlocks) );
    }

    for (int i=0; i<nThreadsPerCase; ++i) {
        threads.emplace_back( std::thread(threadFuncs, 4*nBlocks) );
    }

    for (int i=0; i<nThreadsPerCase; ++i) {
        threads.emplace_back( std::thread(threadFuncs, 8*nBlocks) );
    }

    for (int i=0; i<nThreadsPerCase; ++i) {
        threads.emplace_back( std::thread(threadFuncs, 10*nBlocks) );
    }

    // signal run
    {
        std::scoped_lock lk(m);
        ready = true;
    }
    cv.notify_all();


    // wait for threads
    for (auto& t : threads) {
        t.join();
    }
}