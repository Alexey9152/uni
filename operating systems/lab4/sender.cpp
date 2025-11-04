#include "marker_thread.h"
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <filename>" << std::endl;
        std::cout << "Press Enter to exit...";
        std::cin.get();
        return 1;
    }
    
    std::string filename = argv[1];
    
    if (!SharedFile::open(filename)) {
        std::cerr << "Failed to open shared file: " << filename << std::endl;
        std::cout << "Press Enter to exit...";
        std::cin.get();
        return 1;
    }
    
    // Сообщаем Receiver о готовности
    SharedFile::markSenderReady();
    
    std::cout << "=== SENDER PROCESS ===" << std::endl;
    std::cout << "Commands: 1 - send message, 2 - exit" << std::endl;
    
    while (true) {
        std::cout << "\nSender command (1/2): ";
        std::string input;
        std::getline(std::cin, input);
        
        if (input == "1") {
            std::string message;
            std::cout << "Enter message (max 19 chars): ";
            std::getline(std::cin, message);
            
            if (message.length() >= 20) {
                std::cout << "*** Message too long! Max 19 characters." << std::endl;
                continue;
            }
            
            if (SharedFile::writeMessage(message)) {
                std::cout << ">>> Message sent successfully!" << std::endl;
            } else {
                std::cout << "*** Failed to send message - queue is full" << std::endl;
            }
        } else if (input == "2") {
            break;
        } else {
            std::cout << "*** Unknown command. Please enter 1 or 2." << std::endl;
        }
    }
    
    SharedFile::close();
    std::cout << "Sender exited." << std::endl;
    std::cout << "Press Enter to close...";
    std::cin.get();
    return 0;
}