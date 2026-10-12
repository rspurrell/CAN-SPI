#pragma once

#include "obd2_pids.hpp"

#include <array>
#include <string>

namespace CANSPI
{
    class CanSpiSocket
    {
    public:
        /** @brief Constructs a CAN SPI socket through the can0 interface.*/
        CanSpiSocket() = default;
        /** @brief Constructs a CAN SPI socket through the specified interface. */
        explicit CanSpiSocket(std::string interfaceName);
        ~CanSpiSocket();

        /**
         * @brief Open the CAN SPI socket and bind it to the specified interface.
         *
         * @return True when initialization succeeds.
         */
        bool Open();

        /** @brief Closes the CAN SPI socket when open. */
        void Close() noexcept;

        /**
         * @brief Determines whether the CAN SPI socket interface is open.
         *
         * @return True when the CAN SPI socket interface is active.
         */
        bool IsOpen() const noexcept;

    private:
        /// Linux file descriptor for the CAN SPI connection. Set to -1 when closed or uninitialized.
        int fd_ = -1;

        /// Name of the CAN SPI interface to use (e.g., "can0"). Default is "can0".
        std::string ifName_ = "can0";
    };
}