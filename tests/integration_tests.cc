#include <gtest/gtest.h>

#include <array>
#include <chrono>
#include <cmath>
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

TEST_F(CANSPI_IntegrationTest, DISABLED_ShouldSendThrottlePositionRequest)
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

TEST_F(CANSPI_IntegrationTest, ShouldSendAndReceiveViaLoopback)
{
    constexpr int testDuration = 0.5 /*seconds*/ * 1000000;
    constexpr int iterationCount = 20000;

    CANSPI::CanSpiSocket socket{CANSPI_IntegrationTest::kCanInterface};
    ASSERT_TRUE(socket.Open());
    ASSERT_TRUE(socket.BeginReceiveAsync(std::chrono::milliseconds(100)));

    // std::cout << "--- Socket open and receiving ---" << std::endl;

    std::atomic<bool> writerReady{false};
    std::atomic<bool> readerFinished{false};
    std::thread writer([&]()
    {
        writerReady.store(true, std::memory_order_release);
        CANSPI::CanSpiFrame frame;

        // std::cout << "--- WRITER thread executing ---" << std::endl;

        for (int iteration = 1; true; ++iteration)
        {
            usleep(100);
            const uint8_t value = static_cast<uint8_t>(iteration % 256);

            // std::cout << "Value: " << std::hex << +value << std::endl;

            frame = {
                .id = 0x7E8U,
                .dataLength = 8U,
                .data = { 0x03U, 0x41U, static_cast<uint8_t>(CANSPI::Obd2ProcID::EngineCoolantTemperature), value }
            };
            ASSERT_TRUE(socket.Send(frame));
            if (readerFinished.load(std::memory_order_acquire))
            {
                break;
            }
        }
        // std::cout << "--- WRITER thread finished writing ---" << std::endl;
    });

    while (!writerReady.load(std::memory_order_acquire))
    {
        std::this_thread::yield();
    }

    double max = -41.0;
    double min = 216.0;
    std::chrono::steady_clock::time_point startTime(std::chrono::steady_clock::now());
    std::chrono::steady_clock::time_point endTime;
    do
    {
        const auto snapshot = socket.GetSnapshot();
        //std::cout << "EngineCoolantTemperature: " << snapshot.EngineCoolantTemperature << std::endl;
        EXPECT_GE(snapshot.EngineCoolantTemperature, -40.0);
        EXPECT_LE(snapshot.EngineCoolantTemperature, 215.0);
        max = std::max(max, snapshot.EngineCoolantTemperature);
        min = std::min(min, snapshot.EngineCoolantTemperature);
        usleep(100);
        endTime = std::chrono::steady_clock::now();
    } while (std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() < testDuration);

    // std::cout << "--- READER thread finished reading ---" << std::endl;
    readerFinished.store(true, std::memory_order_release);
    writer.join();
    socket.Close();

    EXPECT_EQ(215.0, max);
    EXPECT_EQ(-40, min);
}