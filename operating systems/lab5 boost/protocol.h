// protocol.h
#pragma once
#include <windows.h>

struct employee {
    int  num;        // ID сотрудника (ключ)
    char name[10];   // имя (без UTF-8, чтобы не париться с размером)
    double hours;    // отработанные часы
};

enum Command : int {
    CMD_READ = 1,        // клиент просит чтение записи
    CMD_WRITE_LOCK = 2,  // клиент начинает модификацию (захватить write-lock)
    CMD_WRITE_COMMIT = 3,// клиент отправляет изменённую запись
    CMD_RELEASE = 4,     // клиент завершает доступ к записи
    CMD_EXIT = 5         // клиент завершает сеанс
};

enum Status : int {
    ST_OK = 0,
    ST_ERROR = 1,
    ST_NOT_FOUND = 2
};

#pragma pack(push, 1)
struct Request {
    int command;
    int id;       // employee.num
    employee emp; // используется для WRITE_COMMIT
};

struct Response {
    int status;
    employee emp; // для READ/WRITE_LOCK сервер возвращает запись
};
#pragma pack(pop)

static const char* PIPE_NAME = R"(\\.\pipe\employee_pipe)";
