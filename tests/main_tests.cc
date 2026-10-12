#include <gtest/gtest.h>

#include "../can_spi.hpp"

TEST(CANSPI_Tests, ShouldOpenAndCloseCanSocket)
{
    CANSPI::CanSpiSocket socket{"can0"};
    ASSERT_EQ(false, socket.IsOpen());
    ASSERT_EQ(true, socket.Open());
    ASSERT_EQ(true, socket.IsOpen());
    socket.Close();
    ASSERT_EQ(false, socket.IsOpen());
}

TEST(CANSPI_Tests, ShouldFailToOpenInvalidCanSocket)
{
    CANSPI::CanSpiSocket socket{"invalid0"};
    ASSERT_EQ(false, socket.IsOpen());
    ASSERT_EQ(false, socket.Open());
    ASSERT_EQ(false, socket.IsOpen());
}
