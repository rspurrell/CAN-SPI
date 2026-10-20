#pragma once

#include "obd2_pids.hpp"
#include "state_model.hpp"

#include <array>
#include <atomic>
#include <chrono>
#include <string>
#include <thread>

namespace CANSPI
{
    struct CanSpiFrame
    {
        uint32_t id = 0U;
        bool extended = false;
        bool remoteRequest = false;
        uint8_t dataLength = 0U;
        std::array<uint8_t, 8> data{};
    };

    /**
     * @brief Represents a CAN SPI socket for communication with a CAN bus.
     * @details This class provides an interface for sending and receiving CAN frames over a CAN bus
     *          using the SocketCAN interface on Linux. It allows for opening a socket, sending
     *          frames, receiving frames, and managing the socket's state.
     * @note This class is designed to be used in applications that require communication with a CAN
     *       bus, such as automotive diagnostics.
     */
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

        /**
         * @brief Decode a received CAN Bus frame and update the state model snapshot.
         * @param frame The received CAN frame.
         * @return True if the frame contains a supported Mode 01 response.
         */
        bool ProcessFrame(const CanSpiFrame& frame);

        /**
         * @brief Return a snapshot of the current CAN Bus state model.
         * @return A consistent copy of the latest responder's state, or a default state.
         */
        StateModel GetSnapshot() const;

        /**
         * @brief Start the background worker to receive and process CAN Bus frames.
         * @param timeout Optional timeout for each receive operation. Zero (default) waits indefinitely.
         * @return True if the worker was started successfully, false otherwise.
         */
        bool BeginReceiveAsync(std::chrono::milliseconds timeout = std::chrono::milliseconds{0});

        /** @brief Stop the background receive worker and wait for it to finish. */
        void EndRecieveAsync();

    private:
        /**
         * @brief Receive frames continuously, processes recognized OBD2 responses, and updates
         *        CAN Bus state model via global state model manager.
         * @details Called from BeginReceiveAsync on worker thread. Operates until EndReceiveAsync is called.
         */
        void ReceiveAsync(std::chrono::milliseconds timeout);

        /// Linux file descriptor for the CAN SPI connection. Set to -1 when closed or uninitialized.
        int fd_ = -1;

        /// Name of the CAN SPI interface to use (e.g., "can0"). Default is "can0".
        std::string ifName_ = "can0";

        /// Manages a thread-safe CAN Bus state model.
        StateModelManager canState_{};

        /// Signals the background receive worker to stop.
        std::atomic<bool> stopWorker_{true};

        /// Library-owned async worker that receives and processes CAN frames.
        std::thread worker_;
    };
}