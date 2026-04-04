#pragma once

#include <string>
#include <vector>

namespace json {

// Extract a string or numeric value by key from a flat JSON object.
// Returns empty string if the key is not found.
std::string get(const std::string& json_str, const std::string& key);

// Response builders
std::string make_error(const std::string& message);
std::string make_list(const std::string& key, const std::vector<int>& values);
std::string make_node_value(const std::string& key, int position, int value);
std::string make_popped(const std::string& key, int value);
std::string make_removed(const std::string& key, int value);
std::string make_created(const std::string& key);
std::string make_size(const std::string& key, int sz);
std::string make_keys(const std::vector<std::string>& keys);

} // namespace json
