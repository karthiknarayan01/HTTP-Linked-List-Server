#pragma once

#include "linked_list.h"
#include <string>
#include <unordered_map>
#include <vector>

class ListStore {
public:
    bool create(const std::string& key);
    bool exists(const std::string& key) const;
    bool drop(const std::string& key);
    std::vector<std::string> keys() const;

    LinkedList& get(const std::string& key);
    const LinkedList& get(const std::string& key) const;

private:
    std::unordered_map<std::string, LinkedList> store_;
};
