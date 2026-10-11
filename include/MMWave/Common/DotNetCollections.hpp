#pragma once

#include <cstddef>
#include <optional>
#include <unordered_map>
#include <utility>
#include <vector>

#include "MMWave/Common/Exceptions.hpp"

/// Containers that enumerate in the same order as .NET's Dictionary<TKey, TValue> and
/// HashSet<T>.
///
/// The C# association code walks dictionaries and sets and acts on what it finds in that order
/// (which wire ids are dropped first, which objects are removed first, the order targets are
/// listed and drawn in). .NET enumerates those collections in entry-slot order, and a removed
/// entry's slot is reused (last freed, first reused) by the next insertion -- not in insertion
/// or key order. Reproducing that keeps the targets in the same order as the C# application.
namespace Sarwave::Core::MMWave {

template <class Key, class Value, class Hash = std::hash<Key>>
class DotNetDictionary {
   public:
    struct Entry {
        Key key;
        Value value;
    };

    [[nodiscard]] std::size_t size() const {
        return m_index.size();
    }

    [[nodiscard]] bool contains(const Key& key) const {
        return m_index.contains(key);
    }

    /// Dictionary.Add: throws ArgumentException when the key is already present.
    void add(const Key& key, Value value) {
        if (m_index.contains(key)) {
            throw ArgumentException("An item with the same key has already been added.");
        }

        std::size_t slot;
        if (!m_freeList.empty()) {
            slot = m_freeList.back();
            m_freeList.pop_back();
            m_slots[slot].emplace(Entry{key, std::move(value)});
        } else {
            slot = m_slots.size();
            m_slots.emplace_back(Entry{key, std::move(value)});
        }
        m_index.emplace(key, slot);
    }

    /// Dictionary.Remove: false when the key was not present.
    bool remove(const Key& key) {
        const auto it = m_index.find(key);
        if (it == m_index.end()) return false;

        m_slots[it->second].reset();
        m_freeList.push_back(it->second);
        m_index.erase(it);
        return true;
    }

    void clear() {
        m_slots.clear();
        m_freeList.clear();
        m_index.clear();
    }

    Value* tryGet(const Key& key) {
        const auto it = m_index.find(key);
        return it == m_index.end() ? nullptr : &m_slots[it->second]->value;
    }

    const Value* tryGet(const Key& key) const {
        const auto it = m_index.find(key);
        return it == m_index.end() ? nullptr : &m_slots[it->second]->value;
    }

    /// Visits every entry in .NET enumeration order.
    template <class F>
    void forEach(F&& visit) const {
        for (const auto& slot : m_slots) {
            if (slot) visit(slot->key, slot->value);
        }
    }

    template <class F>
    void forEach(F&& visit) {
        for (auto& slot : m_slots) {
            if (slot) visit(slot->key, slot->value);
        }
    }

   private:
    std::vector<std::optional<Entry>> m_slots;
    std::vector<std::size_t> m_freeList;  // used as a stack: last freed, first reused
    std::unordered_map<Key, std::size_t, Hash> m_index;
};

/// HashSet<T> with .NET's enumeration order.
template <class T, class Hash = std::hash<T>>
class DotNetHashSet {
   public:
    [[nodiscard]] std::size_t size() const {
        return m_items.size();
    }

    [[nodiscard]] bool contains(const T& item) const {
        return m_items.contains(item);
    }

    /// HashSet.Add: false when already present.
    bool add(const T& item) {
        if (m_items.contains(item)) return false;
        m_items.add(item, Empty{});
        return true;
    }

    bool remove(const T& item) {
        return m_items.remove(item);
    }

    void clear() {
        m_items.clear();
    }

    template <class F>
    void forEach(F&& visit) const {
        m_items.forEach([&](const T& item, const Empty&) { visit(item); });
    }

   private:
    struct Empty {};
    DotNetDictionary<T, Empty, Hash> m_items;
};

}  // namespace Sarwave::Core::MMWave
