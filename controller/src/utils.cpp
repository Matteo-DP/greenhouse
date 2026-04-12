#pragma once
#include "nlohmann/json.hpp"

namespace Utils {

std::string jsonStringOrEmpty(const nlohmann::json& object, const char* key) {
    const auto it = object.find(key);
    if (it == object.end() || it->is_null()) {
        return {};
    }

    if (it->is_string()) {
        return it->get<std::string>();
    }

    return it->dump();
}

};