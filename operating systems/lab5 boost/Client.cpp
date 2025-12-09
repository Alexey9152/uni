// Client.cpp
#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <iostream>
#include <string>
#include "protocol.h"

bool sendRequest(HANDLE hPipe, const Request& req, Response& resp, bool expectResp = true) {
    DWORD written = 0;
    if (!WriteFile(hPipe, &req, sizeof(req), &written, nullptr) || written != sizeof(req)) {
        std::cout << "WriteFile error\n";
        return false;
    }
    if (!expectResp) return true;
    DWORD read = 0;
    if (!ReadFile(hPipe, &resp, sizeof(resp), &read, nullptr) || read == 0) {
        std::cout << "ReadFile error\n";
        return false;
    }
    return true;
}

void do_read(HANDLE hPipe) {
    int id;
    std::cout << "ID сотрудника для чтения: ";
    std::cin >> id;

    Request  req{};
    Response resp{};

    req.command = CMD_READ;
    req.id      = id;

    if (!sendRequest(hPipe, req, resp)) return;

    if (resp.status == ST_OK) {
        std::cout << "Получена запись: num=" << resp.emp.num
                  << " name=" << resp.emp.name
                  << " hours=" << resp.emp.hours << "\n";
    } else {
        std::cout << "Запись не найдена\n";
    }

    std::cout << "Нажмите Enter для завершения доступа к записи...\n";
    std::cin.ignore();
    std::cin.get();

    req = {};
    req.command = CMD_RELEASE;
    req.id      = id;
    sendRequest(hPipe, req, resp, true);
}

void do_modify(HANDLE hPipe) {
    int id;
    std::cout << "ID сотрудника для модификации: ";
    std::cin >> id;

    Request  req{};
    Response resp{};

    // 1) Захватываем запись на запись и читаем текущие данные
    req.command = CMD_WRITE_LOCK;
    req.id      = id;

    if (!sendRequest(hPipe, req, resp)) return;

    if (resp.status == ST_OK) {
        std::cout << "Текущая запись: num=" << resp.emp.num
                  << " name=" << resp.emp.name
                  << " hours=" << resp.emp.hours << "\n";
    } else {
        std::cout << "Запись не найдена, модификация невозможна\n";
        // снимаем lock на всякий случай
        req = {};
        req.command = CMD_RELEASE;
        req.id      = id;
        sendRequest(hPipe, req, resp, true);
        return;
    }

    // 2) Вводим новые значения
    employee e = resp.emp;
    std::cout << "Введите новые name hours: ";
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

    std::cout << "Отправить изменения на сервер? (y/n): ";
    char ch;
    std::cin >> ch;
    if (ch == 'y' || ch == 'Y') {
        req = {};
        req.command = CMD_WRITE_COMMIT;
        req.id      = id;
        req.emp     = e;

        if (!sendRequest(hPipe, req, resp)) return;
        if (resp.status == ST_OK) {
            std::cout << "Изменения сохранены\n";
        } else {
            std::cout << "Ошибка при записи\n";
        }
    } else {
        std::cout << "Изменения отменены (но запись всё ещё заблокирована)\n";
    }

    std::cout << "Нажмите Enter для завершения доступа к записи...\n";
    std::cin.ignore();
    std::cin.get();

    req = {};
    req.command = CMD_RELEASE;
    req.id      = id;
    sendRequest(hPipe, req, resp, true);
}

int main() {
    setlocale(LC_ALL, ".UTF8");
SetConsoleOutputCP(65001);
SetConsoleCP(65001);


    std::cout << "Клиент: подключение к " << PIPE_NAME << "...\n";

    while (true) {
        HANDLE hPipe = CreateFileA(
            PIPE_NAME,
            GENERIC_READ | GENERIC_WRITE,
            0,
            nullptr,
            OPEN_EXISTING,
            0,
            nullptr);

        if (hPipe != INVALID_HANDLE_VALUE) {
            std::cout << "Подключено к серверу\n";

            BOOL modeOk = SetNamedPipeHandleState(
                hPipe,
                (LPDWORD)(new DWORD(PIPE_READMODE_MESSAGE)),
                nullptr, nullptr);

            if (!modeOk) {
                std::cout << "SetNamedPipeHandleState error\n";
            }

            bool running = true;
            while (running) {
                std::cout << "\nМеню клиента:\n";
                std::cout << " 1 - чтение записи\n";
                std::cout << " 2 - модификация записи\n";
                std::cout << " 0 - выход\n";
                std::cout << "Ваш выбор: ";
                int cmd;
                if (!(std::cin >> cmd)) {
                    running = false;
                    break;
                }

                if (cmd == 0) {
                    Request req{};
                    Response resp{};
                    req.command = CMD_EXIT;
                    sendRequest(hPipe, req, resp, false);
                    running = false;
                } else if (cmd == 1) {
                    do_read(hPipe);
                } else if (cmd == 2) {
                    do_modify(hPipe);
                } else {
                    std::cout << "Неизвестная команда\n";
                }
            }

            CloseHandle(hPipe);
            break;
        }

        if (GetLastError() != ERROR_PIPE_BUSY) {
            std::cout << "Не удалось открыть pipe\n";
            return 1;
        }

        if (!WaitNamedPipeA(PIPE_NAME, 5000)) {
            std::cout << "Сервер занят, повтор...\n";
        }
    }

    std::cout << "Клиент завершён\n";
    return 0;
}
