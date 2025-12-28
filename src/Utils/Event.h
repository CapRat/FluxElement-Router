//
// Created by benji on 17.12.25.
//

#ifndef RASPIHOST_EVENT_H
#define RASPIHOST_EVENT_H
#pragma once
#include <functional>
#include <unordered_map>
#include <cstdint>

template<typename Signature>
class Event;

template<typename Ret, typename... Args>
class Event<Ret(Args...)> {
public:
    using Callback = std::function<Ret(Args...)>;

    // Handle zum Entfernen eines Callbacks
    struct Handle {
        uint64_t id{};
        bool operator==(const Handle &other) const { return id == other.id; }
        explicit operator bool() const { return id != 0; }
    };

    Event() = default;

    ~Event() = default;

    // ----------------------------------
    // Hinzufügen
    // ----------------------------------

    // Mit Operator
    Handle operator+=(Callback cb) {
        return add(std::move(cb));
    }

    // Explizite Methode
    Handle add(Callback cb) {
        const uint64_t id = ++m_nextId;
        m_callbacks.emplace(id, std::move(cb));
        return Handle{id};
    }

    // ----------------------------------
    // Entfernen
    // ----------------------------------

    // Mit Operator
    void operator-=(Handle handle) {
        remove(handle);
    }

    // Explizite Methode
    void remove(Handle handle) {
        if (handle)
            m_callbacks.erase(handle.id);
    }

    // ----------------------------------
    // Event auslösen
    // ----------------------------------
    void operator()(Args... args) const {
        for (const auto &[id, cb]: m_callbacks) {
            cb(args...);
        }
    }

    // Alle Callbacks entfernen
    void clear() {
        m_callbacks.clear();
    }

    [[nodiscard]] bool empty() const {
        return m_callbacks.empty();
    }

private:
    std::unordered_map<uint64_t, Callback> m_callbacks;
    uint64_t m_nextId = 0;
};

#endif //RASPIHOST_EVENT_H
