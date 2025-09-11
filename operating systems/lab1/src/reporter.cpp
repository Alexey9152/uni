#include <iostream>
#include <fstream>
#include <iomanip>
#include <cstring>
#include <limits>
#include <string>

struct employee {
    int num;
    char name[10];
    double hours;
};

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cerr << "Error: Invalid number of arguments." << std::endl;
        std::cerr << "Usage: reporter <binary_file> <hourly_payment> <report_file>" << std::endl;
        return 1;
    }
    const char* binaryFilename = argv[1];
    double hourlyPayment = 0.0;
    try {
        hourlyPayment = std::stod(argv[2]);
    }
    catch (const std::exception&) {
        std::cerr << "Error: Invalid hourly payment format. Must be a number." << std::endl;
        return 1;
    }
    if (hourlyPayment < 0) {
        std::cerr << "Error: Hourly payment cannot be negative." << std::endl;
        return 1;
    }
    const char* reportFilename = argv[3];
    std::ifstream inFile(binaryFilename, std::ios::binary);
    if (!inFile) {
        std::cerr << "Error: Cannot open binary file: " << binaryFilename << std::endl;
        return 1;
    }
    inFile.seekg(0, std::ios::end);
    if (inFile.tellg() == 0) {
        std::cerr << "Error: Binary file is empty." << std::endl;
        inFile.close();
        return 1;
    }
    inFile.seekg(0, std::ios::beg);
    std::ofstream outFile(reportFilename);
    if (!outFile) {
        std::cerr << "Error: Cannot create report file: " << reportFilename << std::endl;
        inFile.close();
        return 1;
    }
    outFile << "Report for file \"" << binaryFilename << "\"\n";
    outFile << "Employee number\tName\tHours\tSalary\n";
    employee emp;
    bool hasRecords = false;
    while (inFile.read(reinterpret_cast<char*>(&emp), sizeof(employee))) {
        hasRecords = true;
        if (emp.num <= 0 || emp.hours < 0) {
            std::cerr << "Error: Invalid data in binary file." << std::endl;
            inFile.close();
            outFile.close();
            return 1;
        }
        double salary = emp.hours * hourlyPayment;
        outFile << emp.num << "\t\t" << emp.name << "\t" << std::fixed << std::setprecision(2) << emp.hours << "\t" << std::fixed << std::setprecision(2) << salary << "\n";
        if (outFile.fail()) {
            std::cerr << "Error: Failed to write to report file." << std::endl;
            inFile.close();
            outFile.close();
            return 1;
        }
    }
    if (!hasRecords) {
        std::cerr << "Error: No valid records found in binary file." << std::endl;
        inFile.close();
        outFile.close();
        return 1;
    }
    if (!inFile.eof() && inFile.fail()) {
        std::cerr << "Error: Failed to read from binary file." << std::endl;
        inFile.close();
        outFile.close();
        return 1;
    }
    inFile.close();
    outFile.close();
    std::cout << "Report file '" << reportFilename << "' created successfully." << std::endl;
    return 0;
}