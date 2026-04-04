#include "json.h"

#include <sstream>

namespace json {

std::string get(const std::string& json_str, const std::string& key) {
    std::string search = "\"" + key + "\"";
    size_t pos = json_str.find(search);
    if (pos == std::string::npos) return "";

    pos += search.size();
    while (pos < json_str.size() && (json_str[pos] == ' ' || json_str[pos] == ':'))
        ++pos;
    if (pos >= json_str.size()) return "";

    if (json_str[pos] == '"') {
        ++pos;
        std::string result;
        while (pos < json_str.size() && json_str[pos] != '"') {
            if (json_str[pos] == '\\' && pos + 1 < json_str.size()) {
                ++pos;
                result += json_str[pos];
            } else {
                result += json_str[pos];
            }
            ++pos;
        }
        return result;
    }

    // Number or boolean
    size_t end = pos;
    while (end < json_str.size() &&
           json_str[end] != ',' && json_str[end] != '}' &&
           json_str[end] != ' '  && json_str[end] != '\n' &&
           json_str[end] != '\r' && json_str[end] != '\t') {
        ++end;
    }
    return json_str.substr(pos, end - pos);
}

std::string make_error(const std::string& message) {
    return "{\"error\":\"" + message + "\"}";
}

static std::string int_array(const std::vector<int>& values) {
    std::ostringstream oss;
    oss << "[";
    for (size_t i = 0; i < values.size(); ++i) {
        if (i > 0) oss << ",";
        oss << values[i];
    }
    oss << "]";
    return oss.str();
}

std::string make_list(const std::string& key, const std::vector<int>& values) {
    std::ostringstream oss;
    oss << "{\"key\":\"" << key << "\",\"values\":" << int_array(values)
        << ",\"size\":" << values.size() << "}";
    return oss.str();
}

std::string make_node_value(const std::string& key, int position, int value) {
    std::ostringstream oss;
    oss << "{\"key\":\"" << key << "\",\"position\":" << position
        << ",\"value\":" << value << "}";
    return oss.str();
}

std::string make_popped(const std::string& key, int value) {
    std::ostringstream oss;
    oss << "{\"key\":\"" << key << "\",\"popped\":" << value << "}";
    return oss.str();
}

std::string make_removed(const std::string& key, int value) {
    std::ostringstream oss;
    oss << "{\"key\":\"" << key << "\",\"removed\":" << value << "}";
    return oss.str();
}

std::string make_created(const std::string& key) {
    return "{\"key\":\"" + key + "\",\"created\":true}";
}

std::string make_size(const std::string& key, int sz) {
    std::ostringstream oss;
    oss << "{\"key\":\"" << key << "\",\"size\":" << sz << "}";
    return oss.str();
}

std::string make_keys(const std::vector<std::string>& keys) {
    std::ostringstream oss;
    oss << "{\"keys\":[";
    for (size_t i = 0; i < keys.size(); ++i) {
        if (i > 0) oss << ",";
        oss << "\"" << keys[i] << "\"";
    }
    oss << "]}";
    return oss.str();
}

} // namespace json
