#define NOMINMAX
#include <windows.h>
#include <iostream>
#include <limits>
#include <vector>
#include <string>

using namespace std;

struct MinMaxParams {
    vector<int> array;
    int minIndex;
    int maxIndex;
};

struct AverageParams {
    vector<int> array;
    double average;
};

DWORD WINAPI min_max_thread(LPVOID lpParam) {
    MinMaxParams* params = (MinMaxParams*)lpParam;

    if (params->array.empty()) {
        cout << "Поток min_max: массив пуст" << endl;
        return 0;
    }

    params->minIndex = 0;
    params->maxIndex = 0;

    for (int i = 1; i < params->array.size(); i++) {
        if (params->array[i] < params->array[params->minIndex]) {
            params->minIndex = i;
        }
        Sleep(7);

        if (params->array[i] > params->array[params->maxIndex]) {
            params->maxIndex = i;
        }
        Sleep(7);
    }

    cout << "Поток min_max: min = " << params->array[params->minIndex]
        << ", max = " << params->array[params->maxIndex] << endl;

    return 0;
}

DWORD WINAPI average_thread(LPVOID lpParam) {
    AverageParams* params = (AverageParams*)lpParam;

    if (params->array.empty()) {
        cout << "Поток average: массив пуст" << endl;
        params->average = 0.0;
        return 0;
    }

    double sum = 0;
    for (int i = 0; i < params->array.size(); i++) {
        sum += params->array[i];
        Sleep(12);
    }

    params->average = sum / params->array.size();
    cout << "Поток average: среднее = " << params->average << endl;

    return 0;
}

int safe_input_int(const string& prompt) {
    int value;
    cin >> value
    return value;
}
int main() {
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);

    int size = safe_input_int("Введите размер массива (положительное целое число): ");

    vector<int> array(size);
    cout << "Введите " << size << " элементов массива:" << endl;
    for (int i = 0; i < size; i++) {
        array[i] = safe_input_int("Элемент [" + to_string(i) + "]: ");
    }

    MinMaxParams minMaxParams = { array, 0, 0 };
    AverageParams averageParams = { array, 0.0 };

    if (!array.empty()) {
        array[minMaxParams.minIndex] = static_cast<int>(averageParams.average);
        array[minMaxParams.maxIndex] = static_cast<int>(averageParams.average);

        cout << "Измененный массив: ";
        for (int i = 0; i < array.size(); i++) {
            cout << array[i] << " ";
        }
        cout << endl;
    }

    return 0;
}