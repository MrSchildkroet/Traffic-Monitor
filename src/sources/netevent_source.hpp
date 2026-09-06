#pragma once

#include <windows.h>
#include <fwpmu.h>

#include "../core/event_queue.hpp"
#include "../model/connection.hpp"

class NetEventSource
{
public:
    explicit NetEventSource(EventQueue<Connection> &queue) : queue_(queue) {}
    ~NetEventSource();

    NetEventSource(const NetEventSource &) = delete;
    NetEventSource &operator=(const NetEventSource &) = delete;

    bool subscribe(HANDLE engine);
    void unsubscribe();

private:
    EventQueue<Connection> &queue_;
    HANDLE engine_ = nullptr;
    HANDLE subscription_ = nullptr;
};