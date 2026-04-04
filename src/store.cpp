#include "store.h"

#include <stdexcept>

bool ListStore::create(const std::string& key) {
    if (exists(key)) return false;
    store_.emplace(key, LinkedList());
    return true;
}

bool ListStore::exists(const std::string& key) const {
    return store_.count(key) > 0;
}

bool ListStore::drop(const std::string& key) {
    return store_.erase(key) > 0;
}

std::vector<std::string> ListStore::keys() const {
    std::vector<std::string> result;
    result.reserve(store_.size());
    for (const auto& [k, _] : store_)
        result.push_back(k);
    return result;
}

LinkedList& ListStore::get(const std::string& key) {
    auto it = store_.find(key);
    if (it == store_.end())
        throw std::runtime_error("Key not found: " + key);
    return it->second;
}

const LinkedList& ListStore::get(const std::string& key) const {
    auto it = store_.find(key);
    if (it == store_.end())
        throw std::runtime_error("Key not found: " + key);
    return it->second;
}
