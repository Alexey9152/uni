#include "marker_thread.h"
#include <iostream>
#include <string>
#include <cstdlib>
#include <chrono>
#include <thread>

#ifdef _WIN32
#include <windows.h>
#endif

int main() {
    std::string filename;
    int recordCount;
    
    std::cout << "Enter binary filename: ";
    std::getline(std::cin, filename);
    std::cout << "Enter number of records (1-100): ";
    std::cin >> recordCount;
    std::cin.ignore();
    
    if (recordCount < 1 || recordCount > 100) {
        std::cerr << "Number of records must be between 1 and 100" << std::endl;
        return 1;
    }
    
    if (!SharedFile::create(filename, recordCount)) {
        std::cerr << "Failed to create shared file" << std::endl;
        return 1;
    }
    
    int senderCount;
    std::cout << "Enter number of Sender processes: ";
    std::cin >> senderCount;
    std::cin.ignore();
    
    std::cout << "Waiting for Sender processes to be ready..." << std::endl;
    
#ifdef _WIN32
    for (int i = 0; i < senderCount; ++i) {
        STARTUPINFOA si;
        PROCESS_INFORMATION pi;
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        ZeroMemory(&pi, sizeof(pi));
        
        std::string command = "cmd /c start \"Sender\" sender.exe " + filename;
        
        if (CreateProcessA(
            NULL,
            const_cast<char*>(command.c_str()),
            NULL,
            NULL,
            FALSE,
            CREATE_NEW_CONSOLE,
            NULL,
            NULL,
            &si,
            &pi
        )) {
            std::cout << "Started Sender process " << i + 1 << std::endl;
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
        } else {
            std::cerr << "Failed to start Sender process " << i + 1 << std::endl;
        }
    }
#endif

    int readySenders = 0;
    int attempts = 0;
    const int maxAttempts = 50;
    
    while (readySenders < senderCount && attempts < maxAttempts) {
        if (SharedFile::isSenderReady()) {
            readySenders++;
            std::cout << "Sender " << readySenders << " is ready" << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        attempts++;
    }
    
    if (readySenders < senderCount) {
        std::cout << "Warning: Only " << readySenders << " out of " << senderCount << " senders are ready" << std::endl;
    } else {
        std::cout << "All Sender processes are ready!" << std::endl;
    }
    
    std::cout << "\n=== RECEIVER PROCESS ===" << std::endl;
    std::cout << "Commands: 1 - read message, 2 - exit" << std::endl;
    
    while (true) {
        std::cout << "\nReceiver command (1/2): ";
        std::string input;
        std::getline(std::cin, input);
        
        if (input == "1") {
            std::string message;
            if (SharedFile::readMessage(message)) {
                std::cout << ">>> Received message: '" << message << "'" << std::endl;
            } else {
                std::cout << "*** No messages available" << std::endl;
            }
        } else if (input == "2") {
            break;
        } else {
            std::cout << "*** Unknown command. Please enter 1 or 2." << std::endl;
        }
    }
    
    SharedFile::close();
    std::cout << "Receiver exited." << std::endl;
    return 0;
}