#define BOOST_TEST_MODULE SharedFileBoostTests
#include <boost/test/unit_test.hpp>
#include "marker_thread.h"
#include <cstdio>
#include <string>

struct SharedFileFixture {
    SharedFileFixture() {
        std::remove("test_boost.bin");
    }
    
    ~SharedFileFixture() {
        SharedFile::close();
        std::remove("test_boost.bin");
    }
};

BOOST_FIXTURE_TEST_SUITE(SharedFileBoostTest, SharedFileFixture)

BOOST_AUTO_TEST_CASE(CreateFile) {
    BOOST_CHECK(SharedFile::create("test_boost.bin", 5));
}

BOOST_AUTO_TEST_CASE(WriteReadSingleMessage) {
    BOOST_REQUIRE(SharedFile::create("test_boost.bin", 5));
    
    std::string testMsg = "Hello Boost!";
    BOOST_CHECK(SharedFile::writeMessage(testMsg));
    
    std::string received;
    BOOST_CHECK(SharedFile::readMessage(received));
    BOOST_CHECK_EQUAL(received, testMsg);
}

BOOST_AUTO_TEST_CASE(MultipleMessagesFIFO) {
    BOOST_REQUIRE(SharedFile::create("test_boost.bin", 3));
    
    // Записываем 3 сообщения
    BOOST_CHECK(SharedFile::writeMessage("Msg1"));
    BOOST_CHECK(SharedFile::writeMessage("Msg2"));
    BOOST_CHECK(SharedFile::writeMessage("Msg3"));
    
    // Читаем в том же порядке (FIFO)
    std::string msg;
    BOOST_CHECK(SharedFile::readMessage(msg));
    BOOST_CHECK_EQUAL(msg, "Msg1");
    
    BOOST_CHECK(SharedFile::readMessage(msg));
    BOOST_CHECK_EQUAL(msg, "Msg2");
    
    BOOST_CHECK(SharedFile::readMessage(msg));
    BOOST_CHECK_EQUAL(msg, "Msg3");
}

BOOST_AUTO_TEST_CASE(SenderReadyCount) {
    BOOST_REQUIRE(SharedFile::create("test_boost.bin", 5));
    
    SharedFile::markSenderReady();
    BOOST_CHECK_EQUAL(SharedFile::getSenderReadyCount(), 1);
    
    SharedFile::markSenderReady();
    BOOST_CHECK_EQUAL(SharedFile::getSenderReadyCount(), 2);
}

BOOST_AUTO_TEST_SUITE_END()
