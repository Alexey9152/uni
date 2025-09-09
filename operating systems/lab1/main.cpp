#include <iostream>
#include <fstream>
#define NOMINMAX
#include <windows.h>
#include <cstring>
#include <sstream>
#include <iomanip>
#include <limits>
#include <algorithm>

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

int main() {
    char binaryFileName[100];
    char reportFileName[100];
    int recordCount = 0;
    while (true) {
        std::cout << "Enter binary file name (with .bin extension): ";
        if (!(std::cin >> binaryFileName)) {
            std::cerr << "Error: Failed to read file name." << std::endl;
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            continue;
        }
        size_t len = std::strlen(binaryFileName);
        if (len >= 4 && std::strcmp(binaryFileName + len - 4, ".bin") == 0) {
            break;
        }
        std::cerr << "Error: File name must end with .bin" << std::endl;
    }
    std::cout << "Enter record count: ";
    while (!(std::cin >> recordCount) || recordCount <= 0) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cerr << "Error: Record count must be a positive integer. Try again: ";
    }
    STARTUPINFOA siCreator;
    PROCESS_INFORMATION piCreator;
    ZeroMemory(&siCreator, sizeof(siCreator));
    siCreator.cb = sizeof(siCreator);
    ZeroMemory(&piCreator, sizeof(piCreator));
    std::stringstream creatorCmd;
    creatorCmd << "creator.exe " << binaryFileName << " " << recordCount;
    std::string creatorCmdStr = creatorCmd.str();
    char cmdCreator[200];
    strcpy_s(cmdCreator, creatorCmdStr.c_str());
    if (!CreateProcessA(nullptr, cmdCreator, nullptr, nullptr, FALSE, 0, nullptr, nullptr, &siCreator, &piCreator)) {
        std::cerr << "Error: CreateProcess failed for Creator. Error code: " << GetLastError() << std::endl;
        return 1;
    }
    DWORD waitResult = WaitForSingleObject(piCreator.hProcess, INFINITE);
    if (waitResult != WAIT_OBJECT_0) {
        std::cerr << "Error: WaitForSingleObject failed for Creator. Error code: " << GetLastError() << std::endl;
        CloseHandle(piCreator.hProcess);
        CloseHandle(piCreator.hThread);
        return 1;
    }
    DWORD exitCodeCreator;
    if (!GetExitCodeProcess(piCreator.hProcess, &exitCodeCreator)) {
        std::cerr << "Error: Failed to get exit code from Creator. Error code: " << GetLastError() << std::endl;
        CloseHandle(piCreator.hProcess);
        CloseHandle(piCreator.hThread);
        return 1;
    }
    CloseHandle(piCreator.hProcess);
    CloseHandle(piCreator.hThread);
    if (exitCodeCreator != 0) {
        std::cerr << "Error: Creator failed with exit code " << exitCodeCreator << std::endl;
        return 1;
    }
    printBinaryFile(binaryFileName);
    while (true) {
        std::cout << "Enter report file name (with .txt extension): ";
        if (!(std::cin >> reportFileName)) {
            std::cerr << "Error: Failed to read file name." << std::endl;
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            continue;
        }
        size_t len = std::strlen(reportFileName);
        if (len >= 4 && std::strcmp(reportFileName + len - 4, ".txt") == 0) {
            break;
        }
        std::cerr << "Error: File name must end with .txt" << std::endl;
    }
    double hourlyPayment = 0.0;
    std::cout << "\nEnter hourly payment: ";
    while (!(std::cin >> hourlyPayment) || hourlyPayment < 0) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cerr << "Error: Hourly payment must be a non-negative number. Try again: ";
    }
    STARTUPINFOA siReporter;
    PROCESS_INFORMATION piReporter;
    ZeroMemory(&siReporter, sizeof(siReporter));
    siReporter.cb = sizeof(siReporter);
    ZeroMemory(&piReporter, sizeof(piReporter));
    std::stringstream reporterCmd;
    reporterCmd << "reporter.exe " << binaryFileName << " " << std::fixed << std::setprecision(2) << hourlyPayment << " " << reportFileName;
    std::string reporterCmdStr = reporterCmd.str();
    char cmdReporter[200];
    strcpy_s(cmdReporter, reporterCmdStr.c_str());
    if (!CreateProcessA(nullptr, cmdReporter, nullptr, nullptr, FALSE, 0, nullptr, nullptr, &siReporter, &piReporter)) {
        std::cerr << "Error: CreateProcess failed for Reporter. Error code: " << GetLastError() << std::endl;
        return 1;
    }
    waitResult = WaitForSingleObject(piReporter.hProcess, INFINITE);
    if (waitResult != WAIT_OBJECT_0) {
        std::cerr << "Error: WaitForSingleObject failed for Reporter. Error code: " << GetLastError() << std::endl;
        CloseHandle(piReporter.hProcess);
        CloseHandle(piReporter.hThread);
        return 1;
    }
    DWORD exitCodeReporter;
    if (!GetExitCodeProcess(piReporter.hProcess, &exitCodeReporter)) {
        std::cerr << "Error: Failed to get exit code from Reporter. Error code: " << GetLastError() << std::endl;
        CloseHandle(piReporter.hProcess);
        CloseHandle(piReporter.hThread);
        return 1;
    }
    CloseHandle(piReporter.hProcess);
    CloseHandle(piReporter.hThread);
    if (exitCodeReporter != 0) {
        std::cerr << "Error: Reporter failed with exit code " << exitCodeReporter << std::endl;
        return 1;
    }
    printReport(reportFileName);
    std::cout << "\nProgram completed successfully." << std::endl;
    system("pause");
    return 0;
}