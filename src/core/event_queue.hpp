// core/event_queue.hpp
#pragma once

#include <condition_variable>
#include <mutex>
#include <optional>
#include <queue>
#include <utility>

// Thread-sichere FIFO-Queue. Ein oder mehrere Producer (z.B. der WFP-
// Callback-Thread) pushen, ein Consumer (der main-Thread) popt.
template <typename T>
class EventQueue
{
public:
    // Vom Producer-Thread aufgerufen. Kopiert/verschiebt das Item rein
    // und weckt einen wartenden Consumer.
    void push(T item)
    {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            queue_.push(std::move(item));
        }
        cv_.notify_one();
    }

    // Blockiert, bis ein Item da ist oder stop() gerufen wurde.
    // Gibt nullopt zurück, wenn gestoppt und leer (sauberes Beenden).
    std::optional<T> wait_pop()
    {
        std::unique_lock<std::mutex> lock(mutex_);

        cv_.wait(lock, [this]
                 { return !queue_.empty() || stopped_; });

        if (queue_.empty())
        {
            return std::nullopt; // nur erreichbar, wenn stopped_
        }

        T item = std::move(queue_.front());
        queue_.pop();

        return item;
    }

    // Nicht-blockierend: nimmt ein Item, wenn eins da ist, sonst nullopt.
    std::optional<T> try_pop()
    {
        std::lock_guard<std::mutex> lock(mutex_);

        if (queue_.empty())
        {
            return std::nullopt;
        }

        T item = std::move(queue_.front());
        queue_.pop();

        return item;
    }

    // Weckt alle wartenden Consumer, damit sie sauber austeigen können.
    void stop()
    {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            stopped_ = true;
        }
        cv_.notify_all();
    }

private:
    std::queue<T> queue_;
    std::mutex mutex_;
    std::condition_variable cv_;
    bool stopped_ = false;
};