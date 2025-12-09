// tests/LockManagerTests.cpp
#define BOOST_TEST_MODULE LockManagerTests
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include <boost/thread.hpp>
#include <vector>
#include <atomic>

// Вынеси эти типы из Server.cpp в общий заголовок LockManager.h,
// чтобы можно было подключить и в сервер, и в тесты.
#include "LockManager.h"

BOOST_AUTO_TEST_CASE(single_reader_lock_unlock)
{
    LockManager lm;

    BOOST_CHECK(lm.acquireRead(1));
    lm.releaseRead(1);

    BOOST_CHECK(lm.acquireRead(1)); // после release снова можно
    lm.releaseRead(1);
}

BOOST_AUTO_TEST_CASE(writer_excludes_readers)
{
    LockManager lm;

    // Сначала захватываем writer
    BOOST_CHECK(lm.acquireWrite(2));

    // Читатель не должен зайти
    BOOST_CHECK(!lm.acquireRead(2));

    lm.releaseWrite(2);

    // После release writer читатель должен снова зайти
    BOOST_CHECK(lm.acquireRead(2));
    lm.releaseRead(2);
}

BOOST_AUTO_TEST_CASE(readers_share_writer_exclusive)
{
    LockManager lm;

    // Два читателя на один id должны проходить
    BOOST_CHECK(lm.acquireRead(3));
    BOOST_CHECK(lm.acquireRead(3));

    // Пока есть читатели, writer не должен получить лок
    BOOST_CHECK(!lm.acquireWrite(3));

    lm.releaseRead(3);
    lm.releaseRead(3);

    // Теперь writer может получить лок
    BOOST_CHECK(lm.acquireWrite(3));
    lm.releaseWrite(3);
}

// Пример простого контент-теста с потоками
BOOST_AUTO_TEST_CASE(parallel_readers)
{
    LockManager lm;
    const int id = 4;

    std::atomic<int> successCount{0};

    auto reader = [&]() {
        if (lm.acquireRead(id)) {
            // имитируем работу
            boost::this_thread::sleep_for(boost::chrono::milliseconds(10));
            lm.releaseRead(id);
            ++successCount;
        }
    };

    std::vector<boost::thread> threads;
    for (int i = 0; i < 5; ++i) {
        threads.emplace_back(reader);
    }
    for (auto& t : threads) t.join();

    // Все 5 должны были зайти в чтение
    BOOST_CHECK_EQUAL(successCount.load(), 5);
}
