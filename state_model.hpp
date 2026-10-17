#pragma once

#include <array>
#include <mutex>

#include "obd2_pids.hpp"

namespace CANSPI
{
    struct StateModel
    {
        /// Engine coolant temperature in degrees Celsius
        double EngineCoolantTemperature = 0.0;
        /// Engine oil temperature in degrees Celsius
        double EngineOilTemperature = 0.0;
        /// Engine RPM
        double EngineRPM = 0.0;
        /// Engine runtime since start in seconds
        double EngineRuntime = 0.0;
        /// Fuel level in percentage (0.0 - 100.0)
        double FuelLevel = 0.0;
        /// Intake air temperature in degrees Celsius
        double IntakeAirTemperature = 0.0;
        /// Odometer reading in kilometers
        double Odometer = 0.0;
        /// Relative throttle position in percentage (0.0 - 100.0)
        double RelativeThrottlePosition = 0.0;
        /// Throttle position in percentage (0.0 - 100.0)
        double ThrottlePosition = 0.0;
        /// Vehicle speed in km/h
        double VehicleSpeed = 0.0;

        /**
         * @brief Updates the state model with the given PID and value.
         * @param pid The OBD2 process ID corresponding to the value.
         * @param value The value to update in the state model.
         */
        void UpdateStateModel(CANSPI::Obd2ProcID pid, double value)
        {
            switch (pid)
            {
                case CANSPI::Obd2ProcID::EngineCoolantTemperature:
                    EngineCoolantTemperature = value;
                    break;
                case CANSPI::Obd2ProcID::EngineOilTemperature:
                    EngineOilTemperature = value;
                    break;
                case CANSPI::Obd2ProcID::EngineRPM:
                    EngineRPM = value;
                    break;
                case CANSPI::Obd2ProcID::EngineRuntimeSinceStart:
                    EngineRuntime = value;
                    break;
                case CANSPI::Obd2ProcID::FuelLevel:
                    FuelLevel = value;
                    break;
                case CANSPI::Obd2ProcID::IntakeAirTemperature:
                    IntakeAirTemperature = value;
                    break;
                case CANSPI::Obd2ProcID::Odometer:
                    Odometer = value;
                    break;
                case CANSPI::Obd2ProcID::RelativeThrottlePosition:
                    RelativeThrottlePosition = value;
                    break;
                case CANSPI::Obd2ProcID::ThrottlePosition:
                    ThrottlePosition = value;
                    break;
                case CANSPI::Obd2ProcID::VehicleSpeed:
                    VehicleSpeed = value;
                    break;
                default:
                    break;
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
    public:
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
            buffers_[kIdxWrite].UpdateStateModel(pid, value);
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