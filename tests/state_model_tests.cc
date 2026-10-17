#include <gtest/gtest.h>

#include <array>
#include <atomic>
#include <cmath>
#include <thread>

#include "../state_model.hpp"

TEST(CANSPI_StateModel_Tests, ShouldUpdateAndReadEveryPid)
{
    CANSPI::StateModelManager manager;

    manager.Update(CANSPI::Obd2ProcID::EngineCoolantTemperature, 1.0);
    manager.Update(CANSPI::Obd2ProcID::EngineOilTemperature, 2.0);
    manager.Update(CANSPI::Obd2ProcID::EngineRPM, 3.0);
    manager.Update(CANSPI::Obd2ProcID::EngineRuntimeSinceStart, 4.0);
    manager.Update(CANSPI::Obd2ProcID::FuelLevel, 5.0);
    manager.Update(CANSPI::Obd2ProcID::IntakeAirTemperature, 6.0);
    manager.Update(CANSPI::Obd2ProcID::Odometer, 7.0);
    manager.Update(CANSPI::Obd2ProcID::RelativeThrottlePosition, 8.0);
    manager.Update(CANSPI::Obd2ProcID::ThrottlePosition, 9.0);
    manager.Update(CANSPI::Obd2ProcID::VehicleSpeed, 10.0);

    const auto snapshot = manager.GetSnapshot();
    EXPECT_DOUBLE_EQ(1.0, snapshot.EngineCoolantTemperature);
    EXPECT_DOUBLE_EQ(2.0, snapshot.EngineOilTemperature);
    EXPECT_DOUBLE_EQ(3.0, snapshot.EngineRPM);
    EXPECT_DOUBLE_EQ(4.0, snapshot.EngineRuntime);
    EXPECT_DOUBLE_EQ(5.0, snapshot.FuelLevel);
    EXPECT_DOUBLE_EQ(6.0, snapshot.IntakeAirTemperature);
    EXPECT_DOUBLE_EQ(7.0, snapshot.Odometer);
    EXPECT_DOUBLE_EQ(8.0, snapshot.RelativeThrottlePosition);
    EXPECT_DOUBLE_EQ(9.0, snapshot.ThrottlePosition);
    EXPECT_DOUBLE_EQ(10.0, snapshot.VehicleSpeed);
}

TEST(CANSPI_StateModel_Tests, ShouldProvideValidSnapshotsDuringConcurrentUpdates)
{
    constexpr int iterationCount = 20000;
    constexpr std::array<CANSPI::Obd2ProcID, 10> pids = {
        CANSPI::Obd2ProcID::EngineCoolantTemperature,
        CANSPI::Obd2ProcID::EngineOilTemperature,
        CANSPI::Obd2ProcID::EngineRPM,
        CANSPI::Obd2ProcID::EngineRuntimeSinceStart,
        CANSPI::Obd2ProcID::FuelLevel,
        CANSPI::Obd2ProcID::IntakeAirTemperature,
        CANSPI::Obd2ProcID::Odometer,
        CANSPI::Obd2ProcID::RelativeThrottlePosition,
        CANSPI::Obd2ProcID::ThrottlePosition,
        CANSPI::Obd2ProcID::VehicleSpeed,
    };

    CANSPI::StateModelManager manager;
    std::atomic<bool> readerReady{false};
    std::atomic<bool> writerFinished{false};
    std::atomic<bool> invalidSnapshot{false};

    std::thread reader([&]()
    {
        readerReady.store(true, std::memory_order_release);
        do
        {
            const auto snapshot = manager.GetSnapshot();
            //std::cout << "EngineCoolantTemperature: " << snapshot.EngineCoolantTemperature << std::endl;
            const auto isValidValue = [](double value)
            {
                return std::isfinite(value) && value >= 0.0 &&
                    value <= iterationCount && std::floor(value) == value;
            };

            if (!isValidValue(snapshot.EngineCoolantTemperature) ||
                !isValidValue(snapshot.EngineOilTemperature) ||
                !isValidValue(snapshot.EngineRPM) ||
                !isValidValue(snapshot.EngineRuntime) ||
                !isValidValue(snapshot.FuelLevel) ||
                !isValidValue(snapshot.IntakeAirTemperature) ||
                !isValidValue(snapshot.Odometer) ||
                !isValidValue(snapshot.RelativeThrottlePosition) ||
                !isValidValue(snapshot.ThrottlePosition) ||
                !isValidValue(snapshot.VehicleSpeed)
            )
            {
                invalidSnapshot.store(true, std::memory_order_relaxed);
                break;
            }
        } while (!writerFinished.load(std::memory_order_acquire));
    });

    while (!readerReady.load(std::memory_order_acquire))
    {
        std::this_thread::yield();
    }

    for (int iteration = 1; iteration <= iterationCount; ++iteration)
    {
        for (const auto pid : pids)
        {
            manager.Update(pid, static_cast<double>(iteration));
        }
    }

    writerFinished.store(true, std::memory_order_release);
    reader.join();

    EXPECT_FALSE(invalidSnapshot.load(std::memory_order_relaxed));

    // Perform one last write before final read to ensure the publish model has the final write result.
    for (const auto pid : pids)
    {
        manager.Update(pid, iterationCount);
    }

    const auto snapshot = manager.GetSnapshot();
    //std::cout << "EngineCoolantTemperature: " << snapshot.EngineCoolantTemperature << std::endl;
    EXPECT_DOUBLE_EQ(iterationCount, snapshot.EngineCoolantTemperature);
    EXPECT_DOUBLE_EQ(iterationCount, snapshot.EngineOilTemperature);
    EXPECT_DOUBLE_EQ(iterationCount, snapshot.EngineRPM);
    EXPECT_DOUBLE_EQ(iterationCount, snapshot.EngineRuntime);
    EXPECT_DOUBLE_EQ(iterationCount, snapshot.FuelLevel);
    EXPECT_DOUBLE_EQ(iterationCount, snapshot.IntakeAirTemperature);
    EXPECT_DOUBLE_EQ(iterationCount, snapshot.Odometer);
    EXPECT_DOUBLE_EQ(iterationCount, snapshot.RelativeThrottlePosition);
    EXPECT_DOUBLE_EQ(iterationCount, snapshot.ThrottlePosition);
    EXPECT_DOUBLE_EQ(iterationCount, snapshot.VehicleSpeed);
}

