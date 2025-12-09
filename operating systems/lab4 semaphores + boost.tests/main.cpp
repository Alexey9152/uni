#include "marker_thread.h"

#include <iostream>
#include <string>
#include <windows.h>

int main() {
    std::string filename;
    int recordCount;

    std::cout << "Enter binary filename: ";
    std::getline(std::cin, filename);

    std::cout << "Enter number of records (1-100): ";
    std::cin >> recordCount;
    std::cin.ignore();

    if (recordCount < 1 || recordCount > 100) {
        std::cerr << "Number of records must be between 1 and 100\n";
        return 1;
    }

    if (!SharedFile::create(filename, recordCount)) {
        std::cerr << "Failed to create shared file (Boost.Interprocess)\n";
        return 1;
    }

    int senderCount;
    std::cout << "Enter number of Sender processes: ";
    std::cin >> senderCount;
    std::cin.ignore();

    for (int i = 0; i < senderCount; ++i) {
        STARTUPINFOA si{};
        si.cb = sizeof(si);
        PROCESS_INFORMATION pi{};

        std::string cmd = "sender.exe \"" + filename + "\"";
        BOOL ok = CreateProcessA(
            nullptr,
            cmd.data(),
            nullptr,
            nullptr,
            FALSE,
            CREATE_NEW_CONSOLE,
            nullptr,
            nullptr,
            &si,
            &pi
        );

        if (ok) {
            std::cout << "Started Sender " << i + 1 << "\n";
            CloseHandle(pi.hThread);
            CloseHandle(pi.hProcess);
        } else {
            std::cerr << "Failed to start sender.exe\n";
        }
    }

    // Ожидание готовности всех Sender по счётчику
    std::cout << "Waiting for ready signals...\n";
    while (SharedFile::getSenderReadyCount() < senderCount) {
        Sleep(50);
    }
    std::cout << "All sender(s) ready!\n";

    std::cout << "\n=== RECEIVER ===\n1-read, 2-exit\n";
    for (;;) {
        std::cout << "Receiver command (1/2): ";
        std::string input;
        std::getline(std::cin, input);

        if (input == "1") {
            std::string message;

            // сначала пробуем неблокирующе
            if (!SharedFile::tryReadMessage(message)) {
                std::cout << "*** No messages, waiting...\n";
                SharedFile::readMessage(message); // тут уже блокирующее ожидание
            }

            std::cout << ">> " << message << "\n";
        } else if (input == "2") {
            break;
        } else {
            std::cout << "Invalid command.\n";
        }
    }

    SharedFile::close();
    return 0;
}
