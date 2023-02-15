#include "../include/option.hpp"
#include "../include/util.hpp"
#include <cstring>

bool Option::error(std::string message) {
    printf("The '--%s' option %s\n", m_key.c_str(), message.c_str());
    return false;
}

const std::string& Option::getKey() {
    return m_key;
}


BooleanOption::BooleanOption(std::string key, bool *value) {
    m_key = key;
    m_value = value;
    m_defaultValue = false;
}
        
BooleanOption* BooleanOption::defaultTrue() {
    m_defaultValue = true;
    *m_value = true;
    return this;
}

bool BooleanOption::parse(const char *value) {
    if (value) {
        return error("does not expect a value");
    }
    *m_value = !m_defaultValue;
    return true;
}


StringOption::StringOption(std::string key, std::string *value) {
    m_key = key;
    m_value = value;
    m_allowEmpty = true;
}

StringOption* StringOption::nonEmpty() {
    m_allowEmpty = false;
    return this;
}
        
bool StringOption::parse(const char *value) {
    if (!value) {
        return error("expects a value");
    }
    if (!m_allowEmpty && strlen(value) == 0) {
        return error("cannot be an empty string");
    }
    *m_value = value;
    return true;
}


IntegerOption::IntegerOption(std::string key, int *value) {
    m_key = key;
    m_value = value;
}

IntegerOption* IntegerOption::min(int min) {
    m_min = min;
    return this;
}

IntegerOption* IntegerOption::max(int max) {
    m_max = max;
    return this;
}
        
bool IntegerOption::parse(const char *value) {
    if (!isInteger(value)) {
        return error("expects an integer");
    }

    int num;
    try {
        num = std::stoi(value);
    }
    catch (std::out_of_range) {
        return error("received an out of range integer");
    }

    if (m_min.has_value() && num < m_min.value()) {
        return error("value cannot be less than " + std::to_string(m_min.value()));
    }
    if (m_max.has_value() && num > m_max.value()) {
        return error("value cannot be greater than " + std::to_string(m_max.value()));
    }

    *m_value = num;
    return true;
}
