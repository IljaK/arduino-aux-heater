#include <gtest/gtest.h>
#include <Arduino.h>
#include <StringStackArray.h>
#include <SocketMessageBuffer.h>

TEST(SocketMessageBufferTest, SocketMessageBufferAppendSingle)
{
    uint8_t data1[] = {8, 0, 0, 1,2,3,4,5,6,7,8};

    SocketMessageBuffer msgBuffer(3, 128);

    msgBuffer.Append((uint8_t *)data1, 11);
    EXPECT_EQ(msgBuffer.Size(), 1);
    EXPECT_EQ(msgBuffer.HasMessage(), true);

    SocketMessage *result = msgBuffer.UnshiftFirst();
    EXPECT_EQ(msgBuffer.Size(), 0);
    EXPECT_EQ(msgBuffer.HasMessage(), false);

    msgBuffer.FreeItem(result);

    EXPECT_EQ(msgBuffer.Size(), 0);
    EXPECT_EQ(msgBuffer.HasMessage(), false);
}

TEST(SocketMessageBufferTest, SocketMessageBufferAppendPartial)
{
    uint8_t data1[] = {8};
    uint8_t data2[] = {0};
    uint8_t data3[] = {0};
    uint8_t data4[] = {1,2,3}; 
    uint8_t data5[] = {4,5,6,7,8};

    SocketMessageBuffer msgBuffer(3, 128);

    msgBuffer.Append((uint8_t *)data1, sizeof(data1));
    EXPECT_EQ(msgBuffer.Size(), 1);
    EXPECT_EQ(msgBuffer.HasMessage(), false);

    msgBuffer.Append((uint8_t *)data2, sizeof(data2));
    EXPECT_EQ(msgBuffer.Size(), 1);
    EXPECT_EQ(msgBuffer.HasMessage(), false);

    msgBuffer.Append((uint8_t *)data3, sizeof(data3));
    EXPECT_EQ(msgBuffer.Size(), 1);
    EXPECT_EQ(msgBuffer.HasMessage(), false);

    msgBuffer.Append((uint8_t *)data4, sizeof(data4));
    EXPECT_EQ(msgBuffer.Size(), 1);
    EXPECT_EQ(msgBuffer.HasMessage(), false);

    msgBuffer.Append((uint8_t *)data5, sizeof(data5));
    EXPECT_EQ(msgBuffer.Size(), 1);
    EXPECT_EQ(msgBuffer.HasMessage(), true);

    SocketMessage *result = msgBuffer.UnshiftFirst();
    EXPECT_EQ(msgBuffer.Size(), 0);
    EXPECT_EQ(msgBuffer.HasMessage(), false);

    msgBuffer.FreeItem(result);

    EXPECT_EQ(msgBuffer.Size(), 0);
    EXPECT_EQ(msgBuffer.HasMessage(), false);
}

TEST(SocketMessageBufferTest, SocketMessageBufferAppendPartialHead)
{
    uint8_t data1[] = {8,0};
    uint8_t data2[] = {0};
    uint8_t data3[] = {1,2,3,4,5,6,7,8};

    SocketMessageBuffer msgBuffer(3, 128);

    msgBuffer.Append((uint8_t *)data1, 2);
    EXPECT_EQ(msgBuffer.Size(), 1);
    EXPECT_EQ(msgBuffer.HasMessage(), false);


    msgBuffer.Append((uint8_t *)data2, 1);
    EXPECT_EQ(msgBuffer.Size(), 1);
    EXPECT_EQ(msgBuffer.HasMessage(), false);

    msgBuffer.Append((uint8_t *)data3, 8);
    EXPECT_EQ(msgBuffer.Size(), 1);
    EXPECT_EQ(msgBuffer.HasMessage(), true);

    SocketMessage *result = msgBuffer.UnshiftFirst();
    EXPECT_EQ(msgBuffer.Size(), 0);
    EXPECT_EQ(msgBuffer.HasMessage(), false);

    msgBuffer.FreeItem(result);

    EXPECT_EQ(msgBuffer.Size(), 0);
    EXPECT_EQ(msgBuffer.HasMessage(), false);
}