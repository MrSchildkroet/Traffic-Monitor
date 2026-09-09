#pragma once

#include <cstdint>
#include <string>

enum class Protocol
{
    Tcp,
    Udp,
    Icmp,
    Other
};

enum class Direction
{
    Inbound,
    Outbound,
    Unknown
};

enum class Origin
{
    SocketTable,
    NetEvent,
    Driver
};

enum class Transition
{
    None,
    Added,
    Removed,
};

enum class ConnectionState
{
    None,
    Closed,
    Listen,
    SynSent,
    SynReceived,
    Established,
    FinWait1,
    FinWait2,
    CloseWait,
    Closing,
    LastAck,
    TimeWait,
    DeleteTcb
};

struct Endpoint
{
    std::string address;
    uint16_t port = 0;
    std::string domain;
};

struct Connection
{
    Protocol protocol = Protocol::Other;
    Direction direction = Direction::Unknown;
    ConnectionState state = ConnectionState::None;

    Endpoint local;
    Endpoint remote;

    uint32_t pid = 0;
    std::string process_name;

    Origin origin = Origin::SocketTable;
    Transition transition = Transition::None;
};

inline std::string connection_key(const Connection &conn)
{
    return std::to_string(static_cast<int>(conn.protocol)) + "|" + conn.local.address + ":" + std::to_string(conn.local.port) + "|" + conn.remote.address + ":" + std::to_string(conn.remote.port);
}

enum class Scope
{
    Loopback,
    Internal,
    External
};