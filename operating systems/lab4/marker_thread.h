#pragma once

#include <string>
#include <iostream>
#include <thread>
#include <chrono>
#include <cstring>

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif

struct MessageQueue {
    int max_messages;       // Максимальное количество сообщений
    int read_pos;           // Позиция чтения
    int write_pos;          // Позиция записи
    int count;              // Количество сообщений в очереди
    bool sender_ready;      // Флаг готовности Sender
    char messages[100][20]; // Массив сообщений (до 100 сообщений)
};

class SharedFile {
public:
    static bool create(const std::string& filename, int max_messages);
    static bool open(const std::string& filename);
    
    static bool writeMessage(const std::string& message);
    static bool readMessage(std::string& message);
    static void markSenderReady();
    static bool isSenderReady();
    static void close();
    
private:
    static bool initializeSharedMemory(bool create_new);
    
#ifdef _WIN32
    static HANDLE fileHandle;
    static HANDLE mapHandle;
    static HANDLE mutex;
#else
    static int fileDescriptor;
#endif
    static MessageQueue* sharedQueue;
    static std::string currentFilename;
    static int queue_max_messages;
};