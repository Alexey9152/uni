#include <iostream>
#include <fstream>
#include <cstring>
#include <limits>
#include <cstdlib>
#include <string>

struct employee {
    int num;
    char name[10];
    double hours;
};

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Error: Invalid number of arguments." << std::endl;
        std::cerr << "Usage: creator <binary_file> <record_count>" << std::endl;
        return 1;
    }
    const char* filename = argv[1];
    int recordCount = 0;
    try {
        recordCount = std::stoi(argv[2]);
    }
    catch (const std::exception&) {
        std::cerr << "Error: Invalid record count format. Must be an integer." << std::endl;
        return 1;
    }
    if (recordCount <= 0) {
        std::cerr << "Error: Record count must be a positive integer." << std::endl;
        return 1;
    }
    std::ofstream outFile(filename, std::ios::binary);
    if (!outFile) {
        std::cerr << "Error: Cannot create file: " << filename << std::endl;
        return 1;
    }
    std::cout << "Enter data for " << recordCount << " employees:" << std::endl;
    for (int i = 0; i < recordCount; i++) {
        employee emp;
        emp.num = i + 1;
        std::cout << "\nEmployee #" << emp.num << ":" << std::endl;
        while (true) {
            std::cout << "Enter employee name (up to 9 characters): ";
            std::cin.getline(emp.name, 10000);
            if (std::strlen(emp.name) == 0) {
                std::cerr << "Error: Name cannot be empty.\n";
                continue;
            }
            if (std::strlen(emp.name) > 9) {
                std::cerr << "Error: Name must be up to 9 characters.\n";
                continue;
            }
            break;
        }
        std::cout << "Enter hours worked: ";
        while (!(std::cin >> emp.hours) || emp.hours < 0 || emp.hours > 1000) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cerr << "Error: Hours must be a non-negative number. Try again: ";
        }
        outFile.write(reinterpret_cast<const char*>(&emp), sizeof(employee));
        if (outFile.fail()) {
            std::cerr << "Error: Failed to write to file." << std::endl;
            return 1;
        }
    }
    outFile.close();
    if (outFile.fail()) {
        std::cerr << "Error: Failed to close file properly." << std::endl;
        return 1;
    }
    std::cout << "Binary file '" << filename << "' created successfully with " << recordCount << " records." << std::endl;
    return 0;
}