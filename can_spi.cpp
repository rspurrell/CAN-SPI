#include "can_spi.hpp"

#include <cerrno>
#include <cstring>
#include <iostream>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

#include <linux/can.h>
#include <linux/can/raw.h>

namespace CANSPI
{
    CanSpiSocket::CanSpiSocket(std::string interfaceName)
        : ifName_(std::move(interfaceName))
    {
        if (ifName_.empty())
        {
            ifName_ = "can0";
        }
    }

    CanSpiSocket::~CanSpiSocket()
    {
        Close();
    }

    bool CanSpiSocket::Open()
    {
        if (IsOpen())
        {
            return true;
        }

        fd_ = ::socket(PF_CAN, SOCK_RAW, CAN_RAW);
        if (fd_ < 0)
        {
            std::cerr << "Failed to open CAN socket: " << std::strerror(errno) << std::endl;
            return false;
        }

        struct ifreq ifr{};
        std::strncpy(ifr.ifr_name, ifName_.c_str(), IFNAMSIZ - 1U);
        ifr.ifr_name[IFNAMSIZ - 1U] = '\0';

        if (ioctl(fd_, SIOCGIFINDEX, &ifr) < 0)
        {
            std::cerr << "Failed to resolve CAN interface '" << ifName_ << "': " << std::strerror(errno) << std::endl;
            Close();
            return false;
        }

        struct sockaddr_can addr{};
        std::memset(&addr, 0, sizeof(addr));
        addr.can_family = AF_CAN;
        addr.can_ifindex = ifr.ifr_ifindex;

        if (::bind(fd_, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) < 0)
        {
            std::cerr << "Failed to bind CAN socket to '" << ifName_ << "': " << std::strerror(errno) << std::endl;
            Close();
            return false;
        }

        return true;
    }

    void CanSpiSocket::Close() noexcept
    {
        if (fd_ >= 0)
        {
            ::close(fd_);
            fd_ = -1;
        }
    }

    bool CanSpiSocket::IsOpen() const noexcept
    {
        return fd_ >= 0;
    }

    bool CanSpiSocket::Receive(CanSpiFrame& frame, std::chrono::milliseconds timeout) const
    {
        if (!IsOpen())
        {
            return false;
        }

        if (timeout > std::chrono::milliseconds::zero())
        {
            // block until SocketCAN file descriptor becomes readable, but only until timeout.
            fd_set readFds{};
            FD_ZERO(&readFds);
            FD_SET(fd_, &readFds);

            // convert milliseconds to timeval
            timeval tv{};
            tv.tv_sec = static_cast<long>(timeout.count() / 1000);
            tv.tv_usec = static_cast<long>((timeout.count() % 1000) * 1000);

            const int result = select(fd_ + 1, &readFds, nullptr, nullptr, &tv);
            // zero: timed out. negative: failed.
            if (result <= 0)
            {
                return false;
            }
        }

        struct can_frame raw{};
        const auto bytesRead = ::read(fd_, &raw, sizeof(raw));
        if (bytesRead != static_cast<ssize_t>(sizeof(raw)))
        {
            return false;
        }

        frame.id = raw.can_id & CAN_EFF_MASK;
        frame.extended = (raw.can_id & CAN_EFF_FLAG) != 0U;
        frame.remoteRequest = (raw.can_id & CAN_RTR_FLAG) != 0U;
        frame.dataLength = raw.can_dlc;
        frame.data.fill(0U);
        for (std::size_t i = 0; i < static_cast<std::size_t>(raw.can_dlc); ++i)
        {
            frame.data[i] = raw.data[i];
        }

        return true;
    }
}