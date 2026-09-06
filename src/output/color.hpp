#pragma once
#include <string>
#include "../model/connection.hpp"

namespace ansi
{
    inline const std::string reset = "\033[0m";

    inline const std::string black = "\033[30m";
    inline const std::string red = "\033[31m";
    inline const std::string green = "\033[32m";
    inline const std::string yellow = "\033[33m";
    inline const std::string blue = "\033[34m";
    inline const std::string magenta = "\033[35m";
    inline const std::string cyan = "\033[36m";
    inline const std::string white = "\033[37m";

    inline const std::string bright_red = "\033[91m";
    inline const std::string bright_black = "\033[90m"; // grau
}

inline std::string tcp_color(ConnectionState s)
{
    switch (s)
    {
    case ConnectionState::None:
        return ansi::white;
    case ConnectionState::Closed:
        return ansi::bright_black;
    case ConnectionState::Listen:
        return ansi::blue;
    case ConnectionState::SynSent:
        return ansi::yellow;
    case ConnectionState::SynReceived:
        return ansi::yellow;
    case ConnectionState::Established:
        return ansi::green;
    case ConnectionState::FinWait1:
        return ansi::magenta;
    case ConnectionState::FinWait2:
        return ansi::magenta;
    case ConnectionState::CloseWait:
        return ansi::red;
    case ConnectionState::Closing:
        return ansi::red;
    case ConnectionState::LastAck:
        return ansi::red;
    case ConnectionState::TimeWait:
        return ansi::cyan;
    case ConnectionState::DeleteTcb:
        return ansi::bright_red;
    }

    return ansi::white;
}

inline std::string tcp_state_to_string(ConnectionState s)
{
    switch (s)
    {
    case ConnectionState::None:
        return "None";
    case ConnectionState::Closed:
        return "Closed";
    case ConnectionState::Listen:
        return "Listen";
    case ConnectionState::SynSent:
        return "SynSent";
    case ConnectionState::SynReceived:
        return "SynReceived";
    case ConnectionState::Established:
        return "Established";
    case ConnectionState::FinWait1:
        return "FinWait1";
    case ConnectionState::FinWait2:
        return "FinWait2";
    case ConnectionState::CloseWait:
        return "CloseWait";
    case ConnectionState::Closing:
        return "Closing";
    case ConnectionState::LastAck:
        return "LastAck";
    case ConnectionState::TimeWait:
        return "TimeWait";
    case ConnectionState::DeleteTcb:
        return "DeleteTcb";
    }
    return "Unknown";
}

inline std::string color_state(ConnectionState s)
{
    return tcp_color(s) + tcp_state_to_string(s) + ansi::reset;
}