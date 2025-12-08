#define BOOST_TEST_MODULE SharedFileTests
#include <boost/test/unit_test.hpp>

#include "marker_thread.h"
#include <cstdio>
#include <string>

struct SharedFileFixture {
    SharedFileFixture() {
        std::remove("test_file.bin");
    }
    ~SharedFileFixture() {
        SharedFile::close();
        std::remove("test_file.bin");
    }
};

BOOST_FIXTURE_TEST_SUITE(SharedFileTest, SharedFileFixture)

// Просто проверяем, что файл и общая память создаются
BOOST_AUTO_TEST_CASE(CreateFile) {
    BOOST_CHECK(SharedFile::create("test_file.bin", 5));
}

// Один записал — один прочитал, без пустой очереди
BOOST_AUTO_TEST_CASE(WriteAndReadOneMessage) {
    BOOST_REQUIRE(SharedFile::create("test_file.bin", 5));

    std::string testMessage = "Hello World";
    BOOST_CHECK(SharedFile::writeMessage(testMessage));

    std::string receivedMessage;
    BOOST_CHECK(SharedFile::readMessage(receivedMessage));
    BOOST_CHECK_EQUAL(testMessage, receivedMessage);
}

// Несколько сообщений, но не больше размера очереди
BOOST_AUTO_TEST_CASE(WriteAndReadSeveralMessages) {
    BOOST_REQUIRE(SharedFile::create("test_file.bin", 3));

    BOOST_CHECK(SharedFile::writeMessage("Message 1"));
    BOOST_CHECK(SharedFile::writeMessage("Message 2"));
    BOOST_CHECK(SharedFile::writeMessage("Message 3"));

    std::string msg;

    BOOST_CHECK(SharedFile::readMessage(msg));
    BOOST_CHECK_EQUAL(msg, std::string("Message 1"));

    BOOST_CHECK(SharedFile::readMessage(msg));
    BOOST_CHECK_EQUAL(msg, std::string("Message 2"));

    BOOST_CHECK(SharedFile::readMessage(msg));
    BOOST_CHECK_EQUAL(msg, std::string("Message 3"));
}

// Проверка, что несколько create/close подряд не ломают ресурсы
BOOST_AUTO_TEST_CASE(MultipleCreateClose) {
    for (int i = 0; i < 3; ++i) {
        BOOST_CHECK(SharedFile::create("test_file.bin", 5));
        std::string data = "Test";
        BOOST_CHECK(SharedFile::writeMessage(data));
        std::string out;
        BOOST_CHECK(SharedFile::readMessage(out));
        BOOST_CHECK_EQUAL(out, data);
        SharedFile::close();
        std::remove("test_file.bin");
    }
}

BOOST_AUTO_TEST_SUITE_END()
