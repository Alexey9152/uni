#include "marker_thread.h"
#include <iostream>
#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>

namespace bip = boost::interprocess;

std::string SharedFile::mapped_name;
std::unique_ptr<bip::managed_mapped_file> SharedFile::segment;
MessageQueue* SharedFile::sharedQueue = nullptr;

bool SharedFile::create(const std::string& filename, int max_messages) {
    mapped_name = filename;

    // Удаляем старый файл, если есть
    bip::file_mapping::remove(mapped_name.c_str()); // [web:229]

    try {
        std::size_t size = sizeof(MessageQueue) + 4096;

        segment = std::make_unique<bip::managed_mapped_file>(
            bip::create_only,
            mapped_name.c_str(),
            size
        );

        // ОДИН объект очереди в сегменте
        sharedQueue = segment->construct<MessageQueue>("Queue")(max_messages);
        return sharedQueue != nullptr;
    } catch (const std::exception& ex) {
        std::cerr << "SharedFile::create exception: " << ex.what() << "\n";
        segment.reset();
        sharedQueue = nullptr;
        return false;
    }
}

bool SharedFile::open(const std::string& filename) {
    mapped_name = filename;

    try {
        segment = std::make_unique<bip::managed_mapped_file>(
            bip::open_only,
            mapped_name.c_str()
        );

        auto res = segment->find<MessageQueue>("Queue");
        sharedQueue = res.first;
        return sharedQueue != nullptr;
    } catch (const std::exception& ex) {
        std::cerr << "SharedFile::open exception: " << ex.what() << "\n";
        segment.reset();
        sharedQueue = nullptr;
        return false;
    }
}

bool SharedFile::writeMessage(const std::string& message) {
    if (!sharedQueue || message.length() >= 20) return false;

    // 1) Ждём свободный слот (если очередь полна — блокируемся)
    sharedQueue->free_slots.wait(); // [web:215]

    {
        // 2) Критическая секция
        bip::scoped_lock<bip::interprocess_mutex> lock(sharedQueue->mutex);

        int pos = sharedQueue->write_pos;
        std::strncpy(sharedQueue->messages[pos], message.c_str(), 19);
        sharedQueue->messages[pos][19] = '\0';

        sharedQueue->write_pos =
            (sharedQueue->write_pos + 1) % sharedQueue->max_messages;
        sharedQueue->count++;
    }

    // 3) Сообщаем receiver'у, что появилось сообщение
    sharedQueue->used_slots.post();
    return true;
}

bool SharedFile::tryWriteMessage(const std::string& message) {
    if (!sharedQueue || message.length() >= 20) return false;

    // Неблокирующая попытка
    if (!sharedQueue->free_slots.try_wait())
        return false; // слотов нет — очередь заполнена

    {
        bip::scoped_lock<bip::interprocess_mutex> lock(sharedQueue->mutex);

        int pos = sharedQueue->write_pos;
        std::strncpy(sharedQueue->messages[pos], message.c_str(), 19);
        sharedQueue->messages[pos][19] = '\0';

        sharedQueue->write_pos =
            (sharedQueue->write_pos + 1) % sharedQueue->max_messages;
        sharedQueue->count++;
    }

    sharedQueue->used_slots.post();
    return true;
}

bool SharedFile::readMessage(std::string& message) {
    if (!sharedQueue) return false;

    // 1) Ждём сообщение (если пусто — блокируемся)
    sharedQueue->used_slots.wait();

    {
        bip::scoped_lock<bip::interprocess_mutex> lock(sharedQueue->mutex);

        int pos = sharedQueue->read_pos;
        message = sharedQueue->messages[pos];

        sharedQueue->read_pos =
            (sharedQueue->read_pos + 1) % sharedQueue->max_messages;
        sharedQueue->count--;
    }

    // 3) Освобождаем один слот
    sharedQueue->free_slots.post();
    return true;
}

bool SharedFile::tryReadMessage(std::string& message) {
    if (!sharedQueue) return false;

    // Неблокирующая попытка
    if (!sharedQueue->used_slots.try_wait())
        return false; // сообщений нет

    {
        bip::scoped_lock<bip::interprocess_mutex> lock(sharedQueue->mutex);

        int pos = sharedQueue->read_pos;
        message = sharedQueue->messages[pos];

        sharedQueue->read_pos =
            (sharedQueue->read_pos + 1) % sharedQueue->max_messages;
        sharedQueue->count--;
    }

    sharedQueue->free_slots.post();
    return true;
}

void SharedFile::markSenderReady() {
    if (!sharedQueue) return;
    bip::scoped_lock<bip::interprocess_mutex> lock(sharedQueue->mutex);
    sharedQueue->sender_ready_count++;
}

int SharedFile::getSenderReadyCount() {
    if (!sharedQueue) return 0;
    bip::scoped_lock<bip::interprocess_mutex> lock(sharedQueue->mutex);
    return sharedQueue->sender_ready_count;
}

void SharedFile::close() {
    if (segment && sharedQueue) {
        try {
            segment->destroy<MessageQueue>("Queue");
        } catch (...) {
            // игнорируем
        }
    }
    segment.reset();
    sharedQueue = nullptr;
    // bip::file_mapping::remove(mapped_name.c_str()); // по желанию
}
