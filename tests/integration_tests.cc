#include <gtest/gtest.h>

#include <array>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <unistd.h>

#define FMT_HEADER_ONLY
#include <fmt/compile.h>
#include <fmt/format.h>

#include "../can_spi.hpp"

class CANSPI_IntegrationTest : public testing::Test
{
protected:
    // Per-test-suite set-up.
    // Called before the first test in this test suite.
    static void SetUpTestSuite()
    {
        std::cout << "EXECUTING SetUpTestSuite" << std::endl;
        initialCanEnabled_ = IsInterfaceUp(kCanInterface);
        initialCanLoopbackEnabled_ = IsLoopbackEnabled(kCanInterface);

        // if interface is not enabled OR loopback is not enabled
        if (!initialCanEnabled_ || !initialCanLoopbackEnabled_)
        {
            std::cout << "Bringing down can0 interface" << std::endl;
            ExecCmd(fmt::format(kCanDownCmd, kCanInterface));

            std::cout << "Bringing up can0 interface with loopback on" << std::endl;
            ExecCmd(fmt::format(kCanUpCmd, kCanInterface, "on"));
        }
    }

    // Per-test-suite tear-down.
    // Called after the last test in this test suite.
    static void TearDownTestSuite()
    {
        // restore previous can0 state
        if (!initialCanEnabled_ || !initialCanLoopbackEnabled_)
        {
            std::cout << "Restoring can0 state." << std::endl << "Bringing down can0 interface" << std::endl;
            ExecCmd(fmt::format(kCanDownCmd, kCanInterface));

            if (initialCanEnabled_ && !initialCanLoopbackEnabled_)
            {
                std::cout << "Bringing up can0 interface with loopback off" << std::endl;
                ExecCmd(fmt::format(kCanUpCmd, kCanInterface, "off"));
            }
        }
    }

    static bool IsInterfaceUp(const char *interfaceName = "can0")
    {
        const auto state = ExecCmdWithResult(fmt::format(kCanStateCmd, interfaceName));
        return state == "UP\n";
    }

    static bool IsLoopbackEnabled(const char *interfaceName = "can0")
    {
        const auto state = ExecCmdWithResult(fmt::format(kCanLoopbackStateCmd, interfaceName));
        return state == "LOOPBACK\n";
    }

    struct pclose_deleter
    {
        void operator()(FILE* f) const
        {
            if (f) pclose(f);
        }
    };

    static void ExecCmd(const std::string cmd)
    {
        int ret = std::system(cmd.c_str());
        if (ret != 0)
        {
            throw std::runtime_error("Unable to execute command: " + cmd + "\n" + std::strerror(errno));
        }
    }

    static std::string ExecCmdWithResult(const std::string cmd)
    {
        // Open in read-only mode ("r")
        // unique_ptr ensures pipe closes automatically via pclose
        //std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
        std::unique_ptr<FILE, pclose_deleter> pipe(popen(cmd.c_str(), "r"));

        if (!pipe)
        {
            throw std::runtime_error("Unable to open pipe for command: " + cmd);
        }

        std::array<char, 128> buffer;
        std::string result;
        while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
        {
            result += buffer.data();
        }

        return result;
    }

private:
    inline static constexpr auto kCanUpCmd = FMT_COMPILE("sudo ip link set {} up type can bitrate 500000 sample-point 0.750 restart-ms 100 loopback {}");
    inline static constexpr auto kCanDownCmd = FMT_COMPILE("sudo ip link set {} down");

    inline static constexpr auto kCanStateAndLoopbackCmd = FMT_COMPILE("ip --details link show {} | perl -0777 -ne 'print \"$1$3|$2\" if /state\\s+(up|down).*?(loopback)|state\\s+(up|down)/isg'");
    inline static constexpr auto kCanStateCmd = FMT_COMPILE("ip --details link show {} | grep -Poi \"state \\Kup|down\"");
    inline static constexpr auto kCanLoopbackStateCmd = FMT_COMPILE("ip --details link show {} | grep -oi \"LOOPBACK\"");

public:
    inline static constexpr auto kCanInterface = "can0";
    static bool initialCanEnabled_;
    static bool initialCanLoopbackEnabled_;
};

bool CANSPI_IntegrationTest::initialCanEnabled_ = false;
bool CANSPI_IntegrationTest::initialCanLoopbackEnabled_ = false;

TEST_F(CANSPI_IntegrationTest, DISABLED_ShouldReceiveThrottlePositionCanFrame)
{
    //
    // Use with cansend can0 7E8#030111FF00000000
    //
    CANSPI::CanSpiSocket socket{CANSPI_IntegrationTest::kCanInterface};
    ASSERT_TRUE(socket.Open());

    CANSPI::CanSpiFrame frame{};
    ASSERT_TRUE(socket.Receive(frame, std::chrono::milliseconds{10000}));
    socket.Close();

    EXPECT_EQ(frame.id, 0x7E8U);
    EXPECT_EQ(frame.dataLength, 8U);
    EXPECT_EQ(frame.data[0], 0x03U);
    EXPECT_EQ(frame.data[1], 0x01U);

    const auto pid = static_cast<CANSPI::Obd2ProcID>(frame.data[2]);
    EXPECT_EQ(pid, CANSPI::Obd2ProcID::ThrottlePosition);

    const auto* pidInfo = CANSPI::GetObd2PidInfo(pid);
    EXPECT_NE(nullptr, pidInfo);
    const auto actual = pidInfo->convert(frame.data);
    EXPECT_EQ(100.0, actual);
}

TEST_F(CANSPI_IntegrationTest, DISABLED_ShouldReceiveOdometerCanFrame)
{
    //
    // Use with cansend can0 7E8#0601A6FFFFFFFF00
    //
    CANSPI::CanSpiSocket socket{CANSPI_IntegrationTest::kCanInterface};
    ASSERT_TRUE(socket.Open());

    CANSPI::CanSpiFrame frame{};
    ASSERT_TRUE(socket.Receive(frame, std::chrono::milliseconds{10000}));
    socket.Close();

    EXPECT_EQ(frame.id, 0x7E8U);
    EXPECT_EQ(frame.dataLength, 8U);
    EXPECT_EQ(frame.data[0], 0x06U);
    EXPECT_EQ(frame.data[1], 0x01U);

    const auto pid = static_cast<CANSPI::Obd2ProcID>(frame.data[2]);
    EXPECT_EQ(pid, CANSPI::Obd2ProcID::Odometer);

    const auto* pidInfo = CANSPI::GetObd2PidInfo(pid);
    EXPECT_NE(nullptr, pidInfo);
    const auto actual = pidInfo->convert(frame.data);

    const double expected = 429496729.5;
    EXPECT_EQ(expected, actual);
}

TEST_F(CANSPI_IntegrationTest, ShouldSendThrottlePositionRequest)
{
    CANSPI::CanSpiSocket socket{CANSPI_IntegrationTest::kCanInterface};
    CANSPI::CanSpiFrame frame{
        .id = 0x7DFU,
        .dataLength = 8U,
        .data = std::array<uint8_t, 8> { 2U, 1U, static_cast<uint8_t>(CANSPI::Obd2ProcID::ThrottlePosition) }
    };
    ASSERT_TRUE(socket.Open());
    EXPECT_TRUE(socket.Send(frame));
    socket.Close();
}