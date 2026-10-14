#pragma once

#include "obd2_pids.hpp"

#include <array>
#include <chrono>
#include <string>

namespace CANSPI
{
    struct CanSpiFrame
    {
        uint32_t id = 0U;
        bool extended = false;
        bool remote = false;
        uint8_t dataLength = 0U;
        std::array<uint8_t, 8> data{};
    };

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

        /**
         * @brief Send a CAN frame through the socket.
         * @param frame The frame to send.
         * @return True if the frame was sent successfully, false otherwise.
         */
        bool Send(const CanSpiFrame& frame) const;

        /**
         * @brief Receive a CAN frame from the socket with an optional timeout.
         * @param frame [out] The frame to receive.
         * @param timeout Optional timeout duration for receiving the frame. Default is zero (no timeout).
         * @return True if the frame was received successfully, false otherwise.
         */
        bool Receive(CanSpiFrame& frame, std::chrono::milliseconds timeout = std::chrono::milliseconds{0}) const;

    private:
        /// Linux file descriptor for the CAN SPI connection. Set to -1 when closed or uninitialized.
        int fd_ = -1;

        /// Name of the CAN SPI interface to use (e.g., "can0"). Default is "can0".
        std::string ifName_ = "can0";
    };
}