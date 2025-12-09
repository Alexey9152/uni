#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include <iostream>
#include <string>
#include <limits>

#include "common.h"

static const wchar_t* PIPE_NAME = L"\\\\.\\pipe\\employee_pipe";

bool sendRequest(HANDLE pipe, const Request &req) {
    DWORD written = 0;
    BOOL ok = WriteFile(pipe, &req, sizeof(req), &written, nullptr);
    return ok && written == sizeof(req);
}

bool recvResponse(HANDLE pipe, Response &resp) {
    DWORD read = 0;
    BOOL ok = ReadFile(pipe, &resp, sizeof(resp), &read, nullptr);
    return ok && read == sizeof(resp);
}

void printEmployee(const employee &e) {
    std::cout << "ID: " << e.num
              << " Name: " << e.name
              << " Hours: " << e.hours << "\n";
}

int main() {
    // Переводим консоль в UTF-8 (кодовая страница 65001)
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);  // ввод тоже в UTF‑8 [web:274][web:286]

    std::cout << "Клиент подключается к каналу \\\\.\\pipe\\employee_pipe\n";

    HANDLE pipe = CreateFileW(
        PIPE_NAME,
        GENERIC_READ | GENERIC_WRITE,
        0,
        nullptr,
        OPEN_EXISTING,
        0,
        nullptr
    );

    if (pipe == INVALID_HANDLE_VALUE) {
        std::cerr << "Не удалось открыть именованный канал. Код ошибки: "
                  << GetLastError() << "\n";
        return 1;
    }

    while (true) {
        std::cout << "\nВыберите операцию:\n";
        std::cout << "1 - чтение записи\n";
        std::cout << "2 - модификация записи\n";
        std::cout << "0 - выход\n";
        int choice;
        std::cin >> choice;

        if (!std::cin) {
            std::cin.clear();
            std::cin.ignore((std::numeric_limits<std::streamsize>::max)(), '\n');
            continue;
        }

        if (choice == 0) {
            break;
        } else if (choice == 1) {
            int id;
            std::cout << "Введите ID сотрудника для чтения: ";
            std::cin >> id;

            Request req{};
            req.op = Operation::READ_REQUEST;
            req.id = id;

            if (!sendRequest(pipe, req)) {
                std::cerr << "Ошибка отправки запроса.\n";
                break;
            }

            Response resp{};
            if (!recvResponse(pipe, resp)) {
                std::cerr << "Ошибка получения ответа.\n";
                break;
            }

            if (!resp.ok) {
                std::cout << "Сервер: " << resp.message << "\n";
            } else {
                std::cout << "Получена запись:\n";
                printEmployee(resp.data);

                std::cout << "Нажмите любую клавишу, чтобы завершить доступ к записи...\n";
                std::string tmp;
                std::cin >> tmp;

                Request rel{};
                rel.op = Operation::RELEASE;
                rel.id = id;
                sendRequest(pipe, rel);
                recvResponse(pipe, resp); // читаем ответ для очистки канала
            }
        } else if (choice == 2) {
            int id;
            std::cout << "Введите ID сотрудника для модификации: ";
            std::cin >> id;

            Request req{};
            req.op = Operation::WRITE_BEGIN;
            req.id = id;

            if (!sendRequest(pipe, req)) {
                std::cerr << "Ошибка отправки запроса.\n";
                break;
            }

            Response resp{};
            if (!recvResponse(pipe, resp)) {
                std::cerr << "Ошибка получения ответа.\n";
                break;
            }

            if (!resp.ok) {
                if (std::string(resp.message) == "ID_NOT_FOUND") {
        std::cout << "Сервер: запись с таким ID не найдена\n";
    } else {
        std::cout << "Сервер (ошибка): " << resp.message << "\n";
    }
                continue;
            }

            std::cout << "Текущая запись:\n";
            printEmployee(resp.data);

            employee e = resp.data;
            std::cout << "Введите новые значения (name, hours>=0): ";
            std::cin >> e.name;

            // проверка на неотрицательные часы
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

            std::cout << "Нажмите любую клавишу для отправки изменённой записи...\n";
            std::string tmp;
            std::cin >> tmp;

            Request commit{};
            commit.op   = Operation::WRITE_COMMIT;
            commit.id   = id;
            commit.data = e;

            if (!sendRequest(pipe, commit)) {
                std::cerr << "Ошибка отправки изменённой записи.\n";
                break;
            }

            if (!recvResponse(pipe, resp)) {
                std::cerr << "Ошибка получения ответа.\n";
                break;
            }

            std::cout << "Сервер: " << resp.message << "\n";

            std::cout << "Нажмите любую клавишу, чтобы завершить доступ к записи...\n";
            std::cin >> tmp;

            Request rel{};
            rel.op = Operation::RELEASE;
            rel.id = id;
            sendRequest(pipe, rel);
            recvResponse(pipe, resp);
        } else {
            std::cout << "Неизвестная команда.\n";
        }
    }

    CloseHandle(pipe);
    std::cout << "Клиент завершил работу.\n";
    return 0;
}
