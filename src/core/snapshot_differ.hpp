#pragma once

#include <vector>

#include "../model/connection.hpp"

struct SnapshotDiff
{
    std::vector<Connection> added;
    std::vector<Connection> removed;
};

class SnapshotDiffer
{
public:
    SnapshotDiff diff(const std::vector<Connection> &current);

private:
    std::vector<Connection> previous_;
};