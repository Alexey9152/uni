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

void printFile() {
    std::lock_guard<std::mutex> lock(g_fileMutex);
    std::ifstream in(g_filename, std::ios::binary);
    if (!in) {
        std::cout << "¥ ã¤ «®áì ®âªàëâì ä ©«.\n";
        return;
    }
    std::cout << "‘®¤¥à¦¨¬®¥ ä ©« :\n";
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

void clientThread(HANDLE pipe) {
    while (g_serverRunning) {
        Request req{};
        if (!recvRequest(pipe, req)) {
            break; 
        }

        Response resp{};
        resp.ok = false;
        std::snprintf(resp.message, sizeof(resp.message), "Žè¨¡ª ");

        switch (req.op) {
        case Operation::READ_REQUEST: {
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
    std::cout << "‚¢¥¤¨â¥ ¨¬ï ¡¨­ à­®£® ä ©« : ";
    std::cin >> g_filename;

    int n;
    while (true) {
    std::cout << "‚¢¥¤¨â¥ ª®«¨ç¥áâ¢® á®âàã¤­¨ª®¢ (> 0): ";
    std::cin >> n;

    if (!std::cin) {
        std::cin.clear();
        std::cin.ignore((std::numeric_limits<std::streamsize>::max)(), '\n');
        std::cout << "Žè¨¡ª  ¢¢®¤ . ã¦­® æ¥«®¥ ç¨á«®.\n";
        continue;
    }
    if (n <= 0) {
        std::cout << "Š®«¨ç¥áâ¢® á®âàã¤­¨ª®¢ ¤®«¦­® ¡ëâì ¡®«ìè¥ 0.\n";
        continue;
    }
    break;
}

    {
        std::ofstream out(g_filename, std::ios::binary);
        if (!out) {
            std::cerr << "¥ ã¤ «®áì á®§¤ âì ä ©«.\n";
            return 1;
        }
        for (int i = 0; i < n; ++i) {
            employee e{};
            e.num = i + 1;

            std::cout << "‘®âàã¤­¨ª #" << (i + 1) << " (name, hours>=0): ";
            std::cin >> e.name;
            while (true) {
                std::cin >> e.hours;
                if (!std::cin) {
                    std::cin.clear();
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                    std::cout << "Žè¨¡ª  ¢¢®¤ . ‚¢¥¤¨â¥ ª®«¨ç¥áâ¢® ç á®¢ (>= 0): ";
                    continue;
                }
                if (e.hours < 0.0) {
                    std::cout << "— áë ­¥ ¬®£ãâ ¡ëâì ®âà¨æ â¥«ì­ë¬¨. ‚¢¥¤¨â¥ § ­®¢® (>= 0): ";
                    continue;
                }
                break;
            }

            out.write(reinterpret_cast<const char*>(&e), sizeof(e));
        }
    }

    printFile();

    int clientCount = 0;
    while (true) {
    std::cout << "‚¢¥¤¨â¥ ª®«¨ç¥áâ¢® ª«¨¥­âáª¨å ¯à®æ¥áá®¢ (> 0): ";
    std::cin >> clientCount;

    if (!std::cin) {
        std::cin.clear();
        std::cin.ignore((std::numeric_limits<std::streamsize>::max)(), '\n');
        std::cout << "Žè¨¡ª  ¢¢®¤ . ã¦­® æ¥«®¥ ç¨á«®.\n";
        continue;
    }
    if (clientCount <= 0) {
        std::cout << "Š®«¨ç¥áâ¢® ª«¨¥­â®¢ ¤®«¦­® ¡ëâì ¡®«ìè¥ 0.\n";
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
        CREATE_NEW_CONSOLE,   // dwCreationFlags  <<< ‚€†Ž
        nullptr,              // lpEnvironment
        nullptr,              // lpCurrentDirectory
        &si,
        &pi
    );
    if (!ok) {
        std::cerr << "¥ ã¤ «®áì § ¯ãáâ¨âì client.exe, ª®¤ ®è¨¡ª¨: "
                  << GetLastError() << "\n";
    } else {
        clientProcesses.push_back(pi);
        std::cout << "Š«¨¥­â #" << (i + 1) << " § ¯ãé¥­.\n";
    }
}

    std::cout << "‘¥à¢¥à § ¯ãé¥­. Ž¦¨¤ ­¨¥ ª«¨¥­â®¢...\n";

    std::thread consoleThread([] {
    while (g_serverRunning) {
        std::cout << "Š®¬ ­¤ë á¥à¢¥à : p - print file, q - quit\n";
        char cmd;
        std::cin >> cmd;
        if (cmd == 'p') {
            printFile();
        } else if (cmd == 'q') {
            g_serverRunning = false;

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

    for (auto &t : workers) {
        if (t.joinable()) t.join();
    }
    if (consoleThread.joinable()) consoleThread.join();
    for (auto &pi : clientProcesses) {
        if (pi.hProcess) {
            WaitForSingleObject(pi.hProcess, INFINITE);
            CloseHandle(pi.hProcess);
        }
        if (pi.hThread) {
            CloseHandle(pi.hThread);
        }
    }

    std::cout << "ˆâ®£®¢®¥ á®¤¥à¦¨¬®¥ ä ©« :\n";
    printFile();

    std::cout << "‘¥à¢¥à § ¢¥àèñ­.\n";
    return 0;
}
