#include "marker_thread.h"

#include <iostream>
#include <string>
#include <windows.h>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: sender <filename>\n";
        std::cin.get();
        return 1;
    }

    std::string filename = argv[1];

    if (!SharedFile::open(filename)) {
        std::cerr << "Failed to open shared file (Boost.Interprocess)\n";
        std::cin.get();
        return 1;
    }

    SharedFile::markSenderReady();

    std::cout << "=== SENDER ===\n1-send, 2-exit\n";
    for (;;) {
        std::cout << "Sender command (1/2): ";
        std::string input;
        std::getline(std::cin, input);

        if (input == "1") {
            std::string message;
            std::cout << "Enter message (<20 chars): ";
            std::getline(std::cin, message);

            if (message.length() >= 20) {
                std::cout << "Too long!\n";
                continue;
            }

            // Пытаемся отправить, давая пользователю фидбек при полной очереди
            while (!SharedFile::tryWriteMessage(message)) {
                std::cout << "Queue full, waiting...\n";
                Sleep(100);
            }

            std::cout << "Sent: " << message << "\n";
        } else if (input == "2") {
            break;
        } else {
            std::cout << "Invalid command.\n";
        }
    }

    SharedFile::close();
    return 0;
}
