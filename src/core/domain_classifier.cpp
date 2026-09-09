#include "domain_classifier.hpp";

#include <winsock2.h>
#include <ws2tcpip.h>

std::string DnsResolver::resolve_ip_address(const std::string &ip)
{
    sockaddr_in address{};
    address.sin_family = AF_INET;

    if (inet_pton(AF_INET, ip.c_str(), &address.sin_addr) != 1)
    {
        return {};
    }

    char host[NI_MAXHOST]{};

    const int result = getnameinfo(
        reinterpret_cast<sockaddr *>(&address),
        sizeof(address),
        host,
        sizeof(host),
        nullptr,
        0,
        NI_NAMEREQD);

    if (result != 0)
    {
        return {};
    }

    return host;
}
