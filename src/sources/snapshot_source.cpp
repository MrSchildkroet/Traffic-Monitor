#include "snapshot_source.hpp"

#include <winsock2.h>
#include <WS2tcpip.h>
#include <windows.h>
#include <iphlpapi.h>
#include <psapi.h>

#include <array>
#include <iostream>
#include <vector>

namespace
{
    constexpr uint32_t INVALID_PID = 0;

    ConnectionState map_tcp_state(DWORD state)
    {
        switch (state)
        {
        case MIB_TCP_STATE_CLOSED:
            return ConnectionState::Closed;
        case MIB_TCP_STATE_LISTEN:
            return ConnectionState::Listen;
        case MIB_TCP_STATE_SYN_SENT:
            return ConnectionState::SynSent;
        case MIB_TCP_STATE_SYN_RCVD:
            return ConnectionState::SynReceived;
        case MIB_TCP_STATE_ESTAB:
            return ConnectionState::Established;
        case MIB_TCP_STATE_FIN_WAIT1:
            return ConnectionState::FinWait1;
        case MIB_TCP_STATE_FIN_WAIT2:
            return ConnectionState::FinWait2;
        case MIB_TCP_STATE_CLOSE_WAIT:
            return ConnectionState::CloseWait;
        case MIB_TCP_STATE_CLOSING:
            return ConnectionState::Closing;
        case MIB_TCP_STATE_LAST_ACK:
            return ConnectionState::LastAck;
        case MIB_TCP_STATE_TIME_WAIT:
            return ConnectionState::TimeWait;
        case MIB_TCP_STATE_DELETE_TCB:
            return ConnectionState::DeleteTcb;
        default:
            return ConnectionState::None;
        }
    }

    std::string format_v4(DWORD addr)
    {
        char buffer[INET_ADDRSTRLEN] = {};
        inet_ntop(AF_INET, &addr, buffer, sizeof(buffer));

        return std::string(buffer);
    }

    std::string format_v6(const UCHAR addr[16])
    {
        char buffer[INET6_ADDRSTRLEN] = {};
        inet_ntop(AF_INET6, addr, buffer, sizeof(buffer));

        return std::string(buffer);
    }

    uint16_t port_from_dword(DWORD port)
    {
        return ntohs(static_cast<u_short>(port));
    }

    template <typename Fn>
    std::vector<BYTE> fetch_table(Fn get_table, const char *label)
    {
        DWORD size = 0;
        DWORD result = get_table(nullptr, &size);

        if (result != ERROR_INSUFFICIENT_BUFFER && result != NO_ERROR)
        {
            std::cerr << "[SNAPSHOT] " << label << " sizing failed: " << result << "\n";

            return {};
        }

        std::vector<BYTE> buffer(size);
        result = get_table(buffer.data(), &size);

        if (result != NO_ERROR)
        {
            std::cerr << "[SNAPSHOT] " << label << " fetch failed: " << result << "\n";

            return {};
        }

        return buffer;
    }
} // namespace

std::string SnapshotSource::resolve_process_name(uint32_t pid)
{
    if (pid == INVALID_PID)
    {
        return {};
    }

    HANDLE process = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, static_cast<DWORD>(pid));

    if (!process)
    {
        return {};
    }

    wchar_t path[MAX_PATH] = {};
    DWORD path_size = MAX_PATH;
    std::string name;

    if (QueryFullProcessImageNameW(process, 0, path, &path_size))
    {
        const wchar_t *file = wcschr(path, L'\\');
        const wchar_t *start = file ? file + 1 : path;

        int len = WideCharToMultiByte(CP_UTF8, 0, start, -1, nullptr, 0, nullptr, nullptr);

        if (len > 0)
        {
            name.resize(static_cast<size_t>(len - 1));
            WideCharToMultiByte(
                CP_UTF8, 0, start, -1, name.data(), len, nullptr, nullptr);
        }
    }

    CloseHandle(process);

    return name;
}

void SnapshotSource::collect_tcp_v4(std::vector<Connection> &out)
{
    std::vector<BYTE> buffer = fetch_table([](void *buf, DWORD *size)
                                           { return GetExtendedTcpTable(buf, size, FALSE, AF_INET, TCP_TABLE_OWNER_PID_ALL, 0); }, "tcp_v4");

    if (buffer.empty())
    {
        return;
    }

    auto *table = reinterpret_cast<MIB_TCPTABLE_OWNER_PID *>(buffer.data());

    for (DWORD i = 0; i < table->dwNumEntries; ++i)
    {
        const MIB_TCPROW_OWNER_PID &row = table->table[i];

        Connection conn;
        conn.protocol = Protocol::Tcp;
        conn.state = map_tcp_state(row.dwState);
        conn.local = {format_v4(row.dwLocalAddr), port_from_dword(row.dwLocalPort)};
        conn.remote = {format_v4(row.dwRemoteAddr), port_from_dword(row.dwRemotePort)};
        conn.pid = row.dwOwningPid;
        conn.process_name = resolve_process_name(conn.pid);

        out.push_back(std::move(conn));
    }
}

void SnapshotSource::collect_tcp_v6(std::vector<Connection> &out)
{
    std::vector<BYTE> buffer = fetch_table(
        [](void *buf, DWORD *size)
        {
            return GetExtendedTcpTable(
                buf, size, FALSE, AF_INET6, TCP_TABLE_OWNER_PID_ALL, 0);
        },
        "tcp_v6");

    if (buffer.empty())
    {
        return;
    }

    auto *table = reinterpret_cast<_MIB_TCP6TABLE_OWNER_PID *>(buffer.data());

    for (DWORD i = 0; i < table->dwNumEntries; ++i)
    {
        const MIB_TCP6ROW_OWNER_PID &row = table->table[i];

        Connection conn;
        conn.protocol = Protocol::Tcp;
        conn.state = map_tcp_state(row.dwState);
        conn.local = {format_v6(row.ucLocalAddr), port_from_dword(row.dwLocalPort)};
        conn.remote = {format_v6(row.ucRemoteAddr),
                       port_from_dword(row.dwRemotePort)};
        conn.pid = row.dwOwningPid;
        conn.process_name = resolve_process_name(conn.pid);

        out.push_back(std::move(conn));
    }
}

void SnapshotSource::collect_udp_v4(std::vector<Connection> &out)
{
    std::vector<BYTE> buffer = fetch_table(
        [](void *buf, DWORD *size)
        {
            return GetExtendedUdpTable(
                buf, size, FALSE, AF_INET, UDP_TABLE_OWNER_PID, 0);
        },
        "udp_v4");

    if (buffer.empty())
    {
        return;
    }

    auto *table = reinterpret_cast<MIB_UDPTABLE_OWNER_PID *>(buffer.data());

    for (DWORD i = 0; i < table->dwNumEntries; ++i)
    {
        const MIB_UDPROW_OWNER_PID &row = table->table[i];

        Connection conn;
        conn.protocol = Protocol::Udp;
        conn.state = ConnectionState::None;
        conn.local = {format_v4(row.dwLocalAddr), port_from_dword(row.dwLocalPort)};
        conn.pid = row.dwOwningPid;
        conn.process_name = resolve_process_name(conn.pid);

        out.push_back(std::move(conn));
    }
}

void SnapshotSource::collect_udp_v6(std::vector<Connection> &out)
{
    std::vector<BYTE> buffer = fetch_table(
        [](void *buf, DWORD *size)
        {
            return GetExtendedUdpTable(
                buf, size, FALSE, AF_INET6, UDP_TABLE_OWNER_PID, 0);
        },
        "udp_v6");

    if (buffer.empty())
    {
        return;
    }

    auto *table = reinterpret_cast<MIB_UDP6TABLE_OWNER_PID *>(buffer.data());

    for (DWORD i = 0; i < table->dwNumEntries; ++i)
    {
        const MIB_UDP6ROW_OWNER_PID &row = table->table[i];

        Connection conn;
        conn.protocol = Protocol::Udp;
        conn.state = ConnectionState::None;
        conn.local = {format_v6(row.ucLocalAddr),
                      port_from_dword(row.dwLocalPort)};
        conn.pid = row.dwOwningPid;
        conn.process_name = resolve_process_name(conn.pid);

        out.push_back(std::move(conn));
    }
}

std::vector<Connection> SnapshotSource::collect()
{
    std::vector<Connection> out;

    collect_tcp_v4(out);
    collect_tcp_v6(out);
    collect_udp_v4(out);
    collect_udp_v6(out);

    return out;
}
