#include "gtest/gtest.h"
#include "marker_thread.h"
#include <fstream>
#include <cstdio>

class SharedFileTest : public ::testing::Test {
protected:
    void SetUp() override {
        std::remove("test_file.bin");
    }
    
    void TearDown() override {
        SharedFile::close();
        std::remove("test_file.bin");
    }
};

TEST_F(SharedFileTest, CreateFile) {
    EXPECT_TRUE(SharedFile::create("test_file.bin", 5));
}

TEST_F(SharedFileTest, WriteAndReadMessage) {
    ASSERT_TRUE(SharedFile::create("test_file.bin", 5));
    
    std::string testMessage = "Hello World";
    EXPECT_TRUE(SharedFile::writeMessage(testMessage));
    
    std::string receivedMessage;
    EXPECT_TRUE(SharedFile::readMessage(receivedMessage));
    EXPECT_EQ(testMessage, receivedMessage);
}

TEST_F(SharedFileTest, ReadFromEmpty) {
    ASSERT_TRUE(SharedFile::create("test_file.bin", 5));
    
    std::string message;
    EXPECT_FALSE(SharedFile::readMessage(message));
}

TEST_F(SharedFileTest, MultipleMessages) {
    ASSERT_TRUE(SharedFile::create("test_file.bin", 3));
    
    // Записываем 3 сообщения
    EXPECT_TRUE(SharedFile::writeMessage("Message 1"));
    EXPECT_TRUE(SharedFile::writeMessage("Message 2"));
    EXPECT_TRUE(SharedFile::writeMessage("Message 3"));
    
    // Попытка записать 4-е сообщение должна вернуть false (очередь полна)
    EXPECT_FALSE(SharedFile::writeMessage("Message 4"));
    
    // Читаем сообщения
    std::string msg;
    EXPECT_TRUE(SharedFile::readMessage(msg));
    EXPECT_EQ("Message 1", msg);
    
    EXPECT_TRUE(SharedFile::readMessage(msg));
    EXPECT_EQ("Message 2", msg);
    
    EXPECT_TRUE(SharedFile::readMessage(msg));
    EXPECT_EQ("Message 3", msg);
    
    // После чтения всех сообщений очередь должна быть пуста
    EXPECT_FALSE(SharedFile::readMessage(msg));
}