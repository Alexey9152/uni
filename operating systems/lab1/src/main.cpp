#include <iostream>
#include <fstream>
#define NOMINMAX
#include <windows.h>
#include <cstring>
#include <sstream>
#include <iomanip>
#include <limits>
#include <algorithm>
#include <cstdlib>

struct employee {
    int num;
    char name[10];
    double hours;
};

void printBinaryFile(const char* filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "Error: Cannot open binary file: " << filename << std::endl;
        return;
    }
    file.seekg(0, std::ios::end);
    if (file.tellg() == 0) {
        std::cerr << "Error: Binary file is empty." << std::endl;
        file.close();
        return;
    }
    file.seekg(0, std::ios::beg);
    std::cout << "\nContent of binary file '" << filename << "':" << std::endl;
    std::cout << "Number\tName\tHours" << std::endl;
    employee emp;
    while (file.read(reinterpret_cast<char*>(&emp), sizeof(employee))) {
        std::cout << emp.num << "\t" << emp.name << "\t" << emp.hours << std::endl;
    }
    if (!file.eof() && file.fail()) {
        std::cerr << "Error: Failed to read from binary file." << std::endl;
    }
    file.close();
}

void printReport(const char* filename) {
    std::ifstream file(filename);
    if (!file) {
        std::cerr << "Error: Cannot open report file: " << filename << std::endl;
        return;
    }
    file.seekg(0, std::ios::end);
    if (file.tellg() == 0) {
        std::cerr << "Error: Report file is empty." << std::endl;
        file.close();
        return;
    }
    file.seekg(0, std::ios::beg);
    std::cout << "\nContent of report file '" << filename << "':" << std::endl;
    std::string line;
    while (std::getline(file, line)) {
        std::cout << line << std::endl;
    }
    file.close();
}

bool runProcess(const char* command, DWORD& exitCode) {
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    char cmd[256];
    strncpy_s(cmd, command, _TRUNCATE);

    if (!CreateProcessA(nullptr, cmd, nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi)) {
        std::cerr << "Error: CreateProcess failed. Error code: " << GetLastError() << std::endl;
        return false;
    }

    WaitForSingleObject(pi.hProcess, INFINITE);
    
    if (!GetExitCodeProcess(pi.hProcess, &exitCode)) {
        std::cerr << "Error: Failed to get exit code. Error code: " << GetLastError() << std::endl;
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        return false;
    }

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return true;
}

bool getFilenameWithExtension(char* buffer, const char* extension, const char* prompt) {
    while (true) {
        std::cout << prompt;
        if (!(std::cin >> buffer)) {
            std::cerr << "Error: Failed to read file name." << std::endl;
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            continue;
        }
        size_t len = std::strlen(buffer);
        if (len >= 4 && std::strcmp(buffer + len - 4, extension) == 0) {
            return true;
        }
        std::cerr << "Error: File name must end with " << extension << std::endl;
    }
}

bool handleCreator(const char* binaryFileName, int recordCount, bool interactive) {
    std::stringstream creatorCmd;
    creatorCmd << "creator.exe " << binaryFileName << " " << recordCount;
    
    DWORD exitCode;
    if (!runProcess(creatorCmd.str().c_str(), exitCode)) {
        return false;
    }

    if (exitCode != 0) {
        std::cerr << "Error: Creator failed with exit code " << exitCode << std::endl;
        return false;
    }

    if (interactive) {
        printBinaryFile(binaryFileName);
    }
    return true;
}

bool handleReporter(const char* binaryFileName, const char* reportFileName, double hourlyPayment, bool interactive) {
    std::stringstream reporterCmd;
    reporterCmd << "reporter.exe " << binaryFileName << " " 
                << std::fixed << std::setprecision(2) << hourlyPayment << " " 
                << reportFileName;
    
    DWORD exitCode;
    if (!runProcess(reporterCmd.str().c_str(), exitCode)) {
        return false;
    }

    if (exitCode != 0) {
        std::cerr << "Error: Reporter failed with exit code " << exitCode << std::endl;
        return false;
    }

    if (interactive) {
        printReport(reportFileName);
    }
    return true;
}

int main(int argc, char* argv[]) {
    char binaryFileName[100];
    char reportFileName[100];
    int recordCount;
    double hourlyPayment;

    if (argc == 5) {
        // Режим командной строки
        strncpy_s(binaryFileName, argv[1], _TRUNCATE);
        recordCount = std::atoi(argv[2]);
        strncpy_s(reportFileName, argv[3], _TRUNCATE);
        hourlyPayment = std::atof(argv[4]);

        if (!handleCreator(binaryFileName, recordCount, false) ||
            !handleReporter(binaryFileName, reportFileName, hourlyPayment, false)) {
            return 1;
        }

        std::cout << "\nProgram completed successfully." << std::endl;
        return 0;
    }

    // Интерактивный режим
    getFilenameWithExtension(binaryFileName, ".bin", "Enter binary file name (with .bin extension): ");

    std::cout << "Enter record count: ";
    while (!(std::cin >> recordCount) || recordCount <= 0) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cerr << "Error: Record count must be a positive integer. Try again: ";
    }

    if (!handleCreator(binaryFileName, recordCount, true)) {
        return 1;
    }

    getFilenameWithExtension(reportFileName, ".txt", "Enter report file name (with .txt extension): ");

    std::cout << "\nEnter hourly payment: ";
    while (!(std::cin >> hourlyPayment) || hourlyPayment < 0) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cerr << "Error: Hourly payment must be a non-negative number. Try again: ";
    }

    if (!handleReporter(binaryFileName, reportFileName, hourlyPayment, true)) {
        return 1;
    }

    std::cout << "\nProgram completed successfully." << std::endl;
    
    if (std::getenv("NO_PAUSE") == nullptr) {
        system("pause");
    }
    return 0;
}
