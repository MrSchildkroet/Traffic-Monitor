#include "scope_classifier.hpp"

#include <string>

namespace
{

    bool starts_with(const std::string &value, const std::string &prefix)
    {
        return value.rfind(prefix, 0) == 0;
    }

    Scope classify_address(const std::string &addr)
    {
        if (addr.empty())
        {
            return Scope::Internal;
        }

        // --- IPv4 ---
        if (starts_with(addr, "127."))
        {
            return Scope::Loopback;
        }

        if (addr == "0.0.0.0")
        {
            return Scope::Internal; // Wildcard, kein echtes Ziel
        }

        if (starts_with(addr, "10.") ||
            starts_with(addr, "192.168.") ||
            starts_with(addr, "169.254."))
        { // Link-Local v4
            return Scope::Internal;
        }

        // 172.16.0.0 - 172.31.255.255
        if (starts_with(addr, "172."))
        {
            size_t dot = addr.find('.', 4);

            if (dot != std::string::npos)
            {
                int second = std::stoi(addr.substr(4, dot - 4));

                if (second >= 16 && second <= 31)
                {
                    return Scope::Internal;
                }
            }
        }

        // --- IPv6 ---
        if (addr == "::1")
        {
            return Scope::Loopback;
        }

        if (addr == "::")
        {
            return Scope::Internal; // Wildcard
        }

        if (starts_with(addr, "fe80:") || // Link-Local
            starts_with(addr, "fc") ||    // Unique Local (fc00::/7)
            starts_with(addr, "fd"))
        {
            return Scope::Internal;
        }

                return Scope::External;
    }

} // namespace

Scope classify_scope(const Connection &conn)
{
    return classify_address(conn.remote.address);
}