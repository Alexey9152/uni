#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include <vector>
#include <atomic>
#include <mutex>
#include <limits>

#include "common.h"
#include "lock_manager.h"


static const wchar_t* PIPE_NAME = L"\\\\.\\pipe\\employee_pipe";

LockManager g_lockManager;
std::mutex  g_fileMutex;
std::atomic<bool> g_serverRunning{true};
std::string g_filename;

// Чтение записи по ID из бинарного файла
bool readEmployeeById(int id, employee &out) {
    std::lock_guard<std::mutex> lock(g_fileMutex);
    std::ifstream in(g_filename, std::ios::binary);
    if (!in) return false;
    employee e{};
    while (in.read(reinterpret_cast<char*>(&e), sizeof(e))) {
        if (e.num == id) {
            out = e;
            return true;
        }
    }
    return false;
}

// Перезапись записи по ID в бинарном файле
bool writeEmployeeById(int id, const employee &src) {
    std::lock_guard<std::mutex> lock(g_fileMutex);
    std::fstream io(g_filename, std::ios::binary | std::ios::in | std::ios::out);
    if (!io) return false;
    employee e{};
    while (io.read(reinterpret_cast<char*>(&e), sizeof(e))) {
        if (e.num == id) {
            auto pos = io.tellg();
            pos -= static_cast<std::streamoff>(sizeof(employee));
            io.seekp(pos);
            io.write(reinterpret_cast<const char*>(&src), sizeof(src));
            io.flush();
            return true;
        }
    }
    return false;
}

// Печать всего файла на консоль
void printFile() {
    std::lock_guard<std::mutex> lock(g_fileMutex);
    std::ifstream in(g_filename, std::ios::binary);
    if (!in) {
        std::cout << "Не удалось открыть файл.\n";
        return;
    }
    std::cout << "Содержимое файла:\n";
    employee e{};
    while (in.read(reinterpret_cast<char*>(&e), sizeof(e))) {
        std::cout << "ID: "   << e.num
                  << " Name: " << e.name
                  << " Hours: " << e.hours << "\n";
    }
}

bool sendResponse(HANDLE pipe, const Response &resp) {
    DWORD written = 0;
    BOOL ok = WriteFile(pipe, &resp, sizeof(resp), &written, nullptr);
    return ok && written == sizeof(resp);
}

bool recvRequest(HANDLE pipe, Request &req) {
    DWORD read = 0;
    BOOL ok = ReadFile(pipe, &req, sizeof(req), &read, nullptr);
    return ok && read == sizeof(req);
}

// Обработчик одного клиентского подключения
void clientThread(HANDLE pipe) {
    while (g_serverRunning) {
        Request req{};
        if (!recvRequest(pipe, req)) {
            break; // клиент отключился
        }

        Response resp{};
        resp.ok = false;
        std::snprintf(resp.message, sizeof(resp.message), "Ошибка");

        switch (req.op) {
        case Operation::READ_REQUEST: {
            // захват блокировки чтения (читатели совместимы, писатель блокирует)
            while (!g_lockManager.acquireRead(req.id) && g_serverRunning) {
                Sleep(10);
            }
            employee e{};
            if (readEmployeeById(req.id, e)) {
                resp.ok = true;
                resp.data = e;
                std::snprintf(resp.message, sizeof(resp.message), "READ OK");
            } else {
                g_lockManager.releaseRead(req.id);
                std::snprintf(resp.message, sizeof(resp.message), "ID_NOT_FOUND");
            }
            sendResponse(pipe, resp);
            break;
        }
        case Operation::WRITE_BEGIN: {
            // захват блокировки записи (эксклюзивный доступ)
            while (!g_lockManager.acquireWrite(req.id) && g_serverRunning) {
                Sleep(10);
            }
            employee e{};
            if (readEmployeeById(req.id, e)) {
                resp.ok = true;
                resp.data = e;
                std::snprintf(resp.message, sizeof(resp.message), "WRITE BEGIN OK");
            } else {
                g_lockManager.releaseWrite(req.id);
                std::snprintf(resp.message, sizeof(resp.message), "ID_NOT_FOUND");
            }
            sendResponse(pipe, resp);
            break;
        }
        case Operation::WRITE_COMMIT: {
            if (writeEmployeeById(req.id, req.data)) {
                resp.ok = true;
                resp.data = req.data;
                std::snprintf(resp.message, sizeof(resp.message), "WRITE COMMIT OK");
            } else {
                std::snprintf(resp.message, sizeof(resp.message), "WRITE_ERROR");
            }
            sendResponse(pipe, resp);
            break;
        }
        case Operation::RELEASE: {
            // Освобождаем возможную блокировку чтения и/или записи
            g_lockManager.releaseWrite(req.id);
            g_lockManager.releaseRead(req.id);
            resp.ok = true;
            std::snprintf(resp.message, sizeof(resp.message), "RELEASE OK");
            sendResponse(pipe, resp);
            break;
        }
        case Operation::SHUTDOWN_SERVER: {
            g_serverRunning = false;
            resp.ok = true;
            std::snprintf(resp.message, sizeof(resp.message), "SERVER SHUTDOWN");
            sendResponse(pipe, resp);
            break;
        }
        default:
            std::snprintf(resp.message, sizeof(resp.message), "UNKNOWN_OPERATION");
            sendResponse(pipe, resp);
            break;
        }
    }
    FlushFileBuffers(pipe);
    DisconnectNamedPipe(pipe);
    CloseHandle(pipe);
}

int main() {
    std::cout << "Введите имя бинарного файла: ";
    std::cin >> g_filename;

    int n;
    while (true) {
    std::cout << "Введите количество сотрудников (> 0): ";
    std::cin >> n;

    if (!std::cin) {
        std::cin.clear();
        std::cin.ignore((std::numeric_limits<std::streamsize>::max)(), '\n');
        std::cout << "Ошибка ввода. Нужно целое число.\n";
        continue;
    }
    if (n <= 0) {
        std::cout << "Количество сотрудников должно быть больше 0.\n";
        continue;
    }
    break;
}

    {
        std::ofstream out(g_filename, std::ios::binary);
        if (!out) {
            std::cerr << "Не удалось создать файл.\n";
            return 1;
        }
        for (int i = 0; i < n; ++i) {
            employee e{};
            e.num = i + 1; // ID назначается автоматически

            std::cout << "Сотрудник #" << (i + 1) << " (name, hours>=0): ";
            std::cin >> e.name;

            // Валидация количества часов: неотрицательное double
            while (true) {
                std::cin >> e.hours;
                if (!std::cin) {
                    std::cin.clear();
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                    std::cout << "Ошибка ввода. Введите количество часов (>= 0): ";
                    continue;
                }
                if (e.hours < 0.0) {
                    std::cout << "Часы не могут быть отрицательными. Введите заново (>= 0): ";
                    continue;
                }
                break;
            }

            out.write(reinterpret_cast<const char*>(&e), sizeof(e));
        }
    }

    printFile();

    // Запуск клиентских процессов
    int clientCount = 0;
    while (true) {
    std::cout << "Введите количество клиентских процессов (> 0): ";
    std::cin >> clientCount;

    if (!std::cin) {
        std::cin.clear();
        std::cin.ignore((std::numeric_limits<std::streamsize>::max)(), '\n');
        std::cout << "Ошибка ввода. Нужно целое число.\n";
        continue;
    }
    if (clientCount <= 0) {
        std::cout << "Количество клиентов должно быть больше 0.\n";
        continue;
    }
    break;
}

    std::wstring clientPath = L"client.exe";
    std::vector<PROCESS_INFORMATION> clientProcesses;
    clientProcesses.reserve(clientCount);

    for (int i = 0; i < clientCount; ++i) {
    STARTUPINFOW si{};
    si.cb = sizeof(si);
    PROCESS_INFORMATION pi{};

    BOOL ok = CreateProcessW(
        clientPath.c_str(),   // lpApplicationName
        nullptr,              // lpCommandLine
        nullptr,              // lpProcessAttributes
        nullptr,              // lpThreadAttributes
        FALSE,                // bInheritHandles
        CREATE_NEW_CONSOLE,   // dwCreationFlags  <<< ВАЖНО
        nullptr,              // lpEnvironment
        nullptr,              // lpCurrentDirectory
        &si,
        &pi
    );
    if (!ok) {
        std::cerr << "Не удалось запустить client.exe, код ошибки: "
                  << GetLastError() << "\n";
    } else {
        clientProcesses.push_back(pi);
        std::cout << "Клиент #" << (i + 1) << " запущен.\n";
    }
}

    std::cout << "Сервер запущен. Ожидание клиентов...\n";

    // Поток для команд с консоли: печать файла / завершение
    std::thread consoleThread([] {
    while (g_serverRunning) {
        std::cout << "Команды сервера: p - print file, q - quit\n";
        char cmd;
        std::cin >> cmd;
        if (cmd == 'p') {
            printFile();
        } else if (cmd == 'q') {
            // Сигнал остановки сервера
            g_serverRunning = false;

            // Фиктивное подключение к каналу, чтобы разбудить ConnectNamedPipe
            HANDLE h = CreateFileW(
                PIPE_NAME,
                GENERIC_READ | GENERIC_WRITE,
                0,
                nullptr,
                OPEN_EXISTING,
                0,
                nullptr
            );
            if (h != INVALID_HANDLE_VALUE) {
                CloseHandle(h);
            }

            break;
        }
    }
});

    std::vector<std::thread> workers;

    // Главный цикл: создавать новые экземпляры канала и принимать клиентов
    while (g_serverRunning) {
        HANDLE pipe = CreateNamedPipeW(
            PIPE_NAME,
            PIPE_ACCESS_DUPLEX,
            PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,
            PIPE_UNLIMITED_INSTANCES,
            sizeof(Response),
            sizeof(Request),
            0,
            nullptr
        );

        if (pipe == INVALID_HANDLE_VALUE) {
            std::cerr << "CreateNamedPipeW error: " << GetLastError() << "\n";
            break;
        }

        BOOL connected = ConnectNamedPipe(pipe, nullptr) ?
                         TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);

        if (connected) {
            workers.emplace_back(clientThread, pipe);
        } else {
            CloseHandle(pipe);
        }
    }

    // Дожидаемся завершения потоков
    for (auto &t : workers) {
        if (t.joinable()) t.join();
    }
    if (consoleThread.joinable()) consoleThread.join();

    // Дожидаемся завершения клиентских процессов
    for (auto &pi : clientProcesses) {
        if (pi.hProcess) {
            WaitForSingleObject(pi.hProcess, INFINITE);
            CloseHandle(pi.hProcess);
        }
        if (pi.hThread) {
            CloseHandle(pi.hThread);
        }
    }

    std::cout << "Итоговое содержимое файла:\n";
    printFile();

    std::cout << "Сервер завершён.\n";
    return 0;
}
