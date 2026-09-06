#pragma once

#include <vector>

#include "../model/connection.hpp"

// Zieht einen vollständigen Snapshot aller aktiven TCP/UDP-Verbindungen
// (IPv4 + IPv6) aus den IpHlpApi-Tabellen.
class SnapshotSource
{
public:
    std::vector<Connection> collect();

private:
    void collect_tcp_v4(std::vector<Connection> &out);
    void collect_tcp_v6(std::vector<Connection> &out);
    void collect_udp_v4(std::vector<Connection> &out);
    void collect_udp_v6(std::vector<Connection> &out);
    std::string resolve_process_name(uint32_t pid);
};