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

/**
 * @brief Verifies replies from separate ECUs are kept in same snapshot
 */
TEST(CANSPI_Tests, ShouldKeepEcuResponsesUnified)
{
    CANSPI::CanSpiSocket socket;
    const CANSPI::CanSpiFrame frame1{
        .id = 0x7E8U,
        .dataLength = 8U,
        .data = { 0x03U, 0x41U, static_cast<uint8_t>(CANSPI::Obd2ProcID::ThrottlePosition), 0xCCU }
    };
    const CANSPI::CanSpiFrame frame2{
        .id = 0x7E9U,
        .dataLength = 8U,
        .data = { 0x03U, 0x41U, static_cast<uint8_t>(CANSPI::Obd2ProcID::RelativeThrottlePosition), 0xFFU }
    };

    ASSERT_TRUE(socket.ProcessFrame(frame1));
    ASSERT_TRUE(socket.ProcessFrame(frame2));

    const double expectedFrame1 = 80.0;
    const double expectedFrame2 = 100.0;
    const auto snapshot = socket.GetSnapshot();
    EXPECT_EQ(expectedFrame1, snapshot.ThrottlePosition);
    EXPECT_EQ(expectedFrame2, snapshot.RelativeThrottlePosition);
}
