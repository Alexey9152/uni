#include <gtest/gtest.h>
#include <vector>
#include <windows.h>

DWORD WINAPI min_max_thread(LPVOID lpParam);
DWORD WINAPI average_thread(LPVOID lpParam);

struct MinMaxParams {
    std::vector<int> array;
    int minIndex;
    int maxIndex;
};

struct AverageParams {
    std::vector<int> array;
    double average;
};

void MockSleep(DWORD dwMilliseconds) {
    // В тестах не спим, просто пропускаем задержки
}
// Заменяем Sleep на mock-функцию в тестах
#ifdef TESTING
#define Sleep MockSleep
#endif

TEST(MinMaxThreadTest, FindsMinMaxInArray) {
    std::vector<int> arr = {5, 2, 8, 1, 9, 3};
    MinMaxParams params = {arr, 0, 0};
    
    DWORD result = min_max_thread(&params);
    
    EXPECT_EQ(result, 0);
    EXPECT_EQ(params.minIndex, 3);
    EXPECT_EQ(params.maxIndex, 4);
}

TEST(MinMaxThreadTest, HandlesSingleElement) {
    std::vector<int> arr = {42};
    MinMaxParams params = {arr, 0, 0};
    
    DWORD result = min_max_thread(&params);
    
    EXPECT_EQ(result, 0);
    EXPECT_EQ(params.minIndex, 0);
    EXPECT_EQ(params.maxIndex, 0);
}

TEST(MinMaxThreadTest, HandlesEmptyArray) {
    std::vector<int> arr;
    MinMaxParams params = {arr, 0, 0};
    
    DWORD result = min_max_thread(&params);
    
    EXPECT_EQ(result, 0);
}

TEST(AverageThreadTest, CalculatesAverageCorrectly) {
    std::vector<int> arr = {1, 2, 3, 4, 5};
    AverageParams params = {arr, 0.0};
    
    DWORD result = average_thread(&params);
    
    EXPECT_EQ(result, 0);
    EXPECT_DOUBLE_EQ(params.average, 3.0);
}

TEST(AverageThreadTest, HandlesSingleElement) {
    std::vector<int> arr = {42};
    AverageParams params = {arr, 0.0};
    
    DWORD result = average_thread(&params);
    
    EXPECT_EQ(result, 0);
    EXPECT_DOUBLE_EQ(params.average, 42.0);
}

TEST(AverageThreadTest, HandlesEmptyArray) {
    std::vector<int> arr;
    AverageParams params = {arr, 0.0};
    
    DWORD result = average_thread(&params);
    
    EXPECT_EQ(result, 0);
    EXPECT_DOUBLE_EQ(params.average, 0.0);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}