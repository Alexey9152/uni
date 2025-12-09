#pragma once

#include <string>
#include <memory>
#include <cstring>
#include <boost/interprocess/managed_mapped_file.hpp>
#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/interprocess/sync/interprocess_semaphore.hpp>

namespace bip = boost::interprocess;

// Кольцевая очередь в общем файле (mapped file)
struct MessageQueue {
    bip::interprocess_mutex     mutex;
    bip::interprocess_semaphore free_slots;
    bip::interprocess_semaphore used_slots;

    int max_messages;
    int read_pos;
    int write_pos;
    int count;
    int sender_ready_count;
    char messages[100][20];

    MessageQueue(int max_msgs)
        : free_slots(max_msgs)      // все слоты свободны
        , used_slots(0)            // сообщений нет
        , max_messages(max_msgs)
        , read_pos(0)
        , write_pos(0)
        , count(0)
        , sender_ready_count(0)
    {
        for (int i = 0; i < max_msgs && i < 100; ++i) {
            std::memset(messages[i], 0, 20);
        }
    }
};

class SharedFile {
public:
    // создание/открытие сегмента Boost.Interprocess поверх бинарного файла
    static bool create(const std::string& filename, int max_messages);
    static bool open(const std::string& filename);

    // блокирующие операции (по ТЗ: ждать при пустой/полной очереди)
    static bool writeMessage(const std::string& message);
    static bool readMessage(std::string& message);

    // неблокирующие попытки (для сообщений "очередь заполнена/пуста")
    static bool tryWriteMessage(const std::string& message);
    static bool tryReadMessage(std::string& message);

    static void markSenderReady();
    static int  getSenderReadyCount();
    static void close();

private:
    static std::string mapped_name;
    static std::unique_ptr<bip::managed_mapped_file> segment;
    static MessageQueue* sharedQueue;
};
