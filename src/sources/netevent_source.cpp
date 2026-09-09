#include "netevent_source.hpp"

#include <iostream>

namespace
{
    void CALLBACK on_net_event(void *context,
                               const FWPM_NET_EVENT5 *event)
    {
        if (!context || !event)
        {
            return;
        }

        auto *queue = static_cast<EventQueue<Connection> *>(context);

        Connection conn;
        conn.origin = Origin::NetEvent;

        if (event->header.flags & FWPM_NET_EVENT_FLAG_IP_PROTOCOL_SET)
        {
            switch (event->header.ipProtocol)
            {
            case IPPROTO_TCP:
                conn.protocol = Protocol::Tcp;
                break;
            case IPPROTO_UDP:
                conn.protocol = Protocol::Udp;
                break;
            case IPPROTO_ICMP:
                conn.protocol = Protocol::Icmp;
                break;

            default:
                conn.protocol = Protocol::Other;
                break;
            }
        }

        queue->push(std::move(conn));
    }

}

NetEventSource::~NetEventSource()
{
    unsubscribe();
}

bool NetEventSource::subscribe(HANDLE engine)
{
    if (!engine)
    {
        std::cerr << "[NETEVENT] no engine handle\n";

        return false;
    }

    engine_ = engine;

    FWPM_NET_EVENT_SUBSCRIPTION0 subscription = {};
    FWPM_NET_EVENT_ENUM_TEMPLATE0 enum_template = {};
    subscription.enumTemplate = &enum_template;

    DWORD result = FwpmNetEventSubscribe4(
        engine_,
        &subscription,
        on_net_event,
        &queue_,
        &subscription_);

    if (result != ERROR_SUCCESS)
    {
        std::cerr << "[NETEVENT] FwpmNetEventSubscribe4 failed: " << result << "\n";
        engine_ = nullptr;

        return false;
    }

    std::cout << "[NETEVENT] subscribed\n";

    return true;
}

void NetEventSource::unsubscribe()
{
    if (subscription_ && engine_)
    {
        FwpmNetEventUnsubscribe0(engine_, subscription_);
        subscription_ = nullptr;
    }

    engine_ = nullptr;
}