#include "marker_thread.h"
#include <cstring>
#include <iostream>

// статика
HANDLE SharedFile::fileHandle = INVALID_HANDLE_VALUE;
HANDLE SharedFile::mapHandle = NULL;
HANDLE SharedFile::mutex = NULL;
HANDLE SharedFile::freeSlotsSem = NULL;
HANDLE SharedFile::usedSlotsSem = NULL;
MessageQueue* SharedFile::sharedQueue = nullptr;
std::string SharedFile::currentFilename = "";
int SharedFile::queue_max_messages = 0;

std::string SharedFile::makeName(const std::string& base,
                                 const std::string& suffix) {
    return "Global\\" + base + "_" + suffix;
}

bool SharedFile::create(const std::string& filename, int max_messages) {
    currentFilename = filename;
    queue_max_messages = max_messages;

    // мьютекс для защиты очереди
    mutex = CreateMutexA(nullptr, FALSE, makeName(filename, "mutex").c_str());
    if (!mutex) return false;

    // семафор свободных слотов (начальное значение = max_messages)
    freeSlotsSem = CreateSemaphoreA(nullptr,
                                    max_messages,   // initial count
                                    max_messages,   // max count
                                    makeName(filename, "free").c_str());
    if (!freeSlotsSem) return false;

    // семафор занятых слотов (initial = 0)
    usedSlotsSem = CreateSemaphoreA(nullptr,
                                    0,              // initial
                                    max_messages,   // max
                                    makeName(filename, "used").c_str());
    if (!usedSlotsSem) return false;

    // файл + mmap
    DWORD access = GENERIC_READ | GENERIC_WRITE;
    fileHandle = CreateFileA(filename.c_str(), access,
                             FILE_SHARE_READ | FILE_SHARE_WRITE,
                             nullptr,
                             CREATE_ALWAYS,
                             FILE_ATTRIBUTE_NORMAL,
                             nullptr);
    if (fileHandle == INVALID_HANDLE_VALUE) return false;

    mapHandle = CreateFileMappingA(fileHandle, nullptr,
                                   PAGE_READWRITE,
                                   0, sizeof(MessageQueue),
                                   nullptr);
    if (!mapHandle) return false;

    sharedQueue = static_cast<MessageQueue*>(
        MapViewOfFile(mapHandle, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(MessageQueue)));
    if (!sharedQueue) return false;

    // инициализация очереди
    sharedQueue->max_messages = max_messages;
    sharedQueue->read_pos = 0;
    sharedQueue->write_pos = 0;
    sharedQueue->count = 0;
    sharedQueue->sender_ready_count = 0;
    for (int i = 0; i < max_messages; ++i) {
        std::memset(sharedQueue->messages[i], 0, 20);
    }

    return true;
}

bool SharedFile::open(const std::string& filename) {
    currentFilename = filename;

    mutex = OpenMutexA(MUTEX_ALL_ACCESS, FALSE,
                       makeName(filename, "mutex").c_str());
    if (!mutex) return false;

    freeSlotsSem = OpenSemaphoreA(SEMAPHORE_ALL_ACCESS, FALSE,
                                  makeName(filename, "free").c_str());
    if (!freeSlotsSem) return false;

    usedSlotsSem = OpenSemaphoreA(SEMAPHORE_ALL_ACCESS, FALSE,
                                  makeName(filename, "used").c_str());
    if (!usedSlotsSem) return false;

    fileHandle = CreateFileA(filename.c_str(),
                             GENERIC_READ | GENERIC_WRITE,
                             FILE_SHARE_READ | FILE_SHARE_WRITE,
                             nullptr,
                             OPEN_EXISTING,
                             FILE_ATTRIBUTE_NORMAL,
                             nullptr);
    if (fileHandle == INVALID_HANDLE_VALUE) return false;

    mapHandle = CreateFileMappingA(fileHandle, nullptr,
                                   PAGE_READWRITE,
                                   0, sizeof(MessageQueue),
                                   nullptr);
    if (!mapHandle) return false;

    sharedQueue = static_cast<MessageQueue*>(
        MapViewOfFile(mapHandle, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(MessageQueue)));
    if (!sharedQueue) return false;

    queue_max_messages = sharedQueue->max_messages;
    return true;
}

bool SharedFile::writeMessage(const std::string& message) {
    if (!sharedQueue || message.length() >= 20) return false;

    // 1) ждём свободный слот
    if (WaitForSingleObject(freeSlotsSem, INFINITE) != WAIT_OBJECT_0)
        return false;

    // 2) критическая секция над структурой очереди
    WaitForSingleObject(mutex, INFINITE);

    // (доп. защита от гонок: очередь вдруг оказалась переполнена)
    if (sharedQueue->count >= sharedQueue->max_messages) {
        ReleaseMutex(mutex);
        ReleaseSemaphore(freeSlotsSem, 1, nullptr); // вернуть слот
        return false;
    }

    std::strncpy(sharedQueue->messages[sharedQueue->write_pos],
                 message.c_str(), 19);
    sharedQueue->messages[sharedQueue->write_pos][19] = '\0';
    sharedQueue->write_pos =
        (sharedQueue->write_pos + 1) % sharedQueue->max_messages;
    sharedQueue->count++;

    ReleaseMutex(mutex);

    // 3) сообщаем receiver'у, что появилось одно сообщение
    ReleaseSemaphore(usedSlotsSem, 1, nullptr);

    return true;
}

bool SharedFile::readMessage(std::string& message) {
    if (!sharedQueue) return false;

    // 1) ждём хотя бы одно сообщение
    if (WaitForSingleObject(usedSlotsSem, INFINITE) != WAIT_OBJECT_0)
        return false;

    // 2) критическая секция
    WaitForSingleObject(mutex, INFINITE);

    if (sharedQueue->count == 0) {
        ReleaseMutex(mutex);
        ReleaseSemaphore(freeSlotsSem, 1, nullptr); // вернуть слот
        return false;
    }

    message = sharedQueue->messages[sharedQueue->read_pos];
    sharedQueue->read_pos =
        (sharedQueue->read_pos + 1) % sharedQueue->max_messages;
    sharedQueue->count--;

    ReleaseMutex(mutex);

    // 3) сообщаем sender'ам, что освободился один слот
    ReleaseSemaphore(freeSlotsSem, 1, nullptr);

    return true;
}

void SharedFile::markSenderReady() {
    if (!sharedQueue) return;
    WaitForSingleObject(mutex, INFINITE);
    sharedQueue->sender_ready_count++;
    ReleaseMutex(mutex);
}

int SharedFile::getSenderReadyCount() {
    if (!sharedQueue) return 0;
    WaitForSingleObject(mutex, INFINITE);
    int v = sharedQueue->sender_ready_count;
    ReleaseMutex(mutex);
    return v;
}

void SharedFile::close() {
    if (mutex) { CloseHandle(mutex); mutex = NULL; }
    if (freeSlotsSem) { CloseHandle(freeSlotsSem); freeSlotsSem = NULL; }
    if (usedSlotsSem) { CloseHandle(usedSlotsSem); usedSlotsSem = NULL; }
    if (sharedQueue) { UnmapViewOfFile(sharedQueue); sharedQueue = nullptr; }
    if (mapHandle) { CloseHandle(mapHandle); mapHandle = NULL; }
    if (fileHandle != INVALID_HANDLE_VALUE) {
        CloseHandle(fileHandle);
        fileHandle = INVALID_HANDLE_VALUE;
    }
}
bool SharedFile::tryWriteMessage(const std::string& message) {
    if (!sharedQueue || message.length() >= 20) return false;

    // Пытаемся взять семафор свободных слотов без ожидания
    DWORD res = WaitForSingleObject(freeSlotsSem, 0);
    if (res == WAIT_TIMEOUT) {
        return false; // очередь полна, свободных слотов нет
    }
    if (res != WAIT_OBJECT_0) {
        return false; // ошибка ожидания
    }

    // Критическая секция над очередью
    WaitForSingleObject(mutex, INFINITE);

    if (sharedQueue->count >= sharedQueue->max_messages) {
        // Переполнение на всякий случай – вернуть слот
        ReleaseMutex(mutex);
        ReleaseSemaphore(freeSlotsSem, 1, nullptr);
        return false;
    }

    std::strncpy(sharedQueue->messages[sharedQueue->write_pos],
                 message.c_str(), 19);
    sharedQueue->messages[sharedQueue->write_pos][19] = '\0';
    sharedQueue->write_pos =
        (sharedQueue->write_pos + 1) % sharedQueue->max_messages;
    sharedQueue->count++;

    ReleaseMutex(mutex);

    // Сигнал Receiver’у, что появилось сообщение
    ReleaseSemaphore(usedSlotsSem, 1, nullptr);

    return true;
}

bool SharedFile::tryReadMessage(std::string& message) {
    if (!sharedQueue) return false;

    // Пытаемся взять семафор занятых слотов без ожидания
    DWORD res = WaitForSingleObject(usedSlotsSem, 0);
    if (res == WAIT_TIMEOUT) {
        return false; // сообщений нет
    }
    if (res != WAIT_OBJECT_0) {
        return false; // ошибка ожидания
    }

    // Критическая секция
    WaitForSingleObject(mutex, INFINITE);

    if (sharedQueue->count == 0) {
        ReleaseMutex(mutex);
        ReleaseSemaphore(freeSlotsSem, 1, nullptr);
        return false;
    }

    message = sharedQueue->messages[sharedQueue->read_pos];
    sharedQueue->read_pos =
        (sharedQueue->read_pos + 1) % sharedQueue->max_messages;
    sharedQueue->count--;

    ReleaseMutex(mutex);

    // Сигнал Sender’ам: освободился слот
    ReleaseSemaphore(freeSlotsSem, 1, nullptr);

    return true;
}
