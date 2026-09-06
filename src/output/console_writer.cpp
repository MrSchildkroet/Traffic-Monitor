#include "console_writer.hpp"

#include "../core/scope_classifier.hpp"
#include "./color.hpp"

#include <iomanip>
#include <iostream>
#include <string>
#include <chrono>
#include <ctime>
#include <windows.h>
#include <stdexcept>

namespace
{

    const char *protocol_label(Protocol protocol)
    {
        switch (protocol)
        {
        case Protocol::Tcp:
            return "TCP";
        case Protocol::Udp:
            return "UDP";
        case Protocol::Icmp:
            return "ICMP";
        default:
            return "OTHER";
        }
    }

    const char *state_label(ConnectionState state)
    {
        switch (state)
        {
        case ConnectionState::Closed:
            return "CLOSED";
        case ConnectionState::Listen:
            return "LISTEN";
        case ConnectionState::SynSent:
            return "SYN_SENT";
        case ConnectionState::SynReceived:
            return "SYN_RCVD";
        case ConnectionState::Established:
            return "ESTABLISHED";
        case ConnectionState::FinWait1:
            return "FIN_WAIT1";
        case ConnectionState::FinWait2:
            return "FIN_WAIT2";
        case ConnectionState::CloseWait:
            return "CLOSE_WAIT";
        case ConnectionState::Closing:
            return "CLOSING";
        case ConnectionState::LastAck:
            return "LAST_ACK";
        case ConnectionState::TimeWait:
            return "TIME_WAIT";
        case ConnectionState::DeleteTcb:
            return "DELETE_TCB";
        default:
            return "-";
        }
    }

    std::string format_endpoint(const Endpoint &endpoint)
    {
        if (endpoint.address.empty())
        {
            return "*";
        }

        // IPv6 in Klammern, damit der :port lesbar bleibt
        bool is_v6 = endpoint.address.find(':') != std::string::npos;

        if (is_v6)
        {
            return "[" + endpoint.address + "]:" + std::to_string(endpoint.port);
        }

        return endpoint.address + ":" + std::to_string(endpoint.port);
    }

    std::string timestamp_now()
    {
        auto now = std::chrono::system_clock::now();
        std::time_t t = std::chrono::system_clock::to_time_t(now);

        std::tm local = {};
        localtime_s(&local, &t);

        char buffer[16] = {};
        std::strftime(buffer, sizeof(buffer), "%H:%M:%S", &local);

        return std::string(buffer);
    }

    std::string origin_prefix(const Connection &conn)
    {
        switch (conn.origin)
        {
        case Origin::SocketTable:
            if (conn.transition == Transition::Added)
            {
                return "[SOCK +]";
            }
            if (conn.transition == Transition::Removed)
            {
                return "[SOCK -]";
            }
            return "[SOCK  ]";

        case Origin::NetEvent:
            return "[WFP   ]";

        case Origin::Driver:
            return "[DRV   ]";

        default:
            return "[?     ]";
        }
    }

} // namespace

void ConsoleWriter::write_header()
{
    std::cout << std::left
              << std::setw(6) << "PROTO"
              << std::setw(12) << "STATE"
              << std::setw(24) << "LOCAL"
              << std::setw(24) << "REMOTE"
              << std::setw(8) << "PID"
              << std::setw(24) << "TIMESTAMP"
              << "PROCESS" << "\n";

    std::cout << std::string(86, '-') << "\n";
}

void ConsoleWriter::write_event(const Connection &conn)
{
    if (!passes_filter(conn))
    {
        return;
    }

    std::cout << origin_prefix(conn) << " ";
    write_row(conn);
}

void ConsoleWriter::write_row(const Connection &conn)
{
    std::string process = conn.process_name.empty() ? "<unknown>" : conn.process_name;

    std::cout << tcp_color(conn.state);

    std::cout
        << std::left
        << std::setw(6) << protocol_label(conn.protocol)
        << std::setw(12) << state_label(conn.state)
        << std::setw(24) << format_endpoint(conn.local)
        << std::setw(24) << format_endpoint(conn.remote)
        << std::setw(8) << conn.pid
        << std::setw(10) << timestamp_now()
        << process
        << ansi::reset << "\n";
}

void ConsoleWriter::write_snapshot(const std::vector<Connection> &connections)
{
    write_header();

    for (const Connection &conn : connections)
    {
        write_row(conn);

        std::cout << "\n"
                  << connections.size() << " connections\n";
    }
}

bool ConsoleWriter::passes_filter(const Connection &conn) const
{
    if (!filter_.has_value())
    {
        return true; // "all"
    }

    return classify_scope(conn) == filter_.value();
}

ConsoleWriter::ConsoleWriter(std::optional<Scope> filter) : filter_(filter)
{
    enableVirtualTerminalProcessing();
}

void ConsoleWriter::enableVirtualTerminalProcessing()
{
    HANDLE output_handle = GetStdHandle(STD_OUTPUT_HANDLE);

    if (output_handle == INVALID_HANDLE_VALUE)
    {
        throw std::runtime_error("[CONSOLE] Failed to get stdout handle");
    }

    DWORD console_mode = 0;

    if (!GetConsoleMode(output_handle, &console_mode))
    {
        throw std::runtime_error("[CONSOLE] Failed to get console mode");
    }

    if ((console_mode & ENABLE_VIRTUAL_TERMINAL_PROCESSING) == 0)
    {
        if (!SetConsoleMode(
                output_handle, console_mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING))
        {
            throw std::runtime_error("[CCONSOLE] Failed to enable virtual terminal proccessing");
        }
    }
}
