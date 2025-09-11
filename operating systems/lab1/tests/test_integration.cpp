#include <gtest/gtest.h>
#include <fstream>
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include "employee.h"

#ifdef _WIN32
#include <process.h>
#endif

#ifndef TEST_BIN_DIR
#error "TEST_BIN_DIR must be defined by CMake"
#endif

static std::string BIN_DIR = TEST_BIN_DIR;

// Универсальный запуск exe с аргументами
static int runProcess(const std::string &exe, const std::vector<std::string> &args, const std::string &inputFile = "") {
    if (!inputFile.empty()) {
        // Режим с перенаправлением ввода
        std::string cmd;
#ifdef _WIN32
        cmd = "cmd.exe /C \"";
#else
        cmd = "";
#endif
        cmd += "\"" + exe + "\"";
        for (auto &a : args) {
            cmd += " \"" + a + "\"";
        }
        cmd += " < \"" + inputFile + "\"";
#ifdef _WIN32
        cmd += "\"";
#endif
        return std::system(cmd.c_str());
    } else {
        // Старая логика без перенаправления
#ifdef _WIN32
        switch (args.size()) {
            case 0:
                return _spawnl(_P_WAIT, exe.c_str(), exe.c_str(), nullptr);
            case 1:
                return _spawnl(_P_WAIT, exe.c_str(), exe.c_str(),
                               args[0].c_str(), nullptr);
            case 2:
                return _spawnl(_P_WAIT, exe.c_str(), exe.c_str(),
                               args[0].c_str(), args[1].c_str(), nullptr);
            case 3:
                return _spawnl(_P_WAIT, exe.c_str(), exe.c_str(),
                               args[0].c_str(), args[1].c_str(),
                               args[2].c_str(), nullptr);
            default:
                return -1;
        }
#else
        std::string cmd = "\"" + exe + "\"";
        for (auto &a : args) {
            cmd += " \"" + a + "\"";
        }
        return std::system(cmd.c_str());
#endif
    }
}

// --- ТЕСТ REPORTER ---
TEST(Reporter, ProducesCorrectReport) {
    std::string binPath = BIN_DIR + "/test_reporter.bin";
    std::string reportPath = BIN_DIR + "/test_reporter.txt";

    // создаём бинарный файл вручную
    {
        std::ofstream fout(binPath, std::ios::binary);
        employee e;
        e.num = 1;
        std::strncpy(e.name, "Bob", sizeof(e.name));
        e.hours = 10.0;
        fout.write(reinterpret_cast<const char*>(&e), sizeof(e));
    }

    std::string reporterExe = BIN_DIR + "/reporter.exe";
    std::vector<std::string> args = { binPath, "15", reportPath };

    int rc = runProcess(reporterExe, args);
    ASSERT_EQ(rc, 0) << "reporter.exe failed";

    std::ifstream rf(reportPath);
    ASSERT_TRUE(rf.is_open());
    std::string content((std::istreambuf_iterator<char>(rf)), {});
    rf.close();

    EXPECT_NE(content.find("Bob"), std::string::npos);
    EXPECT_NE(content.find("150.00"), std::string::npos);

    std::filesystem::remove(binPath);
    std::filesystem::remove(reportPath);
}

// --- ТЕСТ CREATOR ---
TEST(Creator, CreatesBinary) {
    std::string binPath = BIN_DIR + "/test_creator.bin";
    std::string inputPath = BIN_DIR + "/test_creator_input.txt";

    // Создаём файл с входными данными
    {
        std::ofstream fin(inputPath);
        fin << "Alice\n8.5\n";
    }

    std::string creatorExe = BIN_DIR + "/creator.exe";
    std::vector<std::string> args = { binPath, "1" };

    int rc = runProcess(creatorExe, args, inputPath);
    ASSERT_EQ(rc, 0) << "creator.exe failed";

    // Проверяем созданный бинарный файл
    std::ifstream binFile(binPath, std::ios::binary);
    ASSERT_TRUE(binFile.is_open());
    employee e;
    binFile.read(reinterpret_cast<char*>(&e), sizeof(e));
    binFile.close();

    EXPECT_EQ(e.num, 1);
    EXPECT_STREQ(e.name, "Alice");
    EXPECT_DOUBLE_EQ(e.hours, 8.5);

    std::filesystem::remove(binPath);
    std::filesystem::remove(inputPath);
}



TEST(Integration, FullWorkflow) {
    std::string binPath    = BIN_DIR + "/test_integration.bin";
    std::string reportPath = BIN_DIR + "/test_integration.txt";
    std::string inputPath  = BIN_DIR + "/test_integration_input.txt";

    // Создаём входные данные для creator.exe
    {
        std::ofstream fin(inputPath);
        fin << "Alice\n8.5\nBob\n10\n";
    }

    std::string mainappExe = BIN_DIR + "/mainapp.exe";
    std::vector<std::string> args = { binPath, "2", reportPath, "15" };

    // Убираем паузу в конце
    #ifdef _WIN32
    _putenv_s("NO_PAUSE", "1");
    #else
    setenv("NO_PAUSE", "1", 1);
    #endif

    int rc = runProcess(mainappExe, args, inputPath);
    ASSERT_EQ(rc, 0) << "mainapp.exe failed";

    // Проверка бинарника
    {
        std::ifstream binFile(binPath, std::ios::binary);
        ASSERT_TRUE(binFile.is_open());
        employee e1, e2;
        binFile.read(reinterpret_cast<char*>(&e1), sizeof(e1));
        binFile.read(reinterpret_cast<char*>(&e2), sizeof(e2));
        EXPECT_STREQ(e1.name, "Alice");
        EXPECT_DOUBLE_EQ(e1.hours, 8.5);
        EXPECT_STREQ(e2.name, "Bob");
        EXPECT_DOUBLE_EQ(e2.hours, 10.0);
    }

    // Проверка отчёта
    {
        std::ifstream rf(reportPath);
        ASSERT_TRUE(rf.is_open());
        std::string content((std::istreambuf_iterator<char>(rf)), {});
        EXPECT_NE(content.find("Alice"), std::string::npos);
        EXPECT_NE(content.find("127.50"), std::string::npos); // 8.5 * 15
        EXPECT_NE(content.find("Bob"), std::string::npos);
        EXPECT_NE(content.find("150.00"), std::string::npos); // 10 * 15
    }

    // Чистим
    #ifdef _WIN32
    _putenv_s("NO_PAUSE", "");
    #else
    unsetenv("NO_PAUSE");
    #endif
    std::filesystem::remove(binPath);
    std::filesystem::remove(reportPath);
    std::filesystem::remove(inputPath);
}
