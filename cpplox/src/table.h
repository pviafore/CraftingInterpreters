#ifndef CPPLOX_TABLE_H_
#define CPPLOX_TABLE_H_

#include <concepts>
#include <utility>
#include <variant>

#include "utility.h"
#include "vector.h"

namespace lox {

    size_t hash(std::integral auto i) {
        return i;
    }

    template <typename T>
    concept GetHash = requires(T a) {
        { a.getHash() } -> std::convertible_to<std::size_t>;
    };

    template <typename T>
    concept Hashable = GetHash<T> && std::equality_comparable<T>;

    size_t hash(Hashable auto h) {
        return h.getHash();
    }

    constexpr double MAX_LOAD = .75;
    template <Hashable K, typename V>
    class Table {
    public:
        struct Empty {};
        struct Tombstone {};
        struct Entry {
            K key;
            V value;
        };

        Table() {
            entries.resize(8, Empty{});
        }
        using TableEntry = std::variant<Empty, Tombstone, Entry>;
        size_t getKeyIndex(const lox::Vector<TableEntry>& entries, const K& key) const {
            size_t tombstoneIndex = entries.size();  // one past the actual size
            for (size_t index = key.getHash() % entries.size(); /* end condition in loop*/; index = (index + 1) % entries.size()) {
                if (std::holds_alternative<Entry>(entries[index]) && std::get<Entry>(entries[index]).key == key) {
                    return index;
                } else if (std::holds_alternative<Tombstone>(entries[index])) {
                    tombstoneIndex = index;
                } else if (std::holds_alternative<Empty>(entries[index])) {
                    return tombstoneIndex == entries.size() ? index : tombstoneIndex;
                }
            }
            std::unreachable();
        }

        void clear() {
            entries.clear();
        }
        bool insert(K key, V value) {
            adjustCapacity();
            auto index = getKeyIndex(entries, key);
            auto& entry = entries[index];
            bool isNewKey = std::holds_alternative<Empty>(entry);
            if (isNewKey) {
                size += 1;
            }
            entries[index] = Entry{std::move(key), std::move(value)};
            return isNewKey;
        }

        void insert(const Table<K, V>& table) {
            for (auto& entry : table.entries) {
                if (std::holds_alternative<Entry>(entry)) {
                    insert(entry.key, entry.value);
                }
            }
        }

        Optional<V> get(const K& key) const {
            if (size == 0) {
                return {};
            }

            size_t index = getKeyIndex(entries, key);
            if (!std::holds_alternative<Entry>(entries[index])) {
                return {};
            }

            return std::get<Entry>(entries[index]).value;
        }

        Optional<K> getKey(const K& key) const {
            if (size == 0) {
                return Optional<K>{};
            }

            size_t index = getKeyIndex(entries, key);
            if (!std::holds_alternative<Entry>(entries[index])) {
                return Optional<K>{};
            }

            const Entry& entry = std::get<Entry>(entries[index]);
            const K& k = entry.key;
            return k;
        }

        bool
        erase(const K& key) {
            auto index = getKeyIndex(entries, key);
            if (!std::holds_alternative<Entry>(entries[index])) {
                return false;
            }
            entries[index] = Tombstone{};
            return true;
        }

    private:
        void adjustCapacity() {
            if (size + 1 > entries.size() * MAX_LOAD) {
                size_t newSize = entries.size() * 2;
                Vector<TableEntry> newEntries;
                newEntries.resize(newSize, Empty{});
                size_t newCount = 0;
                for (auto& entry : entries) {
                    if (std::holds_alternative<Entry>(entry)) {
                        auto index = getKeyIndex(newEntries, std::get<Entry>(entry).key);
                        newEntries[index] = entry;
                        newCount++;
                    }
                }
                size = newCount;
                std::swap(entries, newEntries);
            }
        }
        lox::Vector<TableEntry> entries;  // this is a little slow, as we have two separate indirections (the vector and the string inside)
        size_t size = 0;
    };

    template <Hashable K>
    class HashSet {
    public:
        K insert(const K& key) {
            if (!table.getKey(key).hasValue()) {
                table.insert(key, nullptr);
            }
            return table.getKey(key).value();
        }

        Optional<K> get(const K& key) const {
            return table.getKey(key);
        }

        bool contains(const K& key) const {
            return table.getKey(key).hasValue();
        }

        void clear() {
            table.clear();
        }

    private:
        Table<K, nullptr_t> table;
    };
}
#endif