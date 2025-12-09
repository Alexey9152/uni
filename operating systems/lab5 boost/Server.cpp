// Server.cpp
#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <vector>

#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include "protocol.h"

// ---------- Глобальные переменные ----------

std::string g_fileName;
boost::mutex g_fileMutex;   // защита файла
boost::mutex g_lockMutex;   // защита LockManager

struct LockState {
    int  readers = 0;
    bool writer  = false;
};
std::map<int, LockState> g_locks;

// ---------- LockManager (reader-writer по ID) ----------

bool acquireRead(int id) {
    boost::lock_guard<boost::mutex> lg(g_lockMutex);
    LockState& st = g_locks[id];
    if (st.writer) return false;
    ++st.readers;
    return true;
}

void releaseRead(int id) {
    boost::lock_guard<boost::mutex> lg(g_lockMutex);
    auto it = g_locks.find(id);
    if (it == g_locks.end()) return;
    if (it->second.readers > 0) --it->second.readers;
}

bool acquireWrite(int id) {
    boost::lock_guard<boost::mutex> lg(g_lockMutex);
    LockState& st = g_locks[id];
    if (st.writer || st.readers > 0) return false;
    st.writer = true;
    return true;
}

void releaseWrite(int id) {
    boost::lock_guard<boost::mutex> lg(g_lockMutex);
    auto it = g_locks.find(id);
    if (it == g_locks.end()) return;
    it->second.writer = false;
}

// ---------- Работа с файлом employee ----------

bool readEmployeeById(int id, employee& out) {
    boost::lock_guard<boost::mutex> lg(g_fileMutex);
    std::ifstream f(g_fileName, std::ios::binary);
    if (!f) return false;
    employee e;
    while (f.read(reinterpret_cast<char*>(&e), sizeof(e))) {
        if (e.num == id) {
            out = e;
            return true;
        }
    }
    return false;
}

bool writeEmployeeById(int id, const employee& in) {
    boost::lock_guard<boost::mutex> lg(g_fileMutex);
    std::fstream f(g_fileName, std::ios::binary | std::ios::in | std::ios::out);
    if (!f) return false;
    employee e;
    while (f.read(reinterpret_cast<char*>(&e), sizeof(e))) {
        if (e.num == id) {
            std::streampos pos = f.tellg();
            pos -= static_cast<std::streamoff>(sizeof(e));
            f.seekp(pos);
            f.write(reinterpret_cast<const char*>(&in), sizeof(in));
            return true;
        }
    }
    return false;
}

void printFile() {
    boost::lock_guard<boost::mutex> lg(g_fileMutex);
    std::ifstream f(g_fileName, std::ios::binary);
    if (!f) {
        std::cout << "Не удалось открыть файл\n";
        return;
    }
    employee e;
    std::cout << "Содержимое файла:\n";
    while (f.read(reinterpret_cast<char*>(&e), sizeof(e))) {
        std::cout << "num=" << e.num
                  << " name=" << e.name
                  << " hours=" << e.hours << "\n";
    }
}

// ---------- Обработка одного клиента по pipe ----------

void clientSession(HANDLE hPipe) {
    int currentId = -1;
    bool hasReadLock  = false;
    bool hasWriteLock = false;

    Request  req{};
    Response resp{};

    DWORD bytesRead  = 0;
    DWORD bytesWrite = 0;

    bool running = true;

    while (running) {
        if (!ReadFile(hPipe, &req, sizeof(req), &bytesRead, nullptr) || bytesRead == 0) {
            break; // клиент отключился
        }

        resp = {};
        switch (req.command) {
        case CMD_READ: {
            // ждём, пока сможем взять read-lock
            while (!acquireRead(req.id)) {
                Sleep(10);
            }
            hasReadLock = true;
            currentId   = req.id;

            employee e;
            if (readEmployeeById(req.id, e)) {
                resp.status = ST_OK;
                resp.emp    = e;
            } else {
                resp.status = ST_NOT_FOUND;
            }
            WriteFile(hPipe, &resp, sizeof(resp), &bytesWrite, nullptr);
            break;
        }
        case CMD_WRITE_LOCK: {
            // эксклюзивная блокировка
            while (!acquireWrite(req.id)) {
                Sleep(10);
            }
            hasWriteLock = true;
            currentId    = req.id;

            employee e;
            if (readEmployeeById(req.id, e)) {
                resp.status = ST_OK;
                resp.emp    = e;
            } else {
                resp.status = ST_NOT_FOUND;
            }
            WriteFile(hPipe, &resp, sizeof(resp), &bytesWrite, nullptr);
            break;
        }
        case CMD_WRITE_COMMIT: {
            if (hasWriteLock && currentId == req.id) {
                if (writeEmployeeById(req.id, req.emp)) {
                    resp.status = ST_OK;
                } else {
                    resp.status = ST_ERROR;
                }
            } else {
                resp.status = ST_ERROR;
            }
            WriteFile(hPipe, &resp, sizeof(resp), &bytesWrite, nullptr);
            break;
        }
        case CMD_RELEASE: {
            if (hasReadLock && currentId == req.id) {
                releaseRead(currentId);
                hasReadLock = false;
            }
            if (hasWriteLock && currentId == req.id) {
                releaseWrite(currentId);
                hasWriteLock = false;
            }
            currentId = -1;
            // можно отправить простой ACK
            resp.status = ST_OK;
            WriteFile(hPipe, &resp, sizeof(resp), &bytesWrite, nullptr);
            break;
        }
        case CMD_EXIT: {
            // освобождаем, если забыли
            if (hasReadLock && currentId != -1)  releaseRead(currentId);
            if (hasWriteLock && currentId != -1) releaseWrite(currentId);
            running = false;
            break;
        }
        default:
            resp.status = ST_ERROR;
            WriteFile(hPipe, &resp, sizeof(resp), &bytesWrite, nullptr);
            break;
        }
    }

    // страховка
    if (hasReadLock && currentId != -1)  releaseRead(currentId);
    if (hasWriteLock && currentId != -1) releaseWrite(currentId);

    FlushFileBuffers(hPipe);
    DisconnectNamedPipe(hPipe);
    CloseHandle(hPipe);
}

// ---------- main сервера ----------

int main() {
    setlocale(LC_ALL, ".UTF8");
SetConsoleOutputCP(65001);
SetConsoleCP(65001);


    std::cout << "Имя бинарного файла: ";
    std::cin >> g_fileName;

    int n;
    std::cout << "Количество записей: ";
    std::cin >> n;

    {
        std::ofstream f(g_fileName, std::ios::binary);
        if (!f) {
            std::cout << "Не удалось создать файл\n";
            return 1;
        }
        employee e;
        for (int i = 0; i < n; ++i) {
    e.num = i + 1; // ID автоматически: 1..n
    std::cout << "Employee " << e.num << " (name hours): ";
    std::cin >> e.name;
 while (true) {
                std::cin >> e.hours;
                if (!std::cin) {
                    std::cin.clear();
                    std::cin.ignore((std::numeric_limits<std::streamsize>::max)(), '\n');
                    std::cout << "Ошибка ввода. Введите количество часов (>= 0): ";
                    continue;
                }
                if (e.hours < 0.0) {
                    std::cout << "Часы не могут быть отрицательными. Введите заново (>= 0): ";
                    continue;
                }
                break;
            }
    f.write(reinterpret_cast<char*>(&e), sizeof(e));
}
    }

    printFile();

    int clientCount;
    std::cout << "Сколько клиентов запустить: ";
    std::cin >> clientCount;

    // Запуск процессов-клиентов
    std::vector<HANDLE> clientProcesses;
    clientProcesses.reserve(clientCount);

    for (int i = 0; i < clientCount; ++i) {
        STARTUPINFOA si{};
        PROCESS_INFORMATION pi{};
        si.cb = sizeof(si);

        if (CreateProcessA(
            "Client.exe",   // имя exe клиента
            nullptr,
            nullptr, nullptr,
            FALSE,
            CREATE_NEW_CONSOLE,
            nullptr, nullptr,
            &si, &pi))
        {
            clientProcesses.push_back(pi.hProcess);
            CloseHandle(pi.hThread);
        } else {
            std::cout << "Не удалось запустить Client.exe\n";
        }
    }

    // Многопоточный pipe-сервер: отдельный pipe-экземпляр на каждого клиента
    boost::thread acceptThread([] {
        while (true) {
            HANDLE hPipe = CreateNamedPipeA(
                PIPE_NAME,
                PIPE_ACCESS_DUPLEX,
                PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
                PIPE_UNLIMITED_INSTANCES,
                sizeof(Response),
                sizeof(Request),
                0,
                nullptr);

            if (hPipe == INVALID_HANDLE_VALUE) {
                std::cout << "CreateNamedPipe error\n";
                break;
            }

            BOOL connected = ConnectNamedPipe(hPipe, nullptr) ?
                TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);

            if (connected) {
                boost::thread(clientSession, hPipe).detach();
            } else {
                CloseHandle(hPipe);
            }
        }
    });

    // Ждём завершения клиентов
    if (!clientProcesses.empty()) {
        WaitForMultipleObjects(
            static_cast<DWORD>(clientProcesses.size()),
            clientProcesses.data(),
            TRUE,
            INFINITE);
    }

    std::cout << "\nВсе клиенты завершились. Итоговый файл:\n";
    printFile();

    std::cout << "\nНажмите Enter для завершения сервера...\n";
    std::cin.ignore();
    std::cin.get();

    // остановка acceptThread грубо: завершаем процесс, thread умрёт вместе с ним
    return 0;
}
