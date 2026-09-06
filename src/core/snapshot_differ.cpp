#include "snapshot_differ.hpp"

#include <unordered_map>
#include <unordered_set>

SnapshotDiff SnapshotDiffer::diff(const std::vector<Connection> &current)
{
    SnapshotDiff result;

    std::unordered_set<std::string> previous_keys;
    previous_keys.reserve(previous_.size());

    for (const Connection &conn : previous_)
    {
        previous_keys.insert(connection_key(conn));
    }

    std::unordered_set<std::string> current_keys;
    current_keys.reserve(current.size());

    // added: im current, aber nicht im previous
    for (const Connection &conn : current)
    {
        std::string key = connection_key(conn);
        current_keys.insert(key);

        if (previous_keys.find(key) == previous_keys.end())
        {
            result.added.push_back(conn);
        }
    }

    // removed: im previous, aber nicht mehr im current
    for (const Connection &conn : previous_)
    {
        if (current_keys.find(connection_key(conn)) == current_keys.end())
        {
            result.removed.push_back(conn);
        }
    }

    previous_ = current;

    return result;
}