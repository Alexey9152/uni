#include "functions.h"
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <stdexcept>
#include <limits>

// 1. Числа Фибоначчи
std::vector<uint64_t> fibonacci_sequence(size_t n) {
    if (n == 0) return {};
    if (n == 1) return {0};
    
    std::vector<uint64_t> result = {0, 1};
    result.reserve(n);
    
    for (size_t i = 2; i < n; ++i) {
        // Проверка на переполнение
        if (result[i-1] > UINT64_MAX - result[i-2]) {
            throw std::overflow_error("Fibonacci sequence overflow");
        }
        uint64_t next = result[i-1] + result[i-2];
        result.push_back(next);
    }
    return result;
}

// 2. Проверка палиндрома
bool is_palindrome(int64_t number) {
    if (number < 0) return false;
    if (number < 10) return true; // Однозначные числа - палиндромы
    
    std::string str = std::to_string(number);
    size_t left = 0, right = str.length() - 1;
    
    while (left < right) {
        if (str[left] != str[right]) return false;
        ++left;
        --right;
    }
    return true;
}

// Реализации методов ListNode
ListNode::ListNode(int x) : val(x), next(nullptr) {}
ListNode::~ListNode() { delete next; } // Рекурсивное удаление

// Разворот списка итеративно
ListNode* reverse_list(ListNode* head) {
    ListNode* prev = nullptr;
    ListNode* curr = head;
    
    while (curr != nullptr) {
        ListNode* next_temp = curr->next;
        curr->next = prev;
        prev = curr;
        curr = next_temp;
    }
    return prev;
}

// Вспомогательные функции для работы со списком
ListNode* vector_to_list(const std::vector<int>& vec) {
    if (vec.empty()) return nullptr;
    
    ListNode* head = new ListNode(vec[0]);
    ListNode* current = head;
    for (size_t i = 1; i < vec.size(); ++i) {
        current->next = new ListNode(vec[i]);
        current = current->next;
    }
    return head;
}

std::vector<int> list_to_vector(ListNode* head) {
    std::vector<int> result;
    while (head != nullptr) {
        result.push_back(head->val);
        head = head->next;
    }
    return result;
}

// Вспомогательная функция для конвертации uint64_t в int с проверкой
std::vector<int> convert_to_int_vector(const std::vector<uint64_t>& vec) {
    std::vector<int> result;
    result.reserve(vec.size());
    for (uint64_t num : vec) {
        if (num > static_cast<uint64_t>(std::numeric_limits<int>::max())) {
            throw std::overflow_error("Number too large for int conversion");
        }
        result.push_back(static_cast<int>(num));
    }
    return result;
}

// Демонстрация работы функций
void demonstrate_functions() {
    std::cout << "=== Демонстрация работы функций ===\n";
    
    // 1. Фибоначчи
    std::cout << "1. Первые 10 чисел Фибоначчи: ";
    auto fib = fibonacci_sequence(10);
    for (auto num : fib) {
        std::cout << num << " ";
    }
    std::cout << "\n";
    
    // 2. Палиндромы
    std::cout << "2. Проверка палиндромов:\n";
    std::vector<int64_t> test_numbers = {121, 123, 12321, -121, 0, 1234321};
    for (auto num : test_numbers) {
        std::cout << "   " << num << " -> " 
                  << (is_palindrome(num) ? "палиндром" : "не палиндром") << "\n";
    }
    
    // 3. Разворот списка
    std::cout << "3. Разворот связного списка:\n";
    std::vector<int> list_data = {1, 2, 3, 4, 5};
    ListNode* head = vector_to_list(list_data);
    ListNode* reversed = reverse_list(head);
    std::vector<int> result = list_to_vector(reversed);
    
    std::cout << "   Исходный: ";
    for (auto num : list_data) std::cout << num << " ";
    std::cout << "\n   Развернутый: ";
    for (auto num : result) std::cout << num << " ";
    std::cout << "\n";
    
    delete reversed; // Очистка памяти
}