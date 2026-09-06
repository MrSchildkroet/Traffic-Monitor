#pragma once

#include <optional>
#include <vector>

#include "../model/connection.hpp"

class ConsoleWriter
{
public:
    explicit ConsoleWriter(std::optional<Scope> filter = std::nullopt);
    void write_snapshot(const std::vector<Connection> &connections);
    void write_event(const Connection &conn);
    void write_header();

private:
    void write_row(const Connection &conn);
    void enableVirtualTerminalProcessing();
    bool passes_filter(const Connection &conn) const;

    std::optional<Scope> filter_;
};