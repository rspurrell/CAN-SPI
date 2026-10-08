/******************************************************************************
 * @file    obd2_pids.hpp
 * @brief   OBD2 Process IDs and convertions
 ******************************************************************************/

#pragma once

#include <array>
#include <cstdint>
#include <string_view>

namespace CANSPI
{
    /**
     * @brief OBD2 Process IDs (PIDs) defined by SAE J1979
     */
    enum class Obd2ProcID : uint8_t
    {
        EngineCoolantTemperature = 0x05,
        ThrottlePosition = 0x11,
        EngineRPM = 0x0C,
        VehicleSpeed = 0x0D,
        IntakeAirTemperature = 0x0F,
        EngineRuntimeSinceStart = 0x1F,
        FuelLevel = 0x2F,
        RelativeThrottlePosition = 0x45,
        EngineOilTemperature = 0x5C,
        Odometer = 0xA6
    };

    /**
     * @brief Information about an OBD2 PID, including its name, unit, formula, and conversion function.
     */
    struct Obd2PidInfo
    {
        Obd2ProcID pid;
        std::string_view name;
        std::string_view unit;
        std::string_view formula;

        double (*convert)(const std::array<uint8_t, 8>& payload);
    };

    /**
     * @brief Internal library use only. DO NOT USE.
     */
    namespace internal
    {
        inline constexpr double calc_A_times_256_plus_B(const std::array<uint8_t, 8>& p)
        {
            const auto a = p[3];
            const auto b = p[4];
            return static_cast<double>((static_cast<uint16_t>(a) << 8U) | b);
        }

        inline constexpr double calc_percent(const std::array<uint8_t, 8>& p)
        {
            const auto a = p[3];
            return 100.0 * static_cast<double>(a) / 255.0;
        }

        inline constexpr double calc_temperature(const std::array<uint8_t, 8>& p)
        {
            const auto a = p[3];
            return static_cast<double>(a) - 40.0;
        }

        #define FUNC_ANON_CONVERT(...) [](const std::array<uint8_t, 8>& p) constexpr -> double { __VA_ARGS__ }

        /**
         * @brief OBD2 PID definitions
         */
        inline constexpr std::array<Obd2PidInfo, 10> kObd2PidDefinitions = {{
            { Obd2ProcID::ThrottlePosition, "Throttle Position", "%", "(100 * A) / 255", internal::calc_percent },
            { Obd2ProcID::RelativeThrottlePosition, "Relative Throttle Position", "%", "(100 * A) / 255", internal::calc_percent },
            { Obd2ProcID::VehicleSpeed, "Vehicle Speed", "kph", "A", FUNC_ANON_CONVERT(
                const auto a = p[3];
                return static_cast<double>(a);
            )},
            { Obd2ProcID::EngineRPM, "RPM", "rpm", "((256 * A) + B) / 4", FUNC_ANON_CONVERT(
                return internal::calc_A_times_256_plus_B(p) / 4.0;
            )},
            { Obd2ProcID::EngineRuntimeSinceStart, "Engine Runtime", "s", "(256 * A) + B", internal::calc_A_times_256_plus_B },
            { Obd2ProcID::FuelLevel, "Fuel Level", "%", "(100 * A) / 255", internal::calc_percent },
            { Obd2ProcID::IntakeAirTemperature, "Intake Air Temp", "°C", "A - 40", internal::calc_temperature },
            { Obd2ProcID::EngineCoolantTemperature, "Engine Coolant Temp", "°C", "A - 40", internal::calc_temperature },
            { Obd2ProcID::EngineOilTemperature, "Engine Oil Temp", "°C", "A - 40", internal::calc_temperature },
            { Obd2ProcID::Odometer, "Odometer", "km", "(2^24 * A) + (2^16 * B) + (2^8 * C) + D", FUNC_ANON_CONVERT(
                const auto a = p[3];
                const auto b = p[4];
                const auto c = p[5];
                const auto d = p[6];
                return static_cast<double>(
                    (static_cast<uint32_t>(a) << 24U) |
                    (static_cast<uint32_t>(b) << 16U) |
                    (static_cast<uint32_t>(c) << 8U) |
                    static_cast<uint32_t>(d)
                ) / 10.0;
            )}
        }};

        /**
         * @brief OBD2 definition jump table for fast lookup
         */
        inline const std::array<const Obd2PidInfo*, 256> kObdPidJumpTable = []()
        {
            std::array<const Obd2PidInfo*, 256> jumpTable{};
            for (const auto& active : kObd2PidDefinitions)
            {
                jumpTable[static_cast<uint8_t>(active.pid)] = &active;
            }
            return jumpTable;
        }();
    }

    /**
     * @brief Get OBD2 PID information for a given process ID.
     * @return Pointer to a Obd2PidInfo struct, or nullptr if the process ID is not defined.
     */
    inline const Obd2PidInfo* GetObd2PidInfo(const Obd2ProcID pid)
    {
        return internal::kObdPidJumpTable[static_cast<uint8_t>(pid)];
    }
}
