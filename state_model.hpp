#pragma once

#include <array>
#include <algorithm>
#include <mutex>

#include "obd2_pids.hpp"

namespace CANSPI
{
    using SteadyTimePoint = std::chrono::time_point<
        std::chrono::steady_clock,
        std::chrono::duration<uintptr_t, std::milli>
    >;

    struct StalePid
    {
        Obd2ProcID pid;
        SteadyTimePoint updated;
    };

    struct StateModel
    {
        /// Engine coolant temperature in degrees Celsius
        double EngineCoolantTemperature = 0.0;
        /// Engine coolant temperature update time
        SteadyTimePoint EngineCoolantTemperatureUpdated = SteadyTimePoint::min();

        /// Engine oil temperature in degrees Celsius
        double EngineOilTemperature = 0.0;
        /// Engine oil temperature update time
        SteadyTimePoint EngineOilTemperatureUpdated = SteadyTimePoint::min();

        /// Engine RPM
        double EngineRPM = 0.0;
        /// Engine RPM update time
        SteadyTimePoint EngineRPMUpdated = SteadyTimePoint::min();

        /// Engine runtime since start in seconds
        double EngineRuntime = 0.0;
        /// Engine runtime since start update time
        SteadyTimePoint EngineRuntimeUpdated = SteadyTimePoint::min();

        /// Fuel level in percentage (0.0 - 100.0)
        double FuelLevel = 0.0;
        /// Fuel level update time
        SteadyTimePoint FuelLevelUpdated = SteadyTimePoint::min();

        /// Intake air temperature in degrees Celsius
        double IntakeAirTemperature = 0.0;
        /// Intake air temperature update time
        SteadyTimePoint IntakeAirTemperatureUpdated = SteadyTimePoint::min();

        /// Odometer reading in kilometers
        double Odometer = 0.0;
        /// Odometer update time
        SteadyTimePoint OdometerUpdated = SteadyTimePoint::min();

        /// Relative throttle position in percentage (0.0 - 100.0)
        double RelativeThrottlePosition = 0.0;
        /// Relative throttle position update time
        SteadyTimePoint RelativeThrottlePositionUpdated = SteadyTimePoint::min();

        /// Throttle position in percentage (0.0 - 100.0)
        double ThrottlePosition = 0.0;
        /// Throttle position update time
        SteadyTimePoint ThrottlePositionUpdated = SteadyTimePoint::min();

        /// Vehicle speed in km/h
        double VehicleSpeed = 0.0;
        /// Vehicle speed update time
        SteadyTimePoint VehicleSpeedUpdated = SteadyTimePoint::min();

        /**
         * @brief Stale property order
         * @details First index indicates the MOST stale ODB2 Proc ID property.
         *          Last index indicates the LEAST stale ODB2 Proc ID property.
         */
        std::array<Obd2ProcID, 10> StalePidOrder = {
            Obd2ProcID::EngineCoolantTemperature,
            Obd2ProcID::EngineOilTemperature,
            Obd2ProcID::EngineRPM,
            Obd2ProcID::EngineRuntimeSinceStart,
            Obd2ProcID::FuelLevel,
            Obd2ProcID::IntakeAirTemperature,
            Obd2ProcID::Odometer,
            Obd2ProcID::RelativeThrottlePosition,
            Obd2ProcID::ThrottlePosition,
            Obd2ProcID::VehicleSpeed
        };

        /**
         * @brief Get the most stale OBD2 process ID and its last update time.
         * @return A StalePid struct containing the most stale OBD2 process ID and its last update time.
         */
        StalePid GetStalePid() const
        {
            const auto pid = StalePidOrder.front();
            SteadyTimePoint updateTime = SteadyTimePoint::min();
            switch (pid)
            {
                case Obd2ProcID::EngineCoolantTemperature:
                    updateTime = EngineCoolantTemperatureUpdated;
                    break;
                case Obd2ProcID::EngineOilTemperature:
                    updateTime = EngineOilTemperatureUpdated;
                    break;
                case Obd2ProcID::EngineRPM:
                    updateTime = EngineRPMUpdated;
                    break;
                case Obd2ProcID::EngineRuntimeSinceStart:
                    updateTime = EngineRuntimeUpdated;
                    break;
                case Obd2ProcID::FuelLevel:
                    updateTime = FuelLevelUpdated;
                    break;
                case Obd2ProcID::IntakeAirTemperature:
                    updateTime = IntakeAirTemperatureUpdated;
                    break;
                case Obd2ProcID::Odometer:
                    updateTime = OdometerUpdated;
                    break;
                case Obd2ProcID::RelativeThrottlePosition:
                    updateTime = RelativeThrottlePositionUpdated;
                    break;
                case Obd2ProcID::ThrottlePosition:
                    updateTime = ThrottlePositionUpdated;
                    break;
                case Obd2ProcID::VehicleSpeed:
                    updateTime = VehicleSpeedUpdated;
                    break;
                default:
                    break;
            }
            return { pid, updateTime };
        }

        /**
         * @brief Updates the state model with the given PID and value.
         * @param pid The OBD2 process ID corresponding to the value.
         * @param value The value to update in the state model.
         */
        void UpdateStateModel(Obd2ProcID pid, double value, bool passive = false)
        {
            const auto updateTime = std::chrono::time_point_cast<SteadyTimePoint::duration>(
                std::chrono::steady_clock::now()
            );

            switch (pid)
            {
                case Obd2ProcID::EngineCoolantTemperature:
                    EngineCoolantTemperature = value;
                    EngineCoolantTemperatureUpdated = updateTime;
                    break;
                case Obd2ProcID::EngineOilTemperature:
                    EngineOilTemperature = value;
                    EngineOilTemperatureUpdated = updateTime;
                    break;
                case Obd2ProcID::EngineRPM:
                    EngineRPM = value;
                    EngineRPMUpdated = updateTime;
                    break;
                case Obd2ProcID::EngineRuntimeSinceStart:
                    EngineRuntime = value;
                    EngineRuntimeUpdated = updateTime;
                    break;
                case Obd2ProcID::FuelLevel:
                    FuelLevel = value;
                    FuelLevelUpdated = updateTime;
                    break;
                case Obd2ProcID::IntakeAirTemperature:
                    IntakeAirTemperature = value;
                    IntakeAirTemperatureUpdated = updateTime;
                    break;
                case Obd2ProcID::Odometer:
                    Odometer = value;
                    OdometerUpdated = updateTime;
                    break;
                case Obd2ProcID::RelativeThrottlePosition:
                    RelativeThrottlePosition = value;
                    RelativeThrottlePositionUpdated = updateTime;
                    break;
                case Obd2ProcID::ThrottlePosition:
                    ThrottlePosition = value;
                    ThrottlePositionUpdated = updateTime;
                    break;
                case Obd2ProcID::VehicleSpeed:
                    VehicleSpeed = value;
                    VehicleSpeedUpdated = updateTime;
                    break;
                default:
                    break;
            }

            if (!passive)
            {
                auto iter = std::find(StalePidOrder.begin(), StalePidOrder.end(), pid);
                if (iter != StalePidOrder.end())
                {
                    // Shift every PID after this, one position left. Place this PID at the end.
                    std::rotate(iter, iter + 1, StalePidOrder.end());
                }
            }
        }
    };

    /**
     * @brief Manages the state model for OBD2 PIDs in a thread-safe manner.
     * @details Provides a mechanism to update and retrieve the state model for OBD2 PIDs in a
     *          thread-safe manner. Ensures readers always get a consistent snapshot of the state
     *          model.
     * @note Designed to be used in scenarios where multiple threads may be updating and reading
     *       the state model concurrently.
     */
    class StateModelManager
    {
    private:
        mutable std::mutex mtxPublish; // mutable due to GetSnapshot being const
        std::array<StateModel, 2> buffers_;
        const int kIdxPublished = 0;
        const int kIdxWrite = 1;
        const bool passive_ = true;
    public:
        StateModelManager() = default;
        explicit StateModelManager(bool passive)
            : passive_(passive)
        {}

        /**
         * @brief Updates the state model with a new value for a given OBD2 process ID.
         * @details This function updates the state model in a thread-safe manner. To ensure
         *          consistency, the new process ID value is always written to the write buffer
         *          state model. When no reads are currently reading the published buffer, the
         *          latest state model will be published.
         * @param pid The OBD2 process ID corresponding to the value being updated.
         * @param value The new value to update in the state model.
         * @note Relies on subsequent update calls to ensure latest state model is published when
         *       prevented concurrent current read operation.
         */
        void Update(CANSPI::Obd2ProcID pid, double value)
        {
            // Update the property in the private writing buffer
            buffers_[kIdxWrite].UpdateStateModel(pid, value, passive_);
            {
                // Publish only if a read is not currently executing
                std::unique_lock<std::mutex> lock(mtxPublish, std::try_to_lock);
                if (lock.owns_lock())
                {
                    // copy write buffer to published buffer.
                    buffers_[kIdxPublished] = buffers_[kIdxWrite];
                }
            }
        }

        /**
         * @brief Returns a snapshot of the published state model.
         * @details This function provides thread-safe access the latest state model. It will block
         *          to ensure that the returned snapshot is consistent and not affected by
         *          concurrent updates.
         * @return A copy of the published state model.
         */
        StateModel GetSnapshot() const
        {
            StateModel published;
            {
                // Block write update to published buffer
                std::scoped_lock<std::mutex> lock(mtxPublish);
                published = buffers_[kIdxPublished];
            }
            return published;
        }
    };
}