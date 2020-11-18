#include <gtest/gtest.h>
#include <ByteArrayStream.h>


TEST(ByteArrayStreamTest, ByteArrayStreamTestReadWrite)
{
    uint8_t array[3] = {0,1,2};

    uint8_t size = 8;
    ByteArrayStream * stream = new ByteArrayStream(size);

	EXPECT_EQ(stream->Available(), 0);
    
    stream->Write(array, 3);

	EXPECT_EQ(stream->Available(), 3);

    stream->Write(array, 3);

	EXPECT_EQ(stream->Available(), 6);

    stream->Write(array, 3);

	EXPECT_EQ(stream->Available(), 8);

    stream->Read(array, 3);
    for (uint8_t i = 0; i < 3; i++) {
        EXPECT_EQ(array[i], i);
    }
	EXPECT_EQ(stream->Available(), 5);

    stream->Read(array, 3);
    for (uint8_t i = 0; i < 3; i++) {
        EXPECT_EQ(array[i], i);
    }
	EXPECT_EQ(stream->Available(), 2);


    stream->Read(array, 3);
    for (uint8_t i = 0; i < 2; i++) {
        EXPECT_EQ(array[i], i);
    }
	EXPECT_EQ(stream->Available(), 0);
}