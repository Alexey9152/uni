// tests_lock_manager.cpp
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "lock_manager.h"

// doctest — заголовочная библиотека, достаточно подключить один header
// и определить конфигурационный макрос для генерации main.

TEST_CASE("Несколько читателей на одну запись") {
    LockManager lm;
    CHECK(lm.acquireRead(1) == true);
    CHECK(lm.acquireRead(1) == true);
    lm.releaseRead(1);
    lm.releaseRead(1);
    // после двух release запись должна быть полностью разблокирована
    CHECK(lm.acquireWrite(1) == true);
}

TEST_CASE("Писатель блокирует читателей") {
    LockManager lm;
    CHECK(lm.acquireWrite(42) == true);
    CHECK(lm.acquireRead(42) == false);
    lm.releaseWrite(42);
    CHECK(lm.acquireRead(42) == true);
}

TEST_CASE("Читатели блокируют писателя") {
    LockManager lm;
    CHECK(lm.acquireRead(7) == true);
    CHECK(lm.acquireRead(7) == true);
    CHECK(lm.acquireWrite(7) == false);
    lm.releaseRead(7);
    lm.releaseRead(7);
    CHECK(lm.acquireWrite(7) == true);
}
