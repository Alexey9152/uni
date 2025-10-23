#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <vector>
#include <cstdint>

// 1. Числа Фибоначчи
std::vector<uint64_t> fibonacci_sequence(size_t n);

// 2. Проверка палиндрома
bool is_palindrome(int64_t number);

// 3. Узел связного списка
struct ListNode {
    int val;
    ListNode* next;
    ListNode(int x);
    ~ListNode();
};

// Разворот списка итеративно
ListNode* reverse_list(ListNode* head);

// Вспомогательные функции для работы со списком
ListNode* vector_to_list(const std::vector<int>& vec);
std::vector<int> list_to_vector(ListNode* head);

// Вспомогательная функция для конвертации uint64_t в int с проверкой
std::vector<int> convert_to_int_vector(const std::vector<uint64_t>& vec);

// Демонстрация работы функций
void demonstrate_functions();

#endif // FUNCTIONS_H