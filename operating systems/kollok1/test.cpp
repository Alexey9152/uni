#include <gtest/gtest.h>
#include <vector>
#include <stdexcept>
#include <limits>
#include "functions.h"

// Тесты для чисел Фибоначчи
TEST(FibonacciTest, BasicCases) {
    EXPECT_EQ(fibonacci_sequence(0), std::vector<uint64_t>{});
    EXPECT_EQ(fibonacci_sequence(1), std::vector<uint64_t>{0});
    EXPECT_EQ(fibonacci_sequence(2), (std::vector<uint64_t>{0, 1}));
    EXPECT_EQ(fibonacci_sequence(5), (std::vector<uint64_t>{0, 1, 1, 2, 3}));
}

TEST(FibonacciTest, EdgeCases) {
    // Тест на 10 элементов
    auto result10 = fibonacci_sequence(10);
    std::vector<uint64_t> expected10 = {0, 1, 1, 2, 3, 5, 8, 13, 21, 34};
    EXPECT_EQ(result10, expected10);
    
    // Тест на 1 элемент
    EXPECT_EQ(fibonacci_sequence(1).size(), 1);
    EXPECT_EQ(fibonacci_sequence(1)[0], 0);
}

TEST(FibonacciTest, LargeInput) {
    // Тест с большим n, но без переполнения
    auto result = fibonacci_sequence(50);
    EXPECT_EQ(result.size(), 50);
    EXPECT_GT(result.back(), 0);
}

// Тесты для палиндромов
TEST(PalindromeTest, PositiveCases) {
    EXPECT_TRUE(is_palindrome(121));
    EXPECT_TRUE(is_palindrome(12321));
    EXPECT_TRUE(is_palindrome(1234321));
    EXPECT_TRUE(is_palindrome(0));
    EXPECT_TRUE(is_palindrome(7));
    EXPECT_TRUE(is_palindrome(99));
}

TEST(PalindromeTest, NegativeCases) {
    EXPECT_FALSE(is_palindrome(123));
    EXPECT_FALSE(is_palindrome(1234));
    EXPECT_FALSE(is_palindrome(-121)); // Отрицательные не палиндромы
    EXPECT_FALSE(is_palindrome(10));
}

TEST(PalindromeTest, BoundaryCases) {
    EXPECT_TRUE(is_palindrome(1));
    EXPECT_TRUE(is_palindrome(9));
    EXPECT_TRUE(is_palindrome(1111111));
    EXPECT_FALSE(is_palindrome(1000001)); // Граничный случай
}

// Тесты для разворота списка
TEST(ReverseListTest, BasicReversal) {
    std::vector<int> input = {1, 2, 3, 4, 5};
    ListNode* head = vector_to_list(input);
    ListNode* reversed = reverse_list(head);
    std::vector<int> result = list_to_vector(reversed);
    
    std::reverse(input.begin(), input.end());
    EXPECT_EQ(result, input);
    
    delete reversed;
}

TEST(ReverseListTest, EdgeCases) {
    // Пустой список
    EXPECT_EQ(reverse_list(nullptr), nullptr);
    
    // Один элемент
    std::vector<int> single = {42};
    ListNode* single_node = vector_to_list(single);
    ListNode* reversed_single = reverse_list(single_node);
    
    EXPECT_EQ(reversed_single->val, 42);
    EXPECT_EQ(reversed_single->next, nullptr);
    
    delete reversed_single;
}

TEST(ReverseListTest, TwoElements) {
    std::vector<int> input = {1, 2};
    ListNode* head = vector_to_list(input);
    ListNode* reversed = reverse_list(head);
    std::vector<int> result = list_to_vector(reversed);
    
    EXPECT_EQ(result, (std::vector<int>{2, 1}));
    delete reversed;
}

TEST(ReverseListTest, LargeList) {
    std::vector<int> input;
    for (int i = 0; i < 100; ++i) {
        input.push_back(i);
    }
    
    ListNode* head = vector_to_list(input);
    ListNode* reversed = reverse_list(head);
    std::vector<int> result = list_to_vector(reversed);
    
    std::reverse(input.begin(), input.end());
    EXPECT_EQ(result, input);
    
    delete reversed;
}

// Интеграционные тесты
TEST(IntegrationTest, FibonacciAndPalindrome) {
    // Генерируем числа Фибоначчи и проверяем некоторые на палиндром
    auto fib_seq = fibonacci_sequence(20);
    
    // 8 - палиндром (однозначное)
    EXPECT_TRUE(is_palindrome(static_cast<int64_t>(fib_seq[6]))); // fib_seq[6] = 8
    
    // 55 - палиндром
    EXPECT_TRUE(is_palindrome(static_cast<int64_t>(fib_seq[10]))); // fib_seq[10] = 55
}

TEST(IntegrationTest, ListAndReverse) {
    // Создаем список из чисел Фибоначчи и разворачиваем
    auto fib_seq_uint = fibonacci_sequence(6); // {0, 1, 1, 2, 3, 5}
    std::vector<int> fib_seq_int;
    for (auto num : fib_seq_uint) {
        // Безопасное преобразование для малых чисел Фибоначчи
        fib_seq_int.push_back(static_cast<int>(num));
    }
    
    ListNode* head = vector_to_list(fib_seq_int);
    ListNode* reversed = reverse_list(head);
    std::vector<int> result = list_to_vector(reversed);
    
    std::vector<int> expected = {5, 3, 2, 1, 1, 0};
    EXPECT_EQ(result, expected);
    
    delete reversed;
}

// Тест для функции конвертации
TEST(ConversionTest, SafeConversion) {
    std::vector<uint64_t> small_numbers = {0, 1, 2, 3, 10};
    auto result = convert_to_int_vector(small_numbers);
    EXPECT_EQ(result, (std::vector<int>{0, 1, 2, 3, 10}));
    
    // Тест на слишком большие числа
    std::vector<uint64_t> large_numbers = {static_cast<uint64_t>(std::numeric_limits<int>::max()) + 1};
    EXPECT_THROW(convert_to_int_vector(large_numbers), std::overflow_error);
}