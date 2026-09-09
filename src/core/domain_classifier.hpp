#pragma once

#include <string>

class DnsResolver
{
public:
    std::string resolve_ip_address(const std::string &ip);
};