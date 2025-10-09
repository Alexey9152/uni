#include <gtest/gtest.h>
#include "marker_thread.h"
#include <vector>
#include <thread>
#include <chrono>

class MarkerThreadTest : public ::testing::Test {
protected:
    void SetUp() override {
        array_size = 10;
        array.resize(array_size, 0);
    }

    void TearDown() override {
        for (auto& thread : threads) {
            if (thread) {
                thread->stop();
            }
        }
        threads.clear();
    }

    int array_size;
    std::vector<int> array;
    std::mutex array_mutex;
    std::condition_variable cv;
    std::condition_variable main_cv;
    std::vector<std::unique_ptr<MarkerThread>> threads;
};

TEST_F(MarkerThreadTest, ThreadCreation) {
    MarkerThread thread(1, array, array_mutex, cv, main_cv);
    EXPECT_EQ(thread.getId(), 1);
    EXPECT_FALSE(thread.isFinished());
    EXPECT_FALSE(thread.isWaiting());
    EXPECT_EQ(thread.getMarkedCount(), 0);
}

TEST_F(MarkerThreadTest, SingleThreadMarking) {
    threads.push_back(std::make_unique<MarkerThread>(1, array, array_mutex, cv, main_cv));
    threads[0]->start();
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    EXPECT_TRUE(threads[0]->getMarkedCount() > 0);
    
    threads[0]->stop();
    EXPECT_TRUE(threads[0]->isFinished());
}

TEST_F(MarkerThreadTest, MultipleThreadsCreation) {
    const int thread_count = 3;
    for (int i = 0; i < thread_count; ++i) {
        threads.push_back(std::make_unique<MarkerThread>(i + 1, array, array_mutex, cv, main_cv));
        threads[i]->start();
    }
    
    for (int i = 0; i < thread_count; ++i) {
        EXPECT_EQ(threads[i]->getId(), i + 1);
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    for (int i = 0; i < thread_count; ++i) {
        threads[i]->stop();
        EXPECT_TRUE(threads[i]->isFinished());
    }
}

TEST_F(MarkerThreadTest, ArrayAccessSynchronization) {
    threads.push_back(std::make_unique<MarkerThread>(1, array, array_mutex, cv, main_cv));
    threads.push_back(std::make_unique<MarkerThread>(2, array, array_mutex, cv, main_cv));
    
    threads[0]->start();
    threads[1]->start();
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    bool found_thread1 = false;
    bool found_thread2 = false;
    
    {
        std::unique_lock<std::mutex> lock(array_mutex);
        for (int value : array) {
            if (value == 1) found_thread1 = true;
            if (value == 2) found_thread2 = true;
        }
    }
    
    EXPECT_TRUE(found_thread1 || found_thread2);
    
    threads[0]->stop();
    threads[1]->stop();
}

TEST_F(MarkerThreadTest, ThreadStopClearsMarks) {
    threads.push_back(std::make_unique<MarkerThread>(1, array, array_mutex, cv, main_cv));
    threads[0]->start();
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    int marked_before = threads[0]->getMarkedCount();
    EXPECT_GT(marked_before, 0);
    
    threads[0]->stop();
    
    bool has_marks = false;
    {
        std::unique_lock<std::mutex> lock(array_mutex);
        for (int value : array) {
            if (value == 1) {
                has_marks = true;
                break;
            }
        }
    }
    
    EXPECT_FALSE(has_marks);
}