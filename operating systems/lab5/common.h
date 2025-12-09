// common.h
#pragma once
#include <cstdint>

struct employee {
    int  num;        // ID сотрудника (ключ)
    char name[10];   // имя (без пробелов или с '\0'-терминатором)
    double hours;    // отработанные часы
};

enum class Operation : uint32_t {
    READ_REQUEST      = 1, // запросить чтение (с блокировкой до RELEASE)
    WRITE_BEGIN       = 2, // запросить модификацию (с блокировкой до RELEASE)
    WRITE_COMMIT      = 3, // отправить изменённую запись
    RELEASE           = 4, // освободить блокировку на записи
    SHUTDOWN_SERVER   = 5  // остановка сервера (опционально)
};

struct Request {
    Operation op;
    int       id;       // ключ (num сотрудника)
    employee  data;     // используется для WRITE_COMMIT, в остальных неважен
};

struct Response {
    bool      ok;               // успешно ли выполнена операция
    char      message[64];      // короткое текстовое сообщение
    employee  data;             // запись, возвращаемая сервером
};
