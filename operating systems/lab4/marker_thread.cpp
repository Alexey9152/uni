#include "marker_thread.h"

#ifdef _WIN32
HANDLE SharedFile::fileHandle = INVALID_HANDLE_VALUE;
HANDLE SharedFile::mapHandle = NULL;
HANDLE SharedFile::mutex = NULL;
#else
int SharedFile::fileDescriptor = -1;
#endif

MessageQueue* SharedFile::sharedQueue = nullptr;
std::string SharedFile::currentFilename = "";
int SharedFile::queue_max_messages = 0;

bool SharedFile::create(const std::string& filename, int max_messages) {
    currentFilename = filename;
    queue_max_messages = max_messages;
    
    if (max_messages > 100) {
        std::cerr << "Maximum messages limit is 100" << std::endl;
        return false;
    }
    
#ifdef _WIN32
    mutex = CreateMutexA(NULL, FALSE, ("Global\\" + filename + "_mutex").c_str());
    if (mutex == NULL) {
        std::cerr << "Failed to create mutex" << std::endl;
        return false;
    }
#endif
    
    return initializeSharedMemory(true);
}

bool SharedFile::open(const std::string& filename) {
    currentFilename = filename;
    
#ifdef _WIN32
    mutex = OpenMutexA(MUTEX_ALL_ACCESS, FALSE, ("Global\\" + filename + "_mutex").c_str());
    if (mutex == NULL) {
        std::cerr << "Failed to open mutex" << std::endl;
        return false;
    }
#endif
    
    return initializeSharedMemory(false);
}

bool SharedFile::initializeSharedMemory(bool create_new) {
#ifdef _WIN32
    DWORD dwDesiredAccess = create_new ? GENERIC_READ | GENERIC_WRITE : GENERIC_READ | GENERIC_WRITE;
    DWORD dwCreationDisposition = create_new ? CREATE_ALWAYS : OPEN_EXISTING;

    fileHandle = CreateFileA(
        currentFilename.c_str(),
        dwDesiredAccess,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        dwCreationDisposition,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );
    
    if (fileHandle == INVALID_HANDLE_VALUE) {
        DWORD error = GetLastError();
        std::cerr << "Failed to " << (create_new ? "create" : "open") << " file: " << error << std::endl;
        return false;
    }
    mapHandle = CreateFileMapping(
        fileHandle,
        NULL,
        PAGE_READWRITE,
        0,
        sizeof(MessageQueue),
        NULL
    );
    
    if (mapHandle == NULL) {
        DWORD error = GetLastError();
        std::cerr << "Failed to create file mapping: " << error << std::endl;
        CloseHandle(fileHandle);
        return false;
    }
    sharedQueue = (MessageQueue*)MapViewOfFile(
        mapHandle,
        FILE_MAP_ALL_ACCESS,
        0, 0, sizeof(MessageQueue)
    );
    
    if (sharedQueue == NULL) {
        DWORD error = GetLastError();
        std::cerr << "Failed to map view of file: " << error << std::endl;
        CloseHandle(mapHandle);
        CloseHandle(fileHandle);
        return false;
    }

    if (create_new) {
        sharedQueue->max_messages = queue_max_messages;
        sharedQueue->read_pos = 0;
        sharedQueue->write_pos = 0;
        sharedQueue->count = 0;
        sharedQueue->sender_ready = false;
        
        for (int i = 0; i < queue_max_messages; i++) {
            memset(sharedQueue->messages[i], 0, 20);
        }
        std::cout << "Created shared file with " << queue_max_messages << " message slots" << std::endl;
    } else {
        queue_max_messages = sharedQueue->max_messages;
        std::cout << "Opened shared file with " << queue_max_messages << " message slots" << std::endl;
    }
    
#else
    //o
#endif
    
    return true;
}

bool SharedFile::writeMessage(const std::string& message) {
    if (!sharedQueue || message.length() >= 20) return false;
    
#ifdef _WIN32
    WaitForSingleObject(mutex, INFINITE);
#endif
    
    if (sharedQueue->count >= sharedQueue->max_messages) {
#ifdef _WIN32
        ReleaseMutex(mutex);
#endif
        return false;
    }

    strncpy(sharedQueue->messages[sharedQueue->write_pos], message.c_str(), 19);
    sharedQueue->messages[sharedQueue->write_pos][19] = '\0';

    sharedQueue->write_pos = (sharedQueue->write_pos + 1) % sharedQueue->max_messages;
    sharedQueue->count++;
    
#ifdef _WIN32
    ReleaseMutex(mutex);
#endif
    
    std::cout << "Message written to queue. Count: " << sharedQueue->count << std::endl;
    return true;
}

bool SharedFile::readMessage(std::string& message) {
    if (!sharedQueue) return false;
    
#ifdef _WIN32
    WaitForSingleObject(mutex, INFINITE);
#endif
    
    if (sharedQueue->count == 0) {
#ifdef _WIN32
        ReleaseMutex(mutex);
#endif
        return false;
    }

    message = sharedQueue->messages[sharedQueue->read_pos];

    sharedQueue->read_pos = (sharedQueue->read_pos + 1) % sharedQueue->max_messages;
    sharedQueue->count--;
    
#ifdef _WIN32
    ReleaseMutex(mutex);
#endif
    
    std::cout << "Message read from queue. Count: " << sharedQueue->count << std::endl;
    return true;
}

void SharedFile::markSenderReady() {
    if (sharedQueue) {
#ifdef _WIN32
        WaitForSingleObject(mutex, INFINITE);
#endif
        sharedQueue->sender_ready = true;
#ifdef _WIN32
        ReleaseMutex(mutex);
#endif
        std::cout << "Sender marked as ready" << std::endl;
    }
}

bool SharedFile::isSenderReady() {
    if (!sharedQueue) return false;
    
#ifdef _WIN32
    WaitForSingleObject(mutex, INFINITE);
#endif
    bool ready = sharedQueue->sender_ready;
#ifdef _WIN32
    ReleaseMutex(mutex);
#endif
    
    return ready;
}

void SharedFile::close() {
#ifdef _WIN32
    if (mutex) {
        CloseHandle(mutex);
        mutex = NULL;
    }
    if (sharedQueue) {
        UnmapViewOfFile(sharedQueue);
        sharedQueue = nullptr;
    }
    if (mapHandle) {
        CloseHandle(mapHandle);
        mapHandle = NULL;
    }
    if (fileHandle != INVALID_HANDLE_VALUE) {
        CloseHandle(fileHandle);
        fileHandle = INVALID_HANDLE_VALUE;
    }
#else
    //o
#endif
}