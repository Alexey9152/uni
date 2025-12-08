#pragma once

#include <string>
#include <windows.h>

struct MessageQueue {
    int max_messages;
    int read_pos;
    int write_pos;
    int count;
    int sender_ready_count;
    char messages[100][20];
};

class SharedFile {
public:
    static bool create(const std::string& filename, int max_messages);
    static bool open(const std::string& filename);
    static bool writeMessage(const std::string& message);
    static bool readMessage(std::string& message);
    static bool tryWriteMessage(const std::string& message); 
    static bool tryReadMessage(std::string& message);        
    static void markSenderReady();
    static int getSenderReadyCount();
    static void close();

private:
    static HANDLE fileHandle;
    static HANDLE mapHandle;
    static HANDLE mutex;        // защита структуры очереди
    static HANDLE freeSlotsSem; // семафор свободных слотов
    static HANDLE usedSlotsSem; // семафор занятых слотов
    static MessageQueue* sharedQueue;
    static std::string currentFilename;
    static int queue_max_messages;

    static std::string makeName(const std::string& base,
                                const std::string& suffix);
};
