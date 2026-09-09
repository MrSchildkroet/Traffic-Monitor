#include "./core/domain_classifier.hpp"

#include <winsock2.h>
#include <windows.h>

#include <atomic>
#include <chrono>
#include <cstring>
#include <iostream>
#include <optional>
#include <string>
#include <thread>

#include "cnm.hpp"
#include "core/event_queue.hpp"
#include "core/snapshot_differ.hpp"
#include "output/console_writer.hpp"
#include "sources/netevent_source.hpp"
#include "sources/snapshot_source.hpp"

namespace
{
    constexpr int POLL_INTERVAL_MS = 1000;

    std::optional<Scope> parse_filter(int argc, char **argv)
    {
        for (int i = 0; i < argc - 1; ++i)
        {
            if (std::strcmp(argv[i], "--filter") == 0)
            {
                std::string value = argv[i + 1];

                if (value == "all")
                {
                    return std::nullopt;
                }

                if (value == "external")
                {
                    return Scope::External;
                }

                if (value == "internal")
                {
                    return Scope::Internal;
                }

                if (value == "loopback")
                {
                    return Scope::Loopback;
                }

                std::cerr << "[MAIN] unknown filter '" << value
                          << "', falling back to all\n";
            }
        }

        return std::nullopt;
    }

    void polling_loop(EventQueue<Connection> &queue, std::atomic<bool> &running, DnsResolver &resolver)
    {
        SnapshotSource source(resolver);
        SnapshotDiffer differ;

        while (running.load())
        {
            std::vector<Connection> current = source.collect();
            SnapshotDiff diff = differ.diff(current);

            for (Connection conn : diff.added)
            {
                conn.origin = Origin::SocketTable;
                conn.transition = Transition::Added;
                queue.push(std::move(conn));
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(POLL_INTERVAL_MS));
        }
    }

}

int main(int argc, char **argv)
{
    WSADATA wsa_data = {};
    int wsa_result = WSAStartup(MAKEWORD(2, 2), &wsa_data);

    if (wsa_result != 0)
    {
        std::cerr << "[MAIN] WSAStartup failed: " << wsa_result << "\n";

        return 1;
    }

    std::optional<Scope> filter = parse_filter(argc, argv);

    CnmEngine engine;

    if (!engine.open())
    {
        std::cerr << "[MAIN] engine failed\n";
        WSACleanup();

        return 1;
    }

    std::cout << "[MAIN] engine ok\n";

    DnsResolver resolver;

    EventQueue<Connection> queue;

    NetEventSource net_source(queue);

    if (!net_source.subscribe(engine.handle()))
    {
        std::cerr << "[MAIN] subscribe failed\n";
        WSACleanup();

        return 1;
    }

    std::atomic<bool> running{true};
    std::thread poller(polling_loop, std::ref(queue), std::ref(running), std::ref(resolver));

    ConsoleWriter writer(filter);

    std::cout
        << "[MAIN] monitoring started.\n";

    writer.write_header();

    while (true)
    {
        std::optional<Connection> item = queue.wait_pop();

        if (!item.has_value())
        {
            break;
        }

        writer.write_event(item.value());
    }

    running.store(false);
    poller.join();
    WSACleanup();

    return 0;
}