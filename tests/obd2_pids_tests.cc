#include <gtest/gtest.h>

#include <array>
#include "../obd2_pids.hpp"

TEST(CANSPI_Obd2Pid_Tests, ShouldCalcMaxThrottlePosition)
{
    const double expected = 100.0;
    std::array<uint8_t, 8> payload { 3, 0, 0, 255, 0, 0, 0, 0 };
    const auto* pidInfo = CANSPI::GetObd2PidInfo(CANSPI::Obd2ProcID::ThrottlePosition);
    EXPECT_NE(nullptr, pidInfo);
    const auto actual = pidInfo->convert(payload);
    EXPECT_EQ(expected, actual);
}

TEST(CANSPI_Obd2Pid_Tests, ShouldCalcMaxRelThrottlePosition)
{
    const double expected = 100.0;
    std::array<uint8_t, 8> payload { 3, 0, 0, 255, 0, 0, 0, 0 };
    const auto* pidInfo = CANSPI::GetObd2PidInfo(CANSPI::Obd2ProcID::RelativeThrottlePosition);
    EXPECT_NE(nullptr, pidInfo);
    const auto actual = pidInfo->convert(payload);
    EXPECT_EQ(expected, actual);
}

TEST(CANSPI_Obd2Pid_Tests, ShouldCalcMaxRPM)
{
    const double expected = 16383.75;
    std::array<uint8_t, 8> payload { 4, 0, 0, 255, 255, 0, 0, 0 };
    const auto* pidInfo = CANSPI::GetObd2PidInfo(CANSPI::Obd2ProcID::EngineRPM);
    EXPECT_NE(nullptr, pidInfo);
    const auto actual = pidInfo->convert(payload);
    EXPECT_EQ(expected, actual);
}

TEST(CANSPI_Obd2Pid_Tests, ShouldCalcMaxSpeed)
{
    const double expected = 255;
    std::array<uint8_t, 8> payload { 3, 0, 0, 255, 0, 0, 0, 0 };
    const auto* pidInfo = CANSPI::GetObd2PidInfo(CANSPI::Obd2ProcID::VehicleSpeed);
    EXPECT_NE(nullptr, pidInfo);
    const auto actual = pidInfo->convert(payload);
    EXPECT_EQ(expected, actual);
}

TEST(CANSPI_Obd2Pid_Tests, ShouldCalcMaxEngineRuntime)
{
    const double expected = 65535;
    std::array<uint8_t, 8> payload { 4, 0, 0, 255, 255, 0, 0, 0 };
    const auto* pidInfo = CANSPI::GetObd2PidInfo(CANSPI::Obd2ProcID::EngineRuntimeSinceStart);
    EXPECT_NE(nullptr, pidInfo);
    const auto actual = pidInfo->convert(payload);
    EXPECT_EQ(expected, actual);
}

TEST(CANSPI_Obd2Pid_Tests, ShouldCalcMaxFuelLevel)
{
    const double expected = 100.0;
    std::array<uint8_t, 8> payload { 3, 0, 0, 255, 0, 0, 0, 0 };
    const auto* pidInfo = CANSPI::GetObd2PidInfo(CANSPI::Obd2ProcID::FuelLevel);
    EXPECT_NE(nullptr, pidInfo);
    const auto actual = pidInfo->convert(payload);
    EXPECT_EQ(expected, actual);
}

TEST(CANSPI_Obd2Pid_Tests, ShouldCalcMaxIntakeAirTemp)
{
    const double expected = 215;
    std::array<uint8_t, 8> payload { 3, 0, 0, 255, 0, 0, 0, 0 };
    const auto* pidInfo = CANSPI::GetObd2PidInfo(CANSPI::Obd2ProcID::IntakeAirTemperature);
    EXPECT_NE(nullptr, pidInfo);
    const auto actual = pidInfo->convert(payload);
    EXPECT_EQ(expected, actual);
}

TEST(CANSPI_Obd2Pid_Tests, ShouldCalcMaxEngineCoolantTemp)
{
    const double expected = 215;
    std::array<uint8_t, 8> payload { 3, 0, 0, 255, 0, 0, 0, 0 };
    const auto* pidInfo = CANSPI::GetObd2PidInfo(CANSPI::Obd2ProcID::EngineCoolantTemperature);
    EXPECT_NE(nullptr, pidInfo);
    const auto actual = pidInfo->convert(payload);
    EXPECT_EQ(expected, actual);
}

TEST(CANSPI_Obd2Pid_Tests, ShouldCalcMaxEngineOilTemp)
{
    const double expected = 215;
    std::array<uint8_t, 8> payload { 3, 0, 0, 255, 0, 0, 0, 0 };
    const auto* pidInfo = CANSPI::GetObd2PidInfo(CANSPI::Obd2ProcID::EngineOilTemperature);
    EXPECT_NE(nullptr, pidInfo);
    const auto actual = pidInfo->convert(payload);
    EXPECT_EQ(expected, actual);
}

TEST(CANSPI_Obd2Pid_Tests, ShouldCalcMaxOdometer)
{
    const double expected = 429496729.5;
    std::array<uint8_t, 8> payload { 6, 0, 0, 255, 255, 255, 255, 0 };
    const auto* pidInfo = CANSPI::GetObd2PidInfo(CANSPI::Obd2ProcID::Odometer);
    EXPECT_NE(nullptr, pidInfo);
    const auto actual = pidInfo->convert(payload);
    EXPECT_EQ(expected, actual);
}